#include <QTimer>
#include "lreadwritelock.h"



LReadWriteTask::LReadWriteTask(Operation operation, int priority, int layerOverwrite, int operationOverwrite, QObject *parent) :
    QObject(parent),
    m_thread(QThread::currentThread()),
    m_type(Type::Sync),
    m_operation(operation),
    m_priority(priority),
    m_dateTime(QDateTime::currentDateTime()),
    m_layerOverwrite(layerOverwrite),
    m_operationOverwrite(operationOverwrite)
{
}



LReadWriteTask::LReadWriteTask(Operation operation, QObject *object, const char *member,
                             int priority, int layerOverwrite, int operationOverwrite, QObject *parent) :
    QObject(parent),
    m_thread(QThread::currentThread()),
    m_type(Type::Async),
    m_object(object),
    m_member(member),
    m_operation(operation),
    m_priority(priority),
    m_dateTime(QDateTime::currentDateTime()),
    m_layerOverwrite(layerOverwrite),
    m_operationOverwrite(operationOverwrite)
{
}



LReadWriteTask::LReadWriteTask(Operation operation, QObject *object, std::function<void()> function,
                             int priority, int layerOverwrite, int operationOverwrite, QObject *parent) :
    QObject(parent),
    m_thread(QThread::currentThread()),
    m_type(Type::Async),
    m_object(object),
    m_function(function),
    m_operation(operation),
    m_priority(priority),
    m_dateTime(QDateTime::currentDateTime()),
    m_layerOverwrite(layerOverwrite),
    m_operationOverwrite(operationOverwrite)
{
}



LReadWriteTask::~LReadWriteTask()
{
    if (m_isMutexLocked)
        m_mutex.unlock();
}



void LReadWriteTask::lock()
{
    if (m_isMutexLocked)
        return;

    m_isMutexLocked = true;
    m_mutex.lock();
    m_waitCondition.wait(&m_mutex);
    m_isMutexLocked = false;
    m_mutex.unlock();
}



void LReadWriteTask::unlock()
{
    if (m_isMutexLocked)
        m_waitCondition.wakeAll();
}



bool LReadWriteTask::run()
{
    if (m_type == Type::Sync) {
        if (m_isMutexLocked)
            m_waitCondition.wakeAll();
        return true;
    }

    if (strlen(m_member) > 0)
        return QMetaObject::invokeMethod(m_object.data(), m_member, Qt::ConnectionType::QueuedConnection);

    if (m_function != nullptr) {
        QTimer::singleShot(0, this, [this]() { m_function(); });
        return true;
    }

    return false;
}



bool LReadWriteTask::compareLayerOverwrite(LReadWriteTask *other, int layerOverwrite)
{
    if (layerOverwrite == LReadWriteLock::MemberLayerOverwrite)
        return m_thread == other->m_thread
                && object() == other->object()
                && (m_member == other->m_member || (function() > 0 && (function() == other->function())));

    if (layerOverwrite == LReadWriteLock::ObjectLayerOverwrite)
        return m_thread == other->m_thread && object() == other->object();

    if (layerOverwrite == LReadWriteLock::ThreadLayerOverwrite)
        return m_thread == other->m_thread;

    return true;
}



long LReadWriteTask::function()
{
    if (m_function == nullptr)
        return 0;

    char *c = reinterpret_cast<char *>(&m_function);
    return *reinterpret_cast<long *>(c);
}



bool LReadWriteTask::compareOperationOverwrite(LReadWriteTask *other, int operationOverwrite)
{
    if (operationOverwrite == LReadWriteLock::TheSameOperationOverwrite)
        return m_operation == other->operation();

    if (operationOverwrite == LReadWriteLock::ReadOperationOverwrite)
        return m_operation == LReadWriteTask::Read;

    if (operationOverwrite == LReadWriteLock::WriteOperationOverwrite)
        return m_operation == LReadWriteTask::Write;

    return true;
}
