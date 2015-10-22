#include <QMainWindow>
#include <QTimer>
#include "ui_mainwindow.h"

class SigRok;
class MainWindow : public QMainWindow
{
    enum RunningMode {
        IdleMode,
        SupplyMode,
        SinkMode
    };
Q_OBJECT;
public:
    MainWindow();
private slots:
    void on_actionSaveCSV_triggered(bool);

    void enableOutput(bool);
    void stop();

    //current sink
    void update_set_curve();
    void update_display();
    void on_pushButton_clicked(bool);
    void on_button_start_supply_clicked(bool);
private:
    Ui::MainWindow ui;
    SigRok *sg;
    QTimer *timer;

    RunningMode mode;
    RunningMode lastMode;

    QList<QPoint>  mes_points;
    QList<QPointF> mes_r;
    QList<QString> mes;

    //current sink
    int  cs_set_step;
    QList<QPoint>  cs_set_points;
};
