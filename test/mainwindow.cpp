#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "mainwindow.h"
#include "storage.h"
#include "worker.h"
#include "log.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    INFO_LOG QThread::currentThreadId();

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        QThread *thread = new QThread(this);
        m_threads.append(thread);
        hBoxLayout->addLayout(setWorker(thread, i));
    }

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(hBoxLayout);
    setCentralWidget(centralWidget);
}



QVBoxLayout *MainWindow::setWorker(QThread *thread, int index)
{
    Worker *worker = new Worker;
    m_workers.append(worker);
    thread = new QThread(this);
    worker->moveToThread(thread);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [](){DEBUG_LOG "finished";});
    thread->start();

    const QString indexString = QString::number(index);

    QLabel *threadIdLabel = new QLabel;
    QTimer::singleShot(0, worker, [threadIdLabel]() {
        threadIdLabel->setText(threadIdString());
    });

    threadIdLabel->setAlignment(Qt::AlignHCenter);
    QPushButton *acquereForReadPushButton = new QPushButton("Acquere for read " + indexString);
    QPushButton *acquereForWritePushButton = new QPushButton("Acquere for write " + indexString);
    QPushButton *waitForReadPushButton = new QPushButton("Wait for read " + indexString);
    QPushButton *waitForWritePushButton = new QPushButton("Wait for write " + indexString);
    QLabel *label = new QLabel("-");
    label->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(threadIdLabel);
    vBoxLayout->addWidget(acquereForReadPushButton);
    vBoxLayout->addWidget(acquereForWritePushButton);
    vBoxLayout->addWidget(waitForReadPushButton);
    vBoxLayout->addWidget(waitForWritePushButton);
    vBoxLayout->addWidget(label);

    connect(acquereForReadPushButton,   &QPushButton::clicked, worker,  &Worker::acquereForRead);
    connect(acquereForWritePushButton,  &QPushButton::clicked, worker,  &Worker::acquereForWrite);
    connect(waitForReadPushButton,      &QPushButton::clicked, worker,  &Worker::waitForRead);
    connect(waitForWritePushButton,     &QPushButton::clicked, worker,  &Worker::waitForWrite);
    connect(worker,                     &Worker::stateChanged, label,   &QLabel::setText);

    return vBoxLayout;
}



MainWindow::~MainWindow()
{
    for (QThread *thread : qAsConst(m_threads)) {
        thread->quit();
        if (!thread->wait(3000)) {
            thread->terminate();
            thread->wait();
        }
    }
    Storage::destroyInstance();
}
