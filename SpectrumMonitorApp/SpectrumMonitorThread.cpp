#include "SpectrumMonitorThread.h"
#include <algorithm>

void SpectrumMonitorThread::initialize(Ui_MainWindow* mw)
{
    lmw = mw;
}

void SpectrumMonitorThread::stop()
{
    rcvr.shutdown();
    qv_x.clear();
    qv_y.clear();
    qDebug() << "Receiver shutdown.";
}

void SpectrumMonitorThread::run()
{
    int i,ogFFTSize;
    double real_val;
    double imag_val;
    double abs_val, max_val, min_val,tmp_max_val;
    QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
    double* ogData;

    data1 = (double*)malloc(2*fftSize1*sizeof(double));
    data2 = (double*)malloc(2*fftSize2*sizeof(double));
    ogData = data1;
    currentDataPtr = 1;

    max_val = 20;
    ogFFTSize = fftSize1;

    FFTProcessor fftProc;

    rcvr.setStartupParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));

    rcvr.CreateReceiver(URI,fftSize1);

    qDebug() << "SDR Receiver has been created...";

    while(threadActive)
    {
        if (currentDataPtr == 1)
        {
            ogData = data1;
            ogFFTSize = fftSize1;
        }
        else
        {
            ogData = data2;
            ogFFTSize = fftSize2;
        }

        rcvr.pollRXBuffer(ogData);
        fftProc.fft(ogData,ogFFTSize);

        qv_x.clear();
        qv_y.clear();

        for (i = 0; i < ogFFTSize; i++)
        {
            real_val = (double)REAL(ogData,i);
            imag_val = (double)IMAG(ogData,i);
            abs_val = sqrt(real_val*real_val+imag_val*imag_val)/ogFFTSize;

            if (i<ogFFTSize/2)
            {
                qv_x.append((MHZ(currentFC)-MHZ(currentBW)/2) + (MHZ(currentFS)/ogFFTSize)*(i+ogFFTSize/2));
            }
            else if(i>=ogFFTSize/2)
            {
                qv_x.append((MHZ(currentFC)-MHZ(currentBW)/2) + (MHZ(currentFS)/ogFFTSize)*(i-ogFFTSize/2));
            }

            qv_y.append(abs_val);
        }

        lmw->plot->xAxis->setRange((MHZ(currentFC)-MHZ(currentBW)/2),(MHZ(currentFC)+MHZ(currentBW)/2));

        tmp_max_val = *std::max_element(qv_y.constBegin(), qv_y.constEnd());
        min_val = 0;

        if(tmp_max_val > max_val)
        {
            max_val = tmp_max_val;
        }

        lmw->plot->yAxis->setRange(min_val,max_val);
        lmw->plot->graph(0)->setData(qv_x,qv_y);

        emit valueUpdate();

        QThread::usleep(40000);
    }
}

void SpectrumMonitorThread::setBW(double bwval)
{
    currentBW = bwval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}

void SpectrumMonitorThread::setFS(double fsval)
{
    currentFS = fsval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}

void SpectrumMonitorThread::setFC(double fcval)
{
    currentFC = fcval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}

void SpectrumMonitorThread::setFFTSize(int fftSizeVal)
{
   if(currentDataPtr == 1)
   {
       fftSize2 = fftSizeVal;
       free(data2);
       data2 = (double*)malloc(2*fftSize2*sizeof(double));
       rcvr.updateBufferSize(fftSize2);
       currentDataPtr = 2;       
   }
   else if(currentDataPtr == 2)
   {
       fftSize1 = fftSizeVal;
       free(data1);
       data1 = (double*)malloc(2*fftSize1*sizeof(double));
       rcvr.updateBufferSize(fftSize1);
       currentDataPtr = 1;
   }

}

void SpectrumMonitorThread::setURI(char* urival)
{
    strncpy(URI,urival,sizeof(&urival));
}

void SpectrumMonitorThread::fftValueChanged(int newFFTVal)
{
    setFFTSize(newFFTVal);
}

void SpectrumMonitorThread::bwValueChanged(double newBWVal)
{
    setBW(newBWVal);
}

void SpectrumMonitorThread::fsValueChanged(double newFSVal)
{
    setFS(newFSVal);
}

void SpectrumMonitorThread::fcValueChanged(double newFCVal)
{
    setFC(newFCVal);
}

void SpectrumMonitorThread::stopThread()
{
    threadActive = false;
    quit();
    stop();
}
