#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <QObject>
#include <QThread>
#include <iostream>
#include <string>
#include <complex>
#include <limits.h>
#include <math.h>
#include <fstream>

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

enum demodTypes { FM, AM, SSB };

using namespace std;
using namespace std::complex_literals;

class Demodulator : public QThread
{
    Q_OBJECT
public:
    Demodulator();
    ~Demodulator();
    void setModulation(char newmod);
    void setCenterFreq(int newfc);
    void setReceiverFreq(int newfr);
    void demodulate(short* data, int size);

private slots:
    void demodStart();
    void demodStop();
    void demodTypeChanged(demodTypes newDemodType);
    void fillBuffer(short* data, int size);

private:

    short* data;
    int bufferSize;

    bool newDataArrived = false;
    bool demodulationDone = true;
    FILE* f;
    bool demodActive = false;
    int currentDemodType = FM;
    void run();
};

#endif // DEMODULATOR_H
