#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "Fragment.h"
#include "Interpreter.h"

#include <QWidget>
#include <memory>

// See "Qt In Namespace": https://wiki.qt.io/Qt_In_Namespace
QT_USE_NAMESPACE

namespace Ui {
class Window;
}

namespace basic {

class QBInterpreterWorker : public QObject {
    Q_OBJECT

public:
    explicit QBInterpreterWorker(const std::shared_ptr<Fragment> &frag,
                                 QWidget *input_sender);
    ~QBInterpreterWorker() override = default;

    // No copy or move.
    QBInterpreterWorker(const QBInterpreterWorker &) = delete;
    QBInterpreterWorker(QBInterpreterWorker &&) = delete;
    QBInterpreterWorker &operator=(const QBInterpreterWorker &) = delete;
    QBInterpreterWorker &operator=(QBInterpreterWorker &&) = delete;

public slots:
    void doWork();

signals:
    void resultReady(QString result);
    void requestInput();

private:
    std::shared_ptr<Fragment> frag;
    QWidget *input_sender;
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void execute(std::string_view command);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    // Respond to the finish of the worker. Update UI and clean some states.
    void workerFinish(QString result);

signals:
    void sendInput(QString input);

private:
    Ui::Window *ui;
    std::shared_ptr<Fragment> frag{};
    bool is_runnning{false};
    bool is_inputting{false};

    enum class CommandType {
        RUN,
        LOAD,
        LIST,
        CLEAR,
        HELP,
        QUIT,
        UNKNOWN,
    };
    CommandType getCommandType(std::string_view command);
    void run();
    void load();
    void list();
    void clear();
    void help();
    void quit();
};
} // namespace basic

#endif // MAIN_WINDOW_H