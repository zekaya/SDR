#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#include <QThread>
#include <QtDebug>

#include <string>
#include "fft.h"
#include "ui_mainwindow.h"

#define KHZ(x) ((long long)(x*1000.0 + .5))
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

class SpectrumMonitor : public QThread
{
    Q_OBJECT

public:
    void initialize(Ui_MainWindow* mw);
    void stop();
    void setBW(double bwval);
    void setFS(double fsval);
    void setFC(double fcval);
    void setFFTSize(int fftSizeVal);

signals:
    void valueUpdate();

private slots:
    void bwValueChanged(double newBWVal);
    void fftValueChanged(int newFFTVal);
    void fsValueChanged(double newFSVal);
    void fcValueChanged(double newFCVal);
    void stopThread();
    void fillBuffer(short* data, int size);

private:
    QVector<double> qv1_x, qv1_y, qv2_x, qv2_y, qv3_x, qv3_y, qv4_x, qv4_y;
    Ui_MainWindow* lmw = NULL;
    double currentBW = 1;
    double currentFS = 2.4;
    double currentFC = 99.796;
    int fftSize1 = 4096;
    int fftSize2 = 4096;
    short* data1;
    short* data2;
    double* fftResultArray1;
    double* fftResultArray2;
    int currentDataPtr = 1;
    bool threadActive = true;
    int bufferSize = 65536;

    void run();

};

#endif // SPECTRUMMONITOR_H
