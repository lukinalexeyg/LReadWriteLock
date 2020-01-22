#include "storage.h"
#include "log.h"



Storage *Storage::m_instance = nullptr;
QMutex Storage::m_instanceMutex;



Storage::Storage(QObject *parent) :
    QObject(parent)
{
    m_lmutex = new LReadWriteLock(this);
    m_data = new Data;
}



Storage::~Storage()
{
    delete m_data;
}



Storage *Storage::instance()
{
    QMutexLocker mutexLocker(&m_instanceMutex);
    if (m_instance == nullptr)
        m_instance = new Storage;
    return m_instance;
}



void Storage::destroyInstance()
{
    QMutexLocker mutexLocker(&m_instanceMutex);
    if (m_instance != nullptr) {
        m_instance->deleteLater();
        m_instance = nullptr;
    }
}



const Storage::Data *Storage::waitForRead(int priority)
{
    m_lmutex->waitForRead(priority);
    return m_data;
}



Storage::Data *Storage::waitForWrite(int priority)
{
    m_lmutex->waitForWrite(priority);
    return m_data;
}



void Storage::acquireForRead(QObject *object, const char *member, int priority)
{
    m_lmutex->acquereForRead(object, member, priority);
}



void Storage::acquireForRead(QObject *object, std::function<void()> function, int priority)
{
    m_lmutex->acquereForRead(object, function, priority);
}



void Storage::acquireForWrite(QObject *object, const char *member, int priority)
{
    m_lmutex->acquereForWrite(object, member, priority);
}



void Storage::acquireForWrite(QObject *object, std::function<void ()> function, int priority)
{
    m_lmutex->acquereForWrite(object, function, priority);
}



void Storage::release()
{
    m_lmutex->release();
}



const Storage::Data *Storage::read()
{
    CRITICAL_LOG "access denied";
    return nullptr;
}



Storage::Data *Storage::write()
{
    CRITICAL_LOG "access denied";
    return nullptr;
}
