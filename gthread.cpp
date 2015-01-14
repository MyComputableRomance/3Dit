#include "gthread.h"
#include <QMessageBox>

GThread::GThread(QObject *parent) :
    QThread(parent)
{
    wd = new WaitDialog(0);
    qp = new QProcess(this);
    timer = new QTimer(this);
    count = 0;
    preview = false;
    flag = true;
}

GThread::GThread(bool pre, QObject *)
{
    wd = new WaitDialog(0);
    qp = new QProcess(this);
    timer = new QTimer(this);
    count = 0;
    preview = pre;
    flag = true;
}

GThread::~GThread()
{
    qp->close();
}

void GThread::getCmd(QStringList q)
{
    qsl = q;
}

void GThread::run()
{
    count = 0;

    wd->show();
    wd->clearText();
    wd->setProgress(0);

    qp->start("cmd",qsl);
    qp->waitForStarted();
    timer->singleShot(10,this,SLOT(checkBuffer()));
}

void GThread::quit()
{
    //When ends, delete the tmp stl file.

    wd->close();
}

void GThread::checkBuffer()
{
    bool status = qp->waitForFinished(10);
    if(qp->state() == QProcess::Running && !status){
        QByteArray qb = qp->readAll();
        QString stdOut = stdOut.fromLocal8Bit(qb.data());
        if(stdOut != ""){
            count ++;
            wd->setProgress(100 / 7 * count);
            wd->setText(stdOut);
        }
        timer->singleShot(10,this,SLOT(checkBuffer()));
    }
    else{
        wd->setProgress(100);
        wd->clearText();
        wd->close();
        if (preview && flag){
            emit drawPreview();
        }
        flag = !flag;
    }
}
