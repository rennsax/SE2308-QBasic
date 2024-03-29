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
    void workerFinish(QString output, QString error, QString ast_out);

signals:
    void sendInput(QString input);

private:
    Ui::Window *ui;
    std::shared_ptr<Fragment> frag{};
    std::shared_ptr<Fragment> frag_mini{};
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
    enum class MiniBasicCmd {
        INPUT,
        PRINT,
        LET,
    };
    CommandType getCommandType(std::string_view command);
    std::optional<MiniBasicCmd> getMiniBasicCmd(std::string_view command);
    void run();
    void load();
    void list();
    void clear();
    void help();
    void quit();
    void syncCodeFrag();

    void doRun(std::shared_ptr<Fragment> frag);
};
} // namespace basic

#endif // MAIN_WINDOW_H