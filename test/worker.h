#ifndef WORKER_H
#define WORKER_H

#include <QTimer>
#include "log.h"

const QStringList STATE_STRINGS = {
    "-",
    "Wait for read",
    "Wait for write",
    "Acquere for read",
    "Acquere for write",
    "Acquere for read lambda",
    "Acquere for write lambda",
    "Sync reading",
    "Sync writing",
    "Async reading",
    "Async writing",
    "Async reading lambda",
    "Async writing lambda"
};

class Worker : public QObject
{
    Q_OBJECT

public:
    enum State {
        None,
        WaitForRead,
        WaitForWrite,
        AcquereForRead,
        AcquereForWrite,
        AcquereForReadLambda,
        AcquereForWriteLambda,
        SyncReading,
        SyncWriting,
        AsyncReading,
        AsyncWriting,
        AsyncReadingLambda,
        AsyncWritingLambda
    };

    enum CommonState {
        Reading = 1,
        Writing
    };

public:
    explicit Worker(QObject *parent = nullptr);

    inline void setPriority(int priority)           { m_priority = priority; }

    inline int duration()                           { return m_duration; }
    inline void setDuration(int duration)           { m_duration = duration; }

    inline int minDuration()                        { return m_minDuration; }
    inline void setMinDuration(int duration)        { m_minDuration = duration; }

    inline int maxDuration()                        { return m_maxDuration; }
    inline void setMaxDuration(int duration)        { m_maxDuration = duration; }

    inline int minInterval()                        { return m_minInterval; }
    inline void setMinInterval(int interval)        { m_minInterval = interval; }

    inline int maxInterval()                        { return m_maxInterval; }
    inline void setMaxInterval(int interval)        { m_maxInterval = interval; }

    void waitForRead();
    void waitForWrite();

    void acquereForRead();
    void acquereForWrite();

    void acquereForReadLambda();
    void acquereForWriteLambda();

    void start();

    inline int state()                              { return m_state; }
    int commonState();
    bool isReadingState();
    bool isWritingState();

public:
    QThread* m_thread;

signals:
    void stateChanged(int type);
    void activeChanged(bool actuve);

private:
    QTimer *m_timer;
    int m_priority = 0;
    int m_minDuration = 10;
    int m_maxDuration = 3000;
    int m_duration = 3000;
    int m_minInterval = 10;
    int m_maxInterval = 1000;
    State m_state = None;

private slots:
    void onTimerTimeout();
    void onReadyRead();
    void onReadyWrite();
    void setState(State commonState);
};

#endif // WORKER_H
