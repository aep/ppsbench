#include <libsigrokcxx/libsigrokcxx.hpp>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <map>
#include "mainwindow.h"

class SigRok {
public:
    std::shared_ptr<sigrok::Context> context;
    std::shared_ptr<sigrok::Driver>  driver;
    std::shared_ptr<sigrok::Device>  device;
    const sigrok::ConfigKey *key_target_current;
    const sigrok::ConfigKey *key_target_voltage;
    const sigrok::ConfigKey *key_probe_current;
    const sigrok::ConfigKey *key_probe_voltage;
    const sigrok::ConfigKey *key_enabled;

    SigRok(QStringList args) {
        context = sigrok::Context::create();
        /* Display version information. */
        printf("\nUsing libsigrok %s (lib version %s).\n",
                context->package_version().c_str(),
                context->lib_version().c_str());


        if (args.count() < 2) {
            printf("usage: ./ppabench scpi-pps:conn=/dev/ttyUSB0:serialcomm=9600/8n1\n");
            printf("\nSupported hardware drivers:\n");
            for (auto entry : context->drivers())
            {
                auto driver = entry.second;
                printf("  %-20s %s\n",
                        driver->name().c_str(),
                        driver->long_name().c_str());
            }
            exit(3);
        }

        QStringList driverString = args.at(1).split(":");

        /* Use specified driver. */
        driver = context->drivers()[driverString.takeFirst().toStdString()];

        /* Parse key=value configuration pairs. */
        std::map<const sigrok::ConfigKey *, Glib::VariantBase> scanOptions;
        foreach (QString pair, driverString)
        {
            auto parts = pair.split('=');
            auto name  = parts.first(), value = parts.at(1);
            auto key = sigrok::ConfigKey::get_by_identifier(name.toStdString());
            scanOptions[key] = key->parse_string(value.toStdString());
        }

        /* Scan for devices. */
        auto devices = driver->scan(scanOptions);

        if (devices.size() < 1) {
            printf("no device found\n");
            exit(4);
        }

        /* Use first device found. */
        device = devices.front();
        device->open();


        /* channels */
        printf("channels:\n");
        auto channels = device->channels();
        for (auto channel :  channels) {
            printf("  %s\n", channel->name().c_str());
        }

        /* config */

        printf("config:\n");
        auto config_keys = device->config_keys (sigrok::ConfigKey::DEVICE_OPTIONS);
        for (auto config_key : config_keys) {
            printf("  %s (%s): ", config_key.first->identifier().c_str(), config_key.first->description ().c_str());
            if (config_key.second.count(sigrok::GET)) {
                auto value = device->config_get (config_key.first);
                printf("  %s:%s\n",value.get_type_string().c_str(), value.print().c_str ());
            } else {
                printf("  \n");
            }
        }

        key_target_current =  sigrok::ConfigKey::get_by_identifier("current_limit");
        key_target_voltage =  sigrok::ConfigKey::get_by_identifier("voltage_target");
        key_probe_current  =  sigrok::ConfigKey::get_by_identifier("current");
        key_probe_voltage  =  sigrok::ConfigKey::get_by_identifier("voltage");
        key_enabled        =  sigrok::ConfigKey::get_by_identifier("enabled");

        device->config_set (key_target_voltage, Glib::Variant<double>::create(0.0));
        device->config_set (key_target_current, Glib::Variant<double>::create(0.0));
        device->config_set (key_enabled, Glib::Variant<bool>::create(false));
    }
};


