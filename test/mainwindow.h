#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    QVBoxLayout *setWorker();
    void stopThread(QThread *thread);
};

#endif // MAINWINDOW_H
