#include "moc_MainWindow.cpp"
#include "ui_MainWindow.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent): QWidget{parent}, ui{new Ui::Window{}} {
    Q_INIT_RESOURCE(qbasic);
    this->ui->setupUi(this);
}
