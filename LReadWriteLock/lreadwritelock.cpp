#include "lreadwritelock.h"



LReadWriteLock::LReadWriteLock(QObject *parent) :
    QObject(parent),
    m_max_readers_count(-1),
    m_max_writers_count(1),
    m_policy(WaitForAllWritingQueuedTasks),
    m_layerOverwrite(MemberLayerOverwrite),
    m_operationOverwrite(TheSameOperationOverwrite)
{
}



LReadWriteLock::~LReadWriteLock()
{
    clear();
}



void LReadWriteLock::clear()
{
    QMutexLocker mutexLocker(&m_mutex);

    qDeleteAll(m_sync_running_tasks);
    qDeleteAll(m_async_running_tasks);
    qDeleteAll(m_queued_tasks);

    m_sync_running_tasks.clear();
    m_async_running_tasks.clear();
    m_queued_tasks.clear();
}



void LReadWriteLock::setMaxReadersCount(int count)
{
    QMutexLocker mutexLocker(&m_mutex);

    if (m_max_readers_count != count) {
        m_max_readers_count = count;
        runTasks();
    }
}



int LReadWriteLock::maxReadersCount()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_max_readers_count;
}



void LReadWriteLock::setMaxWritersCount(int count)
{
    QMutexLocker mutexLocker(&m_mutex);

    if (m_max_writers_count != count) {
        m_max_writers_count = count;
        runTasks();
    }
}



int LReadWriteLock::maxWritersCount()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_max_writers_count;
}



void LReadWriteLock::setPolicy(Policy policy)
{
    QMutexLocker mutexLocker(&m_mutex);

    if (m_policy != policy) {
        m_policy = policy;
        runTasks();
    }
}



LReadWriteLock::Policy LReadWriteLock::policy()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_policy;
}



void LReadWriteLock::setDefaultLayerOverwrite(LayerOverwrite layerOverwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_layerOverwrite = layerOverwrite;
}



LReadWriteLock::LayerOverwrite LReadWriteLock::defaultlayerOverwrite()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_layerOverwrite;
}



void LReadWriteLock::setDefaultOperationOverwrite(OperationOverwrite operationOverwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_operationOverwrite = operationOverwrite;
}



LReadWriteLock::OperationOverwrite LReadWriteLock::defaultOperationOverwrite()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_operationOverwrite;
}



void LReadWriteLock::waitForRead(int priority, LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Read, priority, layerOverwrite, operationOverwrite);
    tryRun(task);
}



void LReadWriteLock::waitForWrite(int priority, LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Write, priority, layerOverwrite, operationOverwrite);
    tryRun(task);
}



bool LReadWriteLock::acquereForRead(QObject *object, const char *member, int priority,
                                    LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Read, object, member, priority, layerOverwrite, operationOverwrite);
    return tryRun(task);
}



bool LReadWriteLock::acquereForRead(QObject *object, std::function<void()> function, int priority,
                                    LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Read, object, function, priority, layerOverwrite, operationOverwrite);
    return tryRun(task);
}



bool LReadWriteLock::acquereForWrite(QObject *object, const char *member, int priority,
                                     LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Write, object, member, priority, layerOverwrite, operationOverwrite);
    return tryRun(task);
}



bool LReadWriteLock::acquereForWrite(QObject *object, std::function<void ()> function, int priority,
                                     LayerOverwrite layerOverwrite, OperationOverwrite operationOverwrite)
{
    LReadWriteTask *task = new LReadWriteTask(LReadWriteTask::Write, object, function, priority, layerOverwrite, operationOverwrite);
    return tryRun(task);
}



bool lessThan(LReadWriteTask *a, LReadWriteTask *b)
{
    if (a->type() == b->type()) {
        if (a->priority() == b->priority()) {
            if (a->operation() == b->operation())
                return a->dateTime() < b->dateTime();
            return a->operation() > b->operation();
        }
        return a->priority() > b->priority();
    }

    if (a->thread() != b->thread()) {
        if (a->priority() == b->priority())
            return a->type() < b->type();
        return a->priority() > b->priority();
    }

    return a->type() < b->type();
}



