#include "storage.h"
#include "log.h"



Storage *Storage::m_instance = nullptr;
QMutex Storage::m_instanceMutex;



Storage::Storage(QObject *parent) :
    QObject(parent)
{
    m_lMutex = new LMutex(this);
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



bool Storage::acquireForRead(QObject *object, const char *member, int priority)
{
    return m_lMutex->acquereForRead(object, member, priority);
}



bool Storage::acquireForRead(QObject *object, std::function<void()> function, int priority)
{
    return m_lMutex->acquereForRead(object, function, priority);
}



bool Storage::acquireForWrite(QObject *object, const char *member, int priority)
{
    return m_lMutex->acquereForWrite(object, member, priority);
}



bool Storage::acquireForWrite(QObject *object, std::function<void ()> function, int priority)
{
    return m_lMutex->acquereForWrite(object, function, priority);
}



const Storage::Data *Storage::waitForRead()
{
    m_lMutex->waitForRead();
    return m_data;
}



Storage::Data *Storage::waitForWrite()
{
    m_lMutex->waitForWrite();
    return m_data;
}



void Storage::release()
{
    m_lMutex->release();
}



const Storage::Data *Storage::read()
{
    if (m_lMutex->isAcquiredForRead())
        return m_data;

    CRITICAL_LOG "access denied";
    return nullptr;
}



Storage::Data *Storage::write()
{
    if (m_lMutex->isAcquiredForWrite())
        return m_data;

    CRITICAL_LOG "access denied";
    return nullptr;
}
