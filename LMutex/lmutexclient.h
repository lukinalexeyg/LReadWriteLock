#ifndef LMUTEXCLIENT_H
#define LMUTEXCLIENT_H

#pragma once

#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <QDateTime>
#include <QMutex>

class LMutexClient : public QObject
{
    Q_OBJECT

public:
    enum Access {
        Async,
        Sync
    };

    enum Operation {
        Read,
        Write
    };

    LMutexClient(Operation operation, int priority, QObject *parent = nullptr);
    LMutexClient(Operation operation, QObject *object, const char *member, int priority, int overwrite, QObject *parent = nullptr);
    LMutexClient(Operation operation, QObject *object, std::function<void ()> function, int priority, int overwrite, QObject *parent = nullptr);
    ~LMutexClient();

    inline Access access()          { return m_access; }
    inline Operation operation()    { return m_operation; }
    inline QThread *thread()        { return m_thread; }
    inline QObject *object()        { return m_object; }
    inline const char *member()     { return m_member; }
    size_t function();
    inline int priority()           { return m_priority; }
    inline QDateTime dateTime()     { return m_dateTime; }
    inline int overwrite()          { return m_overwrite; }

    void lock();
    void run();

signals:
    void runned();

private:
    mutable QMutex m_mutex;
    bool m_isMutexLocked = false;
    QWaitCondition m_waitCondition;
    Access m_access = Async;
    Operation m_operation = Read;
    QObject *m_object = nullptr;
    const char *m_member = "\0";
    std::function<void()> m_function = nullptr;
    int m_priority = 0;
    QThread *m_thread;
    QDateTime m_dateTime;
    int m_overwrite;
};

#endif // LMUTEXCLIENT_H