void MainWindow::update_display()
{
    try {
        //    auto value_target_current = sg->device->config_get (sg->key_target_current);
        //    auto value_target_voltage = sg->device->config_get (sg->key_target_voltage);
        auto value_probe_current  = sg->device->config_get (sg->key_probe_current);
        auto value_probe_voltage  = sg->device->config_get (sg->key_probe_voltage);


        ui.display_volt->setText(QString::fromLocal8Bit(value_probe_voltage.print().c_str ()) + "V");
        ui.display_amp->setText(QString::fromLocal8Bit(value_probe_current.print().c_str ()) + "A");

        if (!running)
            return;

        //meassured curve
        double v = QString::fromLocal8Bit(value_probe_voltage.print().c_str ()).toDouble();
        double a = fabsl(QString::fromLocal8Bit(value_probe_current.print().c_str ()).toDouble());

        int mV = floor(v * 1000.0);
        int mA = floor(a * 1000.0);

        printf("I:%lf U:%lf\n",  a, v);

        mes_r << QPointF(a,v);
        mes_points << QPoint(mA, mV);
        mes << QString::number(mA) + " mA";
        ui.chart->setData(mes_points);
        ui.chart->setXHeaderData(mes);

        //safe voltage
        //TODO: this isn't all that safe, considered that we wait an entire second until we trigger
        int safeMv =  floor(ui.input_safe_voltage->value() * 1000.0);
        if (mV < safeMv) {
            qDebug() << "safe voltage trigger";
            ui.groupBox_2->setEnabled(true);
            enableOutput(false);
            running = false;
            return;
        }


        //calculate current point
        if (set_step >= set_points.count()) {
            enableOutput(false);
            ui.groupBox_2->setEnabled(true);
            running = false;
            return;
        }
        double ta = set_points.at(set_step++).y() / 1000.0;

        qDebug() << "drawing " << a;
        sg->device->config_set (sg->key_target_current, Glib::Variant<double>::create(ta));

    } catch (sigrok::Error e) {
        qDebug() << e.what();
        enableOutput(false);
        ui.groupBox_2->setEnabled(true);
        running = false;
        delete sg;
        sg = new SigRok(QCoreApplication::arguments());
        return;
    }
}


void MainWindow::update_set_curve() {
    //calculate set curve
    int time      = ui.input_time->value() * 2; //500ms intervals;
    if (time == 0) {
        return;
    }
    double startA = ui.input_current_start->value();
    double endA   = ui.input_current_end->value();

    double diff = endA - startA;
    if (diff == 0) {
        return;
    }
    double step = diff/time;

    ui.chart->setAutoFillBackground(true);
    ui.chart->setXRange(0, time);
    ui.chart->setYRange(0, floor(endA *1000));
    ui.chart->setYSuffix(" mA");

    QList<QString> head;
    set_points.clear();

    for(int t = 0; t <= time; t++) {
        set_points << QPoint(t , floor(startA * 1000));
        head << QString::number(t);

        startA += step;
    }
    ui.chart->setData(set_points);
    ui.chart->setXHeaderData(head);
}

MainWindow::MainWindow() {
    ui.setupUi(this);

    running = false;
    set_step = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_display()));
    timer->start(500);

    connect(ui.input_current_start, SIGNAL(valueChanged(double)), this, SLOT(update_set_curve()));
    connect(ui.input_current_end, SIGNAL(valueChanged(double)), this, SLOT(update_set_curve()));
    connect(ui.input_time, SIGNAL(valueChanged(int)), this, SLOT(update_set_curve()));


    sg = new SigRok(QCoreApplication::arguments());

    update_display();
}

void MainWindow::on_pushButton_clicked(bool)
{
    if (set_points.count() < 1)
        return;
    ui.groupBox_2->setEnabled(false);
    running = true;
    mes_points.clear();
    mes.clear();
    mes_r.clear();
    set_step = 0;

    ui.chart->setAutoFillBackground(true);
    double endA   = ui.input_current_end->value();
    ui.chart->setXRange(0, floor(endA * 1000));
    ui.chart->setYRange(0, 12000);
    ui.chart->setYSuffix(" mV");
    ui.chart->setXHeaderData(QList<QString>());
    enableOutput(true);

}


void MainWindow::enableOutput(bool e)
{
    qDebug() << "enable" << e;
    sg->device->config_set (sg->key_target_voltage, Glib::Variant<double>::create(0.0));
    sg->device->config_set (sg->key_target_current, Glib::Variant<double>::create(0.0));
    sg->device->config_set (sg->key_enabled, Glib::Variant<bool>::create(e));
}

void MainWindow::on_actionSaveCSV_triggered(bool)
{
    QString fn = QFileDialog::getSaveFileName(this, "save csv", QString(), QString(".csv"));
    if (fn.size() < 1)
        return;
    QFile f(fn);
    if (!f.open(QFile::WriteOnly | QFile::Truncate))
        return;

    {
        QTextStream st(&f);

        st << "A,V\n";
        foreach (QPointF p, mes_r) {
            st << p.x()<< "," << p.y()<< "\n";
        }
    }

    f.close();


}
