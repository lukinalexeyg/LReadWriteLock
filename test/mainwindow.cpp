#include <QCoreApplication>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include "mainwindow.h"
#include "storage.h"
#include "worker.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    INFO_LOG QThread::currentThreadId();

    QLabel *threadIdLabel = new QLabel("Thread id");
    QLabel *priotityLabel = new QLabel("Priority");
    QLabel *durationLabel = new QLabel("Duration");
    QLabel *minDurationLabel = new QLabel("Min duration");
    QLabel *maxDurationLabel = new QLabel("Max duration");
    QLabel *minIntervalLabel = new QLabel("Min interval");
    QLabel *maxIntervalLabel = new QLabel("Max interval");

    const int h = 19;
    threadIdLabel->setFixedHeight(h);
    priotityLabel->setFixedHeight(h);
    durationLabel->setFixedHeight(h);
    minDurationLabel->setFixedHeight(h);
    maxDurationLabel->setFixedHeight(h);
    minIntervalLabel->setFixedHeight(h);
    maxIntervalLabel->setFixedHeight(h);

    QSpacerItem *spacerItem = new QSpacerItem(0, 100, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(threadIdLabel);
    vBoxLayout->addWidget(priotityLabel);
    vBoxLayout->addWidget(durationLabel);
    vBoxLayout->addWidget(minDurationLabel);
    vBoxLayout->addWidget(maxDurationLabel);
    vBoxLayout->addWidget(minIntervalLabel);
    vBoxLayout->addWidget(maxIntervalLabel);
    vBoxLayout->addItem(spacerItem);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->addLayout(vBoxLayout);

    for (int i = 0; i < 3; ++i)
        hBoxLayout->addLayout(setWorker());

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(hBoxLayout);
    setCentralWidget(centralWidget);
}



