#ifndef STORAGE_H
#define STORAGE_H

#pragma once

#include <QMutex>
#include "lreadwritelock.h"

class Storage : public QObject
{
    Q_OBJECT

public:
    struct Data {
        QString name;
        QString value;
    };

public:
    explicit Storage(QObject *parent = nullptr);
    static Storage *instance();
    static void destroyInstance();

    const Data *waitForRead(int priority);
    Data *waitForWrite(int priority);

    void acquireForRead(QObject *object, const char *member, int priority = 0);
    void acquireForRead(QObject *object, std::function<void()> function, int priority = 0);

    void acquireForWrite(QObject *object, const char *member, int priority = 0);
    void acquireForWrite(QObject *object, std::function<void()> function, int priority = 0);

    inline void release()       { m_lReadWriteLock->release(); }

    inline const Data *read()   { return m_data; }
    inline Data *write()        { return m_data; }

private:
    static Storage *m_instance;
    static QMutex m_instanceMutex;

    LReadWriteLock *m_lReadWriteLock;
    Data *m_data;

private:
    ~Storage();
};



#define AUTO_RELEASE_STORAGE AutoReleaseStorage __autoReleaseStorage;

class AutoReleaseStorage
{
public:
    inline ~AutoReleaseStorage() { Storage::instance()->release(); }
};

#endif // STORAGE_H
