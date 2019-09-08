#ifndef STORAGE_H
#define STORAGE_H

#pragma once

#include <QMutex>
#include "lmutex.h"

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

    bool acquireForRead(QObject *object, const char *member, int priority = 0);
    bool acquireForRead(QObject *object, std::function<void()> function, int priority = 0);

    bool acquireForWrite(QObject *object, const char *member, int priority = 0);
    bool acquireForWrite(QObject *object, std::function<void()> function, int priority = 0);

    const Data *waitForRead();
    Data *waitForWrite();
    void release();

    const Data *read();
    Data *write();

private:
    static Storage *m_instance;
    static QMutex m_instanceMutex;

    LMutex *m_lMutex;
    Data *m_data;

private:
    ~Storage();
};



#define AUTO_RELEASE_STORAGE AutoReleaseStorage __autoReleaseStorage;

class AutoReleaseStorage
{
public:
    ~AutoReleaseStorage() { Storage::instance()->release(); }
};

#endif // STORAGE_H
