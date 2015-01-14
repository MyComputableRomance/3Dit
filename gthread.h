#ifndef GTHREAD_H
#define GTHREAD_H

#include <QThread>
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <QFile>
#include "waitdialog.h"

class GThread : public QThread
{
    Q_OBJECT
public:
    explicit GThread(QObject *parent = 0);
    GThread(bool pre, QObject *parent = 0);
    ~GThread();
    void getCmd(QStringList q);
    void run();
    void quit();

signals:
    void drawPreview();

public slots:
    void checkBuffer();

private:
    bool preview;
    bool flag;
    WaitDialog* wd;
    QStringList qsl;
    QProcess* qp;
    QTimer* timer;
    int count;
};

#endif // GTHREAD_H
