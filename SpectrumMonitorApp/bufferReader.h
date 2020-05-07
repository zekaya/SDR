#ifndef BUFFERREADER_H
#define BUFFERREADER_H

#include <QThread>
#include <QtDebug>

#include <string>
#include "fft.h"
#include "pluto_receiver.h"
//#include "adrv9009_receiver.h"
#include "ui_mainwindow.h"

class BufferReader : public QThread
{
    Q_OBJECT

public:
    void stop();
    void setBW(double bwval);
    void setFS(double fsval);
    void setFC(double fcval);
    void setURI(char* urival);

signals:
    void sendToDemod(short* data, int size);
    void sendToSpectrum(short* data, int size);

private slots:
    void bwValueChanged(double newBWVal);
    void fsValueChanged(double newFSVal);
    void fcValueChanged(double newFCVal);
    void bufferSizeChanged(int newFCVal);
    void stopThread();

private:
//    ADRV9009Receiver rcvr;
    PlutoReceiver rcvr;
    double currentBW = 1;
    double currentFS = 2.4;
    double currentFC = 99.796;
    char URI[15] = "ip:192.168.2.1";
    short* data1;
    short* data2;
    int currentDataPtr = 1;
    bool threadActive = true;

    int bufferSize1 = 65536;
    int bufferSize2 = 65536;

    void run();

};

#endif // BUFFERREADER_H
