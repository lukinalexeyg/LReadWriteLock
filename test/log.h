#ifndef LOG_H
#define LOG_H

#pragma once

#include <QDebug>
#include <QThread>

#define FUNCTION_LOG Log __log__(__FUNCTION__);
#define DEBUG_LOG qDebug().noquote() << threadIdString() << __FUNCTION__ <<
#define INFO_LOG qInfo().noquote() << threadIdString() << __FUNCTION__ <<
#define FUNCTION_LINE QString("%1 %2:%3:").arg(threadIdString()).arg(__FUNCTION__).arg(__LINE__)
#define WARNING_LOG qWarning().noquote() << FUNCTION_LINE <<
#define CRITICAL_LOG qCritical().noquote() << FUNCTION_LINE <<
#define FATAL_LOG qFatal().noquote() << FUNCTION_LINE <<

QString threadIdString();

class Log
{
public:
    Log(const char *function) : m_function(function) { qDebug().noquote() << threadIdString() << "+" << function; }
    ~Log() { qDebug().noquote() << threadIdString() << "-" << m_function; }

private:
    const char *m_function;
};

#endif // LOG_H
