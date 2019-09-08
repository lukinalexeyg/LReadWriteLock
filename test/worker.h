#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "log.h"

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr);
    inline void setPriority(int priority) { m_priority = priority; }
    inline void setDuration(quint32 duration) { m_duration = duration; }
    void acquereForRead();
    void acquereForWrite();
    void waitForRead();
    void waitForWrite();

signals:
    void stateChanged(const QString state);

private:
    int m_priority = 0;
    quint32 m_duration = 10;

private slots:
    void onReadyRead();
    void onReadyWrite();
};

#endif // WORKER_H
