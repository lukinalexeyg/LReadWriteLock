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

    void release();

    const Data *read();
    Data *write();

private:
    static Storage *m_instance;
    static QMutex m_instanceMutex;

    LReadWriteLock *m_lmutex;
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
