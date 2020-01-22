#ifndef LREADWRITELOCK_H
#define LREADWRITELOCK_H

#pragma once

#include <QMap>
#include "lreadwritetask.h"

// Alexey Lukin
// https://github.com/lukinalexeyg/LReadWriteLock

class LReadWriteLock : public QObject
{
    Q_OBJECT

public:
    enum Policy {
        DontWaitForAnyTasks,
        WaitForSyncWritingQueuedTasks,
        WaitForAllWritingQueuedTasks
    };

    enum LayerOverwrite {
        DefaultLayerOverwrite,
        NoLayerOverwrite,
        MemberLayerOverwrite,
        ObjectLayerOverwrite,
        ThreadLayerOverwrite,
        AllLayersOverwrite
    };

    enum OperationOverwrite {
        DefaultOperationOverwrite,
        NoOperationOverwrite,
        TheSameOperationOverwrite,
        ReadOperationOverwrite,
        WriteOperationOverwrite,
        AllOperationsOverwrite
    };

public:
    explicit LReadWriteLock(QObject *parent = nullptr);
    ~LReadWriteLock();

    void setMaxReadersCount(int count);
    int maxReadersCount();

    void setMaxWritersCount(int count);
    int maxWritersCount();

    void setPolicy(Policy policy);
    Policy policy();

    void setDefaultLayerOverwrite(LayerOverwrite layerOverwrite);
    LayerOverwrite defaultlayerOverwrite();

    void setDefaultOperationOverwrite(OperationOverwrite operationOverwrite);
    OperationOverwrite defaultOperationOverwrite();

    void waitForRead(int priority = 0, LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                     OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);
    void waitForWrite(int priority = 0, LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                      OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);

    bool acquereForRead(QObject *object, const char *member, int priority = 0,
                        LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                        OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);
    bool acquereForRead(QObject *object, std::function<void()> function, int priority = 0,
                        LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                        OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);

    bool acquereForWrite(QObject *object, const char *member, int priority = 0,
                         LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                         OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);
    bool acquereForWrite(QObject *object, std::function<void()> function, int priority = 0,
                         LayerOverwrite defaultlayerOverwrite = DefaultLayerOverwrite,
                         OperationOverwrite defaultOperationOverwrite = DefaultOperationOverwrite);

    void runTasks();
    void release();

    bool isAqueredForRead(QObject *object, const char *member);
    bool isAqueredForWrite(QObject *object, const char *member);

    void clear();

private:
    typedef QMap<QThread*, LReadWriteTask*> LReadWriteTaskMap;
    typedef QList<LReadWriteTask*> LReadWriteTaskList;

    mutable QMutex m_mutex;

    int m_max_readers_count;
    int m_max_writers_count;

    Policy m_policy;

    LayerOverwrite m_layerOverwrite;
    OperationOverwrite m_operationOverwrite;

    LReadWriteTaskMap m_sync_running_tasks;
    LReadWriteTaskMap m_async_running_tasks;
    LReadWriteTaskList m_queued_tasks;

private:
    bool tryRun(LReadWriteTask *task);
    void overwrite(LReadWriteTask *task);
    void deleteNullAsyncQueuedTasks();
    bool isReadyToRun(LReadWriteTask *task);
    QPair<int, int> runningTasksCount(LReadWriteTask *task);
    LReadWriteTask *syncQueuedTask(QThread *thread);
    int writingQueuedTasksCount();
    void insertTaskToRunning(LReadWriteTask *task);
    bool deleteAsyncRunningTask(LReadWriteTask *task);
    bool isAquered(QObject *object, const char *member, int operation = -1);
};

#endif // LMUTEX_H
