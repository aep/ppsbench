#include <QMainWindow>
#include <QTimer>
#include "ui_mainwindow.h"

class SigRok;
class MainWindow : public QMainWindow
{
Q_OBJECT;
public:
    MainWindow();
private slots:
    void enableOutput(bool);
    void update_set_curve();
    void update_display();
    void on_pushButton_clicked(bool);
    void on_actionSaveCSV_triggered(bool);
private:
    Ui::MainWindow ui;
    SigRok *sg;
    QTimer *timer;

    bool running;
    int set_step;

    QList<QPoint> set_points;
    QList<QPoint> mes_points;
    QList<QPointF> mes_r;
    QList<QString> mes;
};
