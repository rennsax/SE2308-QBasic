#include "MainWindow.h"
#include "moc_QBasicInterpreterWorker.cpp"

#include <QDebug>
#include <QEventLoop>

namespace basic {
void QBInterpreterWorker::doWork() {
    qDebug() << "[worker] doWork()";

    std::stringstream out{};
    std::stringstream err{};

    auto input_action = [this]() -> std::string {
        QEventLoop loop{};
        std::string input_str{};

        connect(dynamic_cast<MainWindow *>(input_sender),
                &MainWindow::sendInput, &loop, &QEventLoop::quit);
        connect(dynamic_cast<MainWindow *>(input_sender),
                &MainWindow::sendInput, [&input_str](QString input) {
                    input_str = input.toStdString();
                });
        emit requestInput();

        // Start a local event loop to wait for the input.
        qDebug() << "[worker] waiting for input...";
        loop.exec();

        return input_str;
    };

    Interpreter interpreter{frag, out, err, std::move(input_action)};

    interpreter.interpret();

    emit resultReady(QString::fromStdString(out.str()),
                     QString::fromStdString(err.str()),
                     QString::fromStdString(interpreter.show_ast()));
    this->deleteLater();
}

QBInterpreterWorker::QBInterpreterWorker(const std::shared_ptr<Fragment> &frag,
                                         QWidget *input_sender)
    : frag(frag), input_sender(input_sender) {
}
} // namespace basic
