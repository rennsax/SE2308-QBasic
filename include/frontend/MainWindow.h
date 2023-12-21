#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>

// See "Qt In Namespace": https://wiki.qt.io/Qt_In_Namespace
QT_USE_NAMESPACE

namespace Ui {
    class Window;
}


class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    Ui::Window *ui;
};

#endif // MAIN_WINDOW_H