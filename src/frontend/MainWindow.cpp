#include "moc_MainWindow.cpp"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QPushButton>
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
    qDebug() << command;

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
    // TODO
    std::stringstream out{};
    std::stringstream err{};
    std::stringstream in{};
    Interpreter interpreter{this->frag, out, err, in};

    interpreter.interpret();

    this->ui->result_browser->setText(QString::fromStdString(out.str()));
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
        execute(command.toStdString());
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    auto code_str = this->frag->render();

    this->ui->code_browser->setText(QString::fromStdString(code_str));
}
} // namespace basic
