#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QVBoxLayout>
#include "worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QVector<Worker*> m_workers;
    QVector<QThread*> m_threads;

private:
    QVBoxLayout *setWorker(QThread *thread, int index);
    void stopThread(QThread *thread);
};

#endif // MAINWINDOW_H
