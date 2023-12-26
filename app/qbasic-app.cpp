#include <QApplication>
#include <frontend/MainWindow.h>

int main(int argc, char *argv[]) {
    QApplication app{argc, argv};
    basic::MainWindow main_window{};
    main_window.show();
    return QApplication::exec();
}