bool LReadWriteLock::tryRun(LReadWriteTask *task)
{
    m_mutex.lock();

    deleteNullAsyncQueuedTasks();
    overwrite(task);

    if (isReadyToRun(task)) {
        if (task->run()) {
            insertTaskToRunning(task);
            m_mutex.unlock();
            return true;
        }

        task->deleteLater();
        m_mutex.unlock();
        return false;
    }

    m_queued_tasks.append(task);
    std::sort(m_queued_tasks.begin(), m_queued_tasks.end(), lessThan);

    m_mutex.unlock();

    if (task->type() == LReadWriteTask::Sync)
        task->lock();

    return false;
}



void LReadWriteLock::overwrite(LReadWriteTask *task)
{
    const int layerOverwrite = task->layerOverwrite() == DefaultLayerOverwrite ? m_layerOverwrite : task->layerOverwrite();

    if (layerOverwrite == NoLayerOverwrite
            || (task->type() == LReadWriteTask::Sync && (layerOverwrite == MemberLayerOverwrite || layerOverwrite == ObjectLayerOverwrite)))
        return;

    const int operationOverwrite = task->operationOverwrite() == DefaultOperationOverwrite ? m_operationOverwrite : task->operationOverwrite();
    if (operationOverwrite == NoOperationOverwrite)
        return;

    for (int i = 0; i < m_queued_tasks.count(); ++i) {
        LReadWriteTask *t = m_queued_tasks.at(i);

        if (t->type() != LReadWriteTask::Sync
                && t->compareLayerOverwrite(task, layerOverwrite)
                && t->compareOperationOverwrite(task, operationOverwrite)) {
            delete m_queued_tasks.takeAt(i);
            --i;
        }
    }
}



void LReadWriteLock::release()
{
    QMutexLocker mutexLocker(&m_mutex);

    QThread *currentThread = QThread::currentThread();
    if (m_sync_running_tasks.contains(currentThread)) {
        m_sync_running_tasks.take(currentThread)->deleteLater();
        runTasks();
    }
    else if (m_async_running_tasks.contains(currentThread)) {
        m_async_running_tasks.take(currentThread)->deleteLater();
        runTasks();
    }
}



void LReadWriteLock::runTasks()
{
    deleteNullAsyncQueuedTasks();

    QThread *currentThread = QThread::currentThread();
    LReadWriteTask *currentThreadSyncTask = nullptr;

    for (int i = 0; i < m_queued_tasks.count(); ++i) {
        LReadWriteTask *task = m_queued_tasks.at(i);

        if (isReadyToRun(task)) {
            m_queued_tasks.removeAt(i);

            if (task->type() == LReadWriteTask::Sync && task->thread() == currentThread) {
                currentThreadSyncTask = task;
                insertTaskToRunning(task);
            }
            else if (task->run())
                insertTaskToRunning(task);
            else
                task->deleteLater();

            --i;
        }
    }

    if (currentThreadSyncTask != nullptr)
        currentThreadSyncTask->run();
}



void LReadWriteLock::deleteNullAsyncQueuedTasks()
{
    for (int i = 0; i < m_queued_tasks.count(); ++i) {
        LReadWriteTask *task = m_queued_tasks.at(i);

        if (task->type() == LReadWriteTask::Async && task->isObjectNull()) {
            m_queued_tasks.takeAt(i)->deleteLater();
            --i;
        }
    }
}



