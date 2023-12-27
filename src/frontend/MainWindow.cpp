#include "moc_MainWindow.cpp"
#include "ui_MainWindow.h"

#include <QEventLoop>
#include <QFileDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QThread>
#include <fstream>

namespace basic {

MainWindow::MainWindow(QWidget *parent)
    : QWidget{parent}, ui{new Ui::Window{}},
      frag{std::make_shared<Fragment>()} {

    this->ui->setupUi(this);

    connect(this->ui->btn_run, &QPushButton::clicked,
            std::bind(&MainWindow::run, this));
    connect(this->ui->btn_load, &QPushButton::clicked,
            std::bind(&MainWindow::load, this));
    connect(this->ui->btn_clear, &QPushButton::clicked,
            std::bind(&MainWindow::clear, this));

    // Focus the cursor to the command line input.
    this->ui->input->setFocus();
}

void MainWindow::execute(std::string_view command) {

    std::stringstream cmd_ss{std::string{command}};
    ws(cmd_ss);
    std::string cmd{};
    std::string arg{};
    cmd_ss >> cmd; // cmd is a work.
    ws(cmd_ss);
    std::getline(cmd_ss, arg); // The left is arg.

    bool cmd_is_number = all_of(begin(cmd), end(cmd), ::isdigit);

    // Insert or remove a line.
    if (cmd_is_number) {
        LSize line_number = std::stoi(cmd);
        if (arg.empty()) {
            this->frag->remove(line_number);
        } else {
            this->frag->insert(line_number, arg);
        }
        this->update();
        return;
    }

    auto cmd_type = getCommandType(cmd);

    switch (cmd_type) {
    case CommandType::RUN:
        run();
        break;
    case CommandType::LOAD:
        load();
        break;
    case CommandType::LIST:
        list();
        break;
    case CommandType::CLEAR:
        clear();
        break;
    case CommandType::HELP:
        help();
        break;
    case CommandType::QUIT:
        quit();
        break;
    case CommandType::UNKNOWN:
        break;
    }
}

auto MainWindow::getCommandType(std::string_view command) -> CommandType {
    if (command == "RUN") {
        return CommandType::RUN;
    } else if (command == "LOAD") {
        return CommandType::LOAD;
    } else if (command == "LIST") {
        return CommandType::LIST;
    } else if (command == "CLEAR") {
        return CommandType::CLEAR;
    } else if (command == "HELP") {
        return CommandType::HELP;
    } else if (command == "QUIT") {
        return CommandType::QUIT;
    }
    return CommandType::UNKNOWN;
}

void MainWindow::run() {
    QThread *thread = new QThread{};
    QBInterpreterWorker *worker = new QBInterpreterWorker{frag, this};
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &QBInterpreterWorker::doWork);
    connect(worker, &QBInterpreterWorker::resultReady, this,
            &MainWindow::workerFinish);
    connect(worker, &QBInterpreterWorker::requestInput, [this]() {
        this->ui->input->setText("? ");
        this->ui->input->setFocus();
        this->is_inputting = true;
    });

    // Each thread is responsible for only one worker. After the worker
    // finishes, the thread shuts down, and the resource is released later.
    connect(worker, &QBInterpreterWorker::resultReady, thread, &QThread::quit);
    connect(worker, &QBInterpreterWorker::resultReady, thread,
            &QThread::deleteLater);
    connect(thread, &QThread::finished, worker,
            &QBInterpreterWorker::deleteLater);

    is_runnning = true;
    thread->start();
}

void MainWindow::load() {
    QString file_name = QFileDialog::getOpenFileName();
    std::ifstream is{file_name.toStdString()};
    if (is.is_open()) {
        this->frag = std::make_shared<Fragment>(Fragment::read_stream(is));
        this->update();
    }
}

void MainWindow::list() {
    // Ignored
    return;
}

void MainWindow::clear() {
    this->frag = std::make_shared<Fragment>();
    this->ui->result_browser->clear();
    this->ui->ast_browser->clear();
    this->ui->code_browser->clear();
    this->update();
}

void MainWindow::help() {
    this->ui->result_browser->setText(
        QString::fromStdString("Hello, here is QBasic interpreter!"));
}

void MainWindow::quit() {
    QApplication::quit();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return) {
        auto command = this->ui->input->text();
        this->ui->input->clear();
        if (is_runnning) {
            if (!is_inputting) {
                return;
            }

            if (command.startsWith("? ")) {
                qDebug() << "[main] send input to the worker thread";
                sendInput(command.sliced(2));
            } else {
                this->ui->input->setText("? ");
            }

        } else {
            execute(command.toStdString());
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    auto code_str = this->frag->render();

    this->ui->code_browser->setText(QString::fromStdString(code_str));
}

void MainWindow::workerFinish(QString result) {
    qDebug() << "[main] worker finished";
    this->ui->result_browser->setText(result);
    is_runnning = false;
}

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

    emit resultReady(QString::fromStdString(out.str()));
}

QBInterpreterWorker::QBInterpreterWorker(const std::shared_ptr<Fragment> &frag,
                                         QWidget *input_sender)
    : frag(frag), input_sender(input_sender) {
}
} // namespace basic
