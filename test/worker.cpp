#include <QRandomGenerator>
#include "worker.h"
#include "storage.h"
#include "log.h"



Worker::Worker(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Worker::onTimerTimeout);
}



void Worker::onTimerTimeout()
{
    const int variant = QRandomGenerator::global()->bounded(5);
    m_duration = QRandomGenerator::global()->bounded(m_minDuration, m_maxDuration);

    if (variant == 0)
        waitForRead();
    else if (variant == 1)
        waitForWrite();
    else if (variant == 2)
        acquereForRead();
    else if (variant == 3)
        acquereForWrite();
    else if (variant == 4)
        acquereForReadLambda();
    else if (variant == 5)
        acquereForWriteLambda();

    m_timer->start(QRandomGenerator::global()->bounded(m_maxInterval, m_maxInterval));
}



void Worker::waitForRead()
{
    setState(WaitForRead);
    Storage::instance()->waitForRead(m_priority);
    setState(SyncReading);
    QThread::msleep(static_cast<quint32>(m_duration));
    setState(None);
    Storage::instance()->release();
}



void Worker::waitForWrite()
{
    setState(WaitForWrite);
    Storage::instance()->waitForWrite(m_priority);
    setState(SyncWriting);
    QThread::msleep(static_cast<quint32>(m_duration));
    setState(None);
    Storage::instance()->release();
}



void Worker::acquereForRead()
{
    Storage::instance()->acquireForRead(this, "onReadyRead", m_priority);
}



void Worker::onReadyRead()
{
    setState(AsyncReading);
    QThread::msleep(static_cast<quint32>(m_duration));
    setState(None);
    Storage::instance()->release();
}



void Worker::acquereForWrite()
{
    Storage::instance()->acquireForWrite(this, "onReadyWrite", m_priority);
}



void Worker::onReadyWrite()
{
    setState(AsyncWriting);
    QThread::msleep(static_cast<quint32>(m_duration));
    setState(None);
    Storage::instance()->release();
}



void Worker::acquereForReadLambda()
{
    Storage::instance()->acquireForRead(this, [this]() {
        setState(AsyncReadingLambda);
        QThread::msleep(static_cast<quint32>(m_duration));
        setState(None);
        Storage::instance()->release();
    });
}



void Worker::acquereForWriteLambda()
{
    Storage::instance()->acquireForWrite(this, [this]() {
        setState(AsyncWritingLambda);
        QThread::msleep(static_cast<quint32>(m_duration));
        setState(None);
        Storage::instance()->release();
    });
}



void Worker::setState(State state)
{
    m_state = state;
    emit stateChanged(state);
}



void Worker::start()
{
    if (!m_timer->isActive()) {
        m_timer->start();
        emit activeChanged(true);
    }
    else {
        m_timer->stop();
        emit activeChanged(false);
    }
}



int Worker::commonState()
{
    if (isReadingState())
        return Reading;

    if (isWritingState())
        return Writing;

    return None;
}



bool Worker::isReadingState()
{
    return m_state == SyncReading || m_state == AsyncReading || m_state == AsyncReadingLambda;
}



bool Worker::isWritingState()
{
    return m_state == SyncWriting || m_state == AsyncWriting || m_state == AsyncWritingLambda;
}
