#ifndef QBASIC_INTERPRETER_WORKER_H
#define QBASIC_INTERPRETER_WORKER_H

#include "Fragment.h"

#include <QObject>
#include <memory>

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
    void resultReady(QString output, QString error, QString ast_out);
    void requestInput();

private:
    std::shared_ptr<Fragment> frag;
    QWidget *input_sender;
};

} // namespace basic

#endif // QBASIC_INTERPRETER_WORKER_H
