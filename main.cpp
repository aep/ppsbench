#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

#include "mainwindow.h"

int main(int argc, char **argv) {
    QApplication app(argc,argv);

    MainWindow main;
    main.show();

    return app.exec();
}

