#include "lmutex.h"



bool lMutexLessThan(LMutexClient *a, LMutexClient *b)
{
    if (a->access() == b->access()) {
        if (a->priority() == b->priority()) {
            if (a->operation() == b->operation())
                return a->dateTime() < b->dateTime();

            return a->operation() > b->operation();
        }

        return a->priority() > b->priority();
    }

    return a->access() > b->access();
}



LMutex::LMutex(QObject *parent) :
    QObject(parent),
    m_max_running_reader_clients_count(-1),
    m_max_running_writer_clients_count(1),
    m_lessThan(lMutexLessThan)
{
}



void LMutex::setSortFunction(bool (*function)(LMutexClient*, LMutexClient*))
{
    if (function != nullptr)
        m_lessThan = function;
}



LMutex::~LMutex()
{
    clear();
}



void LMutex::clear()
{
    QMutexLocker mutexLocker(&m_mutex);

    qDeleteAll(m_running_clients);
    qDeleteAll(m_pre_running_clients);
    qDeleteAll(m_queued_clients);

    m_running_clients.clear();
    m_pre_running_clients.clear();
    m_queued_clients.clear();
}



void LMutex::setMaxReadersCount(int count)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_max_running_reader_clients_count = count;
    runClients();
}



int LMutex::maxReadersCount()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_max_running_reader_clients_count;
}



void LMutex::setMaxWritersCount(int count)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_max_running_writer_clients_count = count;
    runClients();
}



int LMutex::maxWritersCount()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_max_running_writer_clients_count;
}



bool LMutex::acquereForRead(QObject *object, const char *member, int priority, Overwrite overwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    LMutexClient *client = new LMutexClient(LMutexClient::Read, object, member, priority, overwrite);
    return acquire(client);
}



bool LMutex::acquereForRead(QObject *object, std::function<void()> function, int priority, Overwrite overwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    LMutexClient *client = new LMutexClient(LMutexClient::Read, object, function, priority, overwrite);
    return acquire(client);
}



bool LMutex::acquereForWrite(QObject *object, const char *member, int priority, Overwrite overwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    LMutexClient *client = new LMutexClient(LMutexClient::Write, object, member, priority, overwrite);
    return acquire(client);
}



bool LMutex::acquereForWrite(QObject *object, std::function<void ()> function, int priority, Overwrite overwrite)
{
    QMutexLocker mutexLocker(&m_mutex);
    LMutexClient *client = new LMutexClient(LMutexClient::Write, object, function, priority, overwrite);
    return acquire(client);
}



bool LMutex::waitForRead(int priority)
{
    m_mutex.lock();
    LMutexClient *client = new LMutexClient(LMutexClient::Read, priority);
    return wait(client);
}



bool LMutex::waitForWrite(int priority)
{
    m_mutex.lock();
    LMutexClient *client = new LMutexClient(LMutexClient::Write, priority);
    return wait(client);
}



bool LMutex::wait(LMutexClient *client)
{
    if (acquire(client)) {
        m_mutex.unlock();
        return true;
    }

    m_mutex.unlock();
    client->lock();
    return false;
}



bool LMutex::acquire(LMutexClient *client)
{
    overwrite(client);

    if (isReady(client)) {
        runClient(client);
        return true;
    }

    m_queued_clients.append(client);
    std::sort(m_queued_clients.begin(), m_queued_clients.end(), m_lessThan);

    return false;
}



bool LMutex::overwrite(LMutexClient *client)
{
    bool overwriten = false;

    for (int i = 0; i < m_queued_clients.count(); ++i) {
        LMutexClient *c = m_queued_clients.at(i);

        if (c->access() == LMutexClient::Async
                && ((client->overwrite() == Overwrite::ThreadOverwrite && c->thread() == client->thread())
                    || (client->overwrite() == Overwrite::ObjectOverwrite
                        && (c->thread() == client->thread() && c->object() == client->object()))
                    || (client->overwrite() == Overwrite::FunctionOverwrite
                        && c->thread() == client->thread()
                        && c->object() == client->object()
                        && (c->member() == client->member()
                            || (c->function() > 0 && (c->function() == client->function())))))) {
            delete m_queued_clients.takeAt(i);
            --i;
            overwriten = true;
        }
    }

    return overwriten;
}



void LMutex::release()
{
    QMutexLocker mutexLocker(&m_mutex);

    const int index = indexOfRunningClient();
    if (index >= 0) {
        delete m_running_clients.takeAt(index);
        runClients();
    }
}



void LMutex::runClients()
{
    bool ready = true;
    while (ready && !m_queued_clients.isEmpty()) {
        LMutexClient *client = m_queued_clients.first();

        if (isReady(client)) {
            ready = true;
            runClient(client);
            m_queued_clients.removeFirst();
        }
        else
            ready = false;
    }
}



void LMutex::runClient(LMutexClient *client)
{
    if (client->access() == LMutexClient::Async) {
        m_pre_running_clients.append(client);
        connect(client, &LMutexClient::runned, this, [this, client]() {
            m_running_clients.append(client);
            m_pre_running_clients.removeOne(client);
        });
    }

    else
        m_running_clients.append(client);

    client->run();
}



bool LMutex::isReady(LMutexClient *client)
{
    if (client->operation() == LMutexClient::Read)
        return (m_max_running_reader_clients_count < 0
                 || (runningClientsCount(client, LMutexClient::Read) < m_max_running_reader_clients_count))
                && runningClientsCount(client, LMutexClient::Write) == 0
                && ((client->access() == LMutexClient::Async && queuedClientsCount(LMutexClient::Write) == 0)
                    || client->access() == LMutexClient::Sync);

    return (m_max_running_writer_clients_count < 0
          || (runningClientsCount(client, LMutexClient::Write) < m_max_running_writer_clients_count))
            && runningClientsCount(client, LMutexClient::Read) == 0;
}



int LMutex::runningClientsCount(LMutexClient *client, LMutexClient::Operation operation)
{
    int count = 0;

    for (LMutexClient *c : qAsConst(m_running_clients))
        if (c->operation() == operation)
            ++count;

    for (LMutexClient *c : qAsConst(m_pre_running_clients))
        if (c->operation() == operation
                && (client->access() == LMutexClient::Async
                    || (client->access() == LMutexClient::Sync && c->thread() != client->thread())))
            ++count;

    return count;
}



int LMutex::queuedClientsCount(LMutexClient::Operation operation)
{
    int count = 0;

    for (LMutexClient *client : qAsConst(m_queued_clients))
        if (client->operation() == operation)
            ++count;

    return count;
}



bool LMutex::isAcquiredForRead()
{
    return indexOfRunningClient() >= 0;
}



bool LMutex::isAcquiredForWrite()
{
    return indexOfRunningClient(LMutexClient::Write) >= 0;
}



int LMutex::indexOfRunningClient(int operation)
{
    const QThread *currentThread = QThread::currentThread();

    for (int i = 0; i < m_running_clients.count(); ++i) {
        LMutexClient *client = m_running_clients.at(i);
        if (client->thread() == currentThread && (operation == -1 || client->operation() == operation))
            return i;
    }

    return -1;
}