QVBoxLayout *MainWindow::setWorker()
{
    Worker *worker = new Worker;
    worker->m_thread = new QThread(this);
    m_workers.append(worker);
    worker->moveToThread(worker->m_thread);
    connect(worker->m_thread, &QThread::finished, worker, &QObject::deleteLater);
    worker->m_thread->start();

    QLabel *threadIdLabel = new QLabel;
    threadIdLabel->setAlignment(Qt::AlignHCenter);
    QTimer::singleShot(0, worker, [threadIdLabel]() { threadIdLabel->setText(threadIdString()); });

    QSpinBox *prioritySpinBox = new QSpinBox;
    QSpinBox *durationSpinBox = new QSpinBox;
    QSpinBox *minDurationSpinBox = new QSpinBox;
    QSpinBox *maxDurationSpinBox = new QSpinBox;
    QSpinBox *minIntervalSpinBox = new QSpinBox;
    QSpinBox *maxIntervalSpinBox = new QSpinBox;

    const int maxValue = 60000;
    durationSpinBox->setMaximum(maxValue);
    minDurationSpinBox->setMaximum(maxValue);
    maxDurationSpinBox->setMaximum(maxValue);
    minIntervalSpinBox->setMaximum(maxValue);
    maxIntervalSpinBox->setMaximum(maxValue);

    durationSpinBox->setValue(worker->duration());
    minDurationSpinBox->setValue(worker->minDuration());
    maxDurationSpinBox->setValue(worker->maxDuration());
    minIntervalSpinBox->setValue(worker->minInterval());
    maxIntervalSpinBox->setValue(worker->maxInterval());

    connect(prioritySpinBox,    QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setPriority);
    connect(durationSpinBox,    QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setDuration);
    connect(minDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setMinDuration);
    connect(maxDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setMaxDuration);
    connect(minIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setMinInterval);
    connect(maxIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), worker, &Worker::setMaxInterval);

    QPushButton *startPushButton = new QPushButton("Start / Stop");
    QPushButton *waitForReadPushButton = new QPushButton(STATE_STRINGS.at(Worker::WaitForRead));
    QPushButton *waitForWritePushButton = new QPushButton(STATE_STRINGS.at(Worker::WaitForWrite));
    QPushButton *acquereForReadPushButton = new QPushButton(STATE_STRINGS.at(Worker::AcquereForRead));
    QPushButton *acquereForWritePushButton = new QPushButton(STATE_STRINGS.at(Worker::AcquereForWrite));
    QPushButton *acquereForReadLambdaPushButton = new QPushButton(STATE_STRINGS.at(Worker::AcquereForReadLambda));
    QPushButton *acquereForWriteLambdaPushButton = new QPushButton(STATE_STRINGS.at(Worker::AcquereForWriteLambda));

    connect(startPushButton, &QPushButton::clicked, worker, &Worker::start);
    connect(worker, &Worker::activeChanged, this, [startPushButton](bool active) {
        if (active)
            startPushButton->setStyleSheet("QPushButton { background-color: lightgreen; }");
        else
            startPushButton->setStyleSheet("QPushButton { background-color: #e1e1e1; }");
    });

    connect(waitForReadPushButton,              &QPushButton::clicked,  worker, &Worker::waitForRead);
    connect(waitForWritePushButton,             &QPushButton::clicked,  worker, &Worker::waitForWrite);
    connect(acquereForReadPushButton,           &QPushButton::clicked,  worker, &Worker::acquereForRead);
    connect(acquereForWritePushButton,          &QPushButton::clicked,  worker, &Worker::acquereForWrite);
    connect(acquereForReadLambdaPushButton,     &QPushButton::clicked,  worker, &Worker::acquereForReadLambda);
    connect(acquereForWriteLambdaPushButton,    &QPushButton::clicked,  worker, &Worker::acquereForWriteLambda);

    QLabel *stateLabel = new QLabel;
    stateLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(threadIdLabel);
    vBoxLayout->addWidget(prioritySpinBox);
    vBoxLayout->addWidget(durationSpinBox);
    vBoxLayout->addWidget(minDurationSpinBox);
    vBoxLayout->addWidget(maxDurationSpinBox);
    vBoxLayout->addWidget(minIntervalSpinBox);
    vBoxLayout->addWidget(maxIntervalSpinBox);
    vBoxLayout->addWidget(startPushButton);
    vBoxLayout->addWidget(waitForReadPushButton);
    vBoxLayout->addWidget(waitForWritePushButton);
    vBoxLayout->addWidget(acquereForReadPushButton);
    vBoxLayout->addWidget(acquereForWritePushButton);
    vBoxLayout->addWidget(acquereForReadLambdaPushButton);
    vBoxLayout->addWidget(acquereForWriteLambdaPushButton);
    vBoxLayout->addWidget(stateLabel);

    connect(worker, &Worker::stateChanged, this, [this, worker, stateLabel] (int state) {
        stateLabel->setText(STATE_STRINGS.at(state));

        if (worker->isReadingState())
            stateLabel->setStyleSheet("QLabel { color : green; }");
        else if (worker->isWritingState())
            stateLabel->setStyleSheet("QLabel { color : blue; }");
        else
            stateLabel->setStyleSheet("QLabel { color : black; }");

        qDebug() << m_workers.at(0)->state() << m_workers.at(1)->state() << m_workers.at(2)->state();

        if ((m_workers.at(0)->commonState() == Worker::Writing && (m_workers.at(1)->commonState() == Worker::Reading || m_workers.at(2)->commonState() == Worker::Reading))
                || (m_workers.at(1)->commonState() == Worker::Writing && (m_workers.at(0)->commonState() == Worker::Reading || m_workers.at(2)->commonState() == Worker::Reading))
                || (m_workers.at(2)->commonState() == Worker::Writing && (m_workers.at(0)->commonState() == Worker::Reading || m_workers.at(1)->commonState() == Worker::Reading))
                || (m_workers.at(0)->commonState() == Worker::Writing && m_workers.at(1)->commonState() == Worker::Writing)
                || (m_workers.at(0)->commonState() == Worker::Writing && m_workers.at(2)->commonState() == Worker::Writing)
                || (m_workers.at(1)->commonState() == Worker::Writing && m_workers.at(2)->commonState() == Worker::Writing)
                ) {
            CRITICAL_LOG "ERROR";
            qApp->exit();
        }
    });

    return vBoxLayout;
}



MainWindow::~MainWindow()
{
    for (Worker *worker : qAsConst(m_workers)) {
        QThread *thread = worker->m_thread;
        thread->quit();
        if (!thread->wait(3000)) {
            thread->terminate();
            thread->wait();
        }
    }
    Storage::destroyInstance();
}