bool LReadWriteLock::isReadyToRun(LReadWriteTask *task)
{
    if (task->type() == LReadWriteTask::Async
            && (m_sync_running_tasks.contains(task->thread()) || m_async_running_tasks.contains(task->thread())))
        return false;

    const QPair<int, int> _runningTasksCount = runningTasksCount(task);

    if (task->operation() == LReadWriteTask::Read)
        return _runningTasksCount.second == 0
                && (m_max_readers_count < 0 || (_runningTasksCount.first < m_max_readers_count))
                && (task->type() == LReadWriteTask::Sync || writingQueuedTasksCount() == 0);

    return _runningTasksCount.first == 0
            && (m_max_writers_count < 0 || (_runningTasksCount.second < m_max_writers_count));
}



QPair<int, int> LReadWriteLock::runningTasksCount(LReadWriteTask *task)
{
    QPair<int, int> count = qMakePair(0, 0);

    for (LReadWriteTask *t : qAsConst(m_sync_running_tasks))
        if (t->operation() == LReadWriteTask::Read)
            ++count.first;
        else
            ++count.second;

    for (LReadWriteTask *t : qAsConst(m_async_running_tasks))
        if (task->type() == LReadWriteTask::Async || (task->type() == LReadWriteTask::Sync && t->thread() != task->thread())) {
            LReadWriteTask *sync_running_task = m_sync_running_tasks.value(t->thread(), nullptr);

            if (sync_running_task == nullptr || (t->operation() != sync_running_task->operation())) {
                LReadWriteTask *sync_queued_task = syncQueuedTask(t->thread());

                if (sync_queued_task == nullptr || (t->operation() != sync_queued_task->operation())) {
                    if (t->operation() == LReadWriteTask::Read)
                        ++count.first;
                    else
                        ++count.second;
                }
            }
        }

    return count;
}



LReadWriteTask *LReadWriteLock::syncQueuedTask(QThread *thread)
{
    for (LReadWriteTask *task : qAsConst(m_queued_tasks))
        if (task->type() == LReadWriteTask::Sync && task->thread() == thread)
            return task;

    return nullptr;
}



int LReadWriteLock::writingQueuedTasksCount()
{
    int count = 0;

    for (LReadWriteTask *task : qAsConst(m_queued_tasks))
        if (task->operation() == LReadWriteTask::Write
                && ((task->type() == LReadWriteTask::Sync && m_policy != LReadWriteLock::DontWaitForAnyTasks)
                    || (task->type() == LReadWriteTask::Async && m_policy == LReadWriteLock::WaitForAllWritingQueuedTasks)))
            ++count;

    return count;
}



void LReadWriteLock::insertTaskToRunning(LReadWriteTask *task)
{
    if (task->type() == LReadWriteTask::Sync)
        m_sync_running_tasks.insert(task->thread(), task);

    else {
        m_async_running_tasks.insert(task->thread(), task);
        if (strlen(task->member()) > 0)
            connect(task->object(), &QObject::destroyed, this, [this, task]() {
                QMutexLocker mutexLocker(&m_mutex);
                if (deleteAsyncRunningTask(task))
                    runTasks();
            });
    }
}



bool LReadWriteLock::deleteAsyncRunningTask(LReadWriteTask *task)
{
    for (auto it = m_async_running_tasks.begin(); it != m_async_running_tasks.end(); ++it) {
        LReadWriteTask *t = it.value();

        if (t == task) {
            m_async_running_tasks.take(it.key())->deleteLater();
            return true;
        }
    }

    return false;
}



bool LReadWriteLock::isAqueredForRead(QObject *object, const char *member)
{
    QMutexLocker mutexLocker(&m_mutex);
    return isAquered(object, member);
}



bool LReadWriteLock::isAqueredForWrite(QObject *object, const char *member)
{
    QMutexLocker mutexLocker(&m_mutex);
    return isAquered(object, member, LReadWriteTask::Write);
}



bool LReadWriteLock::isAquered(QObject *object, const char *member, int operation)
{
    QThread *thread = QThread::currentThread();
    LReadWriteTask *task = m_async_running_tasks.value(thread, nullptr);

    return task != nullptr
            && task->object() == object
            && strcmp(task->member(), member) == 0
            && (operation == -1 || task->operation() == operation);
}
