#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>

// See "Qt In Namespace": https://wiki.qt.io/Qt_In_Namespace
QT_USE_NAMESPACE


class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
};

#endif // MAIN_WINDOW_H