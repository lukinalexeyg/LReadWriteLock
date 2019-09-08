#include "worker.h"
#include "storage.h"
#include "log.h"



Worker::Worker(QObject *parent) :
    QObject(parent)
{
}



void Worker::acquereForRead()
{
    FUNCTION_LOG
    emit stateChanged("acquere for read");
    INFO_LOG Storage::instance()->acquireForRead(this, "onReadyRead", m_priority);
}



void Worker::acquereForWrite()
{
    FUNCTION_LOG
    emit stateChanged("acquere for write");
    INFO_LOG Storage::instance()->acquireForWrite(this, "onReadyWrite", m_priority);
}



void Worker::waitForRead()
{
    FUNCTION_LOG
    emit stateChanged("wait for read");
    INFO_LOG Storage::instance()->waitForRead();
    onReadyRead();
}



void Worker::waitForWrite()
{
    FUNCTION_LOG
    emit stateChanged("wait for write");
    INFO_LOG Storage::instance()->waitForWrite();
    onReadyWrite();
}



void Worker::onReadyRead()
{
    FUNCTION_LOG
    emit stateChanged("reading");
    DEBUG_LOG "START READING";
    QThread::sleep(m_duration);
    DEBUG_LOG "STOP READING";
    emit stateChanged("-");
    Storage::instance()->release();
}



void Worker::onReadyWrite()
{
    FUNCTION_LOG
    emit stateChanged("writing");
    DEBUG_LOG "START WRITING";
    QThread::sleep(m_duration);
    DEBUG_LOG "STOP WRITING";
    emit stateChanged("-");
    Storage::instance()->release();
}
