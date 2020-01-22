#ifndef LREADWRITETASK_H
#define LREADWRITETASK_H

#pragma once

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QPointer>
#include <QDateTime>

class LReadWriteTask : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Sync,
        Async
    };

    enum Operation {
        Read,
        Write
    };

    LReadWriteTask(Operation operation, int priority, int layerOverwrite, int operationOverwrite, QObject *parent = nullptr);

    LReadWriteTask(Operation operation, QObject *object, const char *member, int priority,
                   int layerOverwrite, int operationOverwrite, QObject *parent = nullptr);

    LReadWriteTask(Operation operation, QObject *object, std::function<void ()> function, int priority,
                   int layerOverwrite, int operationOverwrite, QObject *parent = nullptr);

    ~LReadWriteTask();

    inline QThread *thread()        { return m_thread; }
    inline Type type()              { return m_type; }
    inline Operation operation()    { return m_operation; }
    inline QObject *object()        { return m_object.data(); }
    inline bool isObjectNull()      { return m_object.isNull(); }
    inline const char *member()     { return m_member; }
    inline int priority()           { return m_priority; }
    inline QDateTime dateTime()     { return m_dateTime; }
    inline int layerOverwrite()     { return m_layerOverwrite; }
    inline int operationOverwrite() { return m_operationOverwrite; }

    void lock();
    void unlock();
    bool run();

    bool compareLayerOverwrite(LReadWriteTask *other, int layerOverwrite);
    bool compareOperationOverwrite(LReadWriteTask *other, int operationOverwrite);

private:
    mutable QMutex m_mutex;
    bool m_isMutexLocked = false;
    QThread *m_thread;
    QWaitCondition m_waitCondition;
    Type m_type;
    QPointer<QObject> m_object;
    const char *m_member = "\0";
    std::function<void()> m_function = nullptr;
    Operation m_operation;
    int m_priority = 0;
    QDateTime m_dateTime;
    int m_layerOverwrite;
    int m_operationOverwrite;

private:
    long function();
};

#endif // LREADWRITETASK_H
