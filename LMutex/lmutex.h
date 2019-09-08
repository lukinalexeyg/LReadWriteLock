#ifndef LMUTEX_H
#define LMUTEX_H

#pragma once

#include "lmutexclient.h"

typedef QList<LMutexClient*> LMutexClientList;

bool lMutexLessThan(LMutexClient *a, LMutexClient *b);

class LMutex : public QObject
{
    Q_OBJECT

public:
    enum Overwrite {
        NoOverwrite,
        ThreadOverwrite,
        ObjectOverwrite,
        FunctionOverwrite
    };

public:
    explicit LMutex(QObject *parent = nullptr);
    void setSortFunction(bool (*function)(LMutexClient*, LMutexClient*) = lMutexLessThan);
    void clear();

    void setMaxReadersCount(int count);
    int maxReadersCount();

    void setMaxWritersCount(int count);
    int maxWritersCount();

    bool acquereForRead(QObject *object, const char *member, int priority = 0,
                        Overwrite overwrite = Overwrite::FunctionOverwrite);
    bool acquereForRead(QObject *object, std::function<void()> function, int priority = 0,
                        Overwrite overwrite = Overwrite::FunctionOverwrite);

    bool acquereForWrite(QObject *object, const char *member, int priority = 0,
                         Overwrite overwrite = Overwrite::FunctionOverwrite);
    bool acquereForWrite(QObject *object, std::function<void()> function, int priority = 0,
                         Overwrite overwrite = Overwrite::FunctionOverwrite);

    bool waitForRead(int priority = 0);
    bool waitForWrite(int priority = 0);

    void release();

    bool isAcquiredForRead();
    bool isAcquiredForWrite();

private:
    mutable QMutex m_mutex;
    int m_max_running_reader_clients_count;
    int m_max_running_writer_clients_count;
    bool (*m_lessThan)(LMutexClient*, LMutexClient*);
    LMutexClientList m_running_clients;
    LMutexClientList m_pre_running_clients;
    LMutexClientList m_queued_clients;

private:
    ~LMutex();

    bool wait(LMutexClient *client);
    bool acquire(LMutexClient *client);
    bool overwrite(LMutexClient *client);

    void runClients();
    void runClient(LMutexClient *client);

    bool isReady(LMutexClient *client);

    int runningClientsCount(LMutexClient *client, LMutexClient::Operation operation);
    int queuedClientsCount(LMutexClient::Operation operation);

    int indexOfRunningClient(int operation = -1);
};

#endif // LMUTEX_H
