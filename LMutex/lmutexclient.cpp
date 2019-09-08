#include <QTimer>
#include "lmutexclient.h"



LMutexClient::LMutexClient(Operation operation, int priority, QObject *parent) :
    QObject(parent),
    m_access(Access::Sync),
    m_operation(operation),
    m_priority(priority),
    m_thread(QThread::currentThread()),
    m_dateTime(QDateTime::currentDateTime())
{
}



LMutexClient::LMutexClient(Operation operation, QObject *object, const char *member,
                           int priority, int overwrite, QObject *parent) :
    QObject(parent),
    m_access(Access::Async),
    m_operation(operation),
    m_object(object),
    m_member(member),
    m_priority(priority),
    m_thread(QThread::currentThread()),
    m_dateTime(QDateTime::currentDateTime()),
    m_overwrite(overwrite)
{
}



LMutexClient::LMutexClient(Operation operation, QObject *object, std::function<void()> function,
                           int priority, int overwrite, QObject *parent) :
    QObject(parent),
    m_access(Async),
    m_operation(operation),
    m_object(object),
    m_function(function),
    m_priority(priority),
    m_thread(QThread::currentThread()),
    m_dateTime(QDateTime::currentDateTime()),
    m_overwrite(overwrite)
{
}



LMutexClient::~LMutexClient()
{
    if (m_isMutexLocked)
        m_mutex.unlock();
}



size_t LMutexClient::function()
{
    if (m_function == nullptr)
        return 0;
    typedef void (f)();
    f **fPointer = m_function.template target<f*>();
    return (size_t)*fPointer;
}



void LMutexClient::lock()
{
    if (m_isMutexLocked)
        return;

    m_mutex.lock();
    m_isMutexLocked = true;
    m_waitCondition.wait(&m_mutex);
    m_isMutexLocked = false;
    m_mutex.unlock();
}



void LMutexClient::run()
{
    if (m_isMutexLocked)
        m_waitCondition.wakeAll();

    QTimer::singleShot(0, this, [this]() {
        emit runned();
        if (strlen(m_member) > 0)
            QMetaObject::invokeMethod(m_object, m_member);
        else if (m_function != nullptr)
            m_function();
    });
}
