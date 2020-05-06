#ifndef DISPLAYTHREAD_H
#define DISPLAYTHREAD_H

#include <QThread>
#include <QtDebug>

#include <string>
#include "fft.h"
#include "pluto_receiver.h"
//#include "adrv9009_receiver.h"
#include "ui_mainwindow.h"

class SpectrumMonitorThread : public QThread
{
    Q_OBJECT

public:
    void initialize(Ui_MainWindow* mw);
    void stop();
    void setBW(double bwval);
    void setFS(double fsval);
    void setFC(double fcval);
    void setFFTSize(int fftSizeVal);
    void setURI(char* urival);

signals:
    void valueUpdate();

private slots:
    void fftValueChanged(int newFFTVal);
    void bwValueChanged(double newBWVal);
    void fsValueChanged(double newFSVal);
    void fcValueChanged(double newFCVal);
    void stopThread();

private:
//    ADRV9009Receiver rcvr;
    PlutoReceiver rcvr;
    QVector<double> qv_x, qv_y;
    Ui_MainWindow* lmw = NULL;
    double currentBW = 10;
    double currentFS = 56;
    double currentFC = 100;
    int fftSize1 = 4096;
    int fftSize2 = 4096;
    char URI[15] = "ip:192.168.2.1";
    double* data1;
    double* data2;
    int currentDataPtr = 1;
    bool threadActive = true;

    void run();

};

#endif // DISPLAYTHREAD_H
