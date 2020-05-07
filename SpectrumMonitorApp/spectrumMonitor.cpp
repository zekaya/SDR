#include "spectrumMonitor.h"

void SpectrumMonitor::initialize(Ui_MainWindow* mw)
{
    lmw = mw;
    data1 = (short*)malloc(2*fftSize1*sizeof(short));
    data2 = (short*)malloc(2*fftSize2*sizeof(short));
    fftResultArray1 = (double*)malloc(2*fftSize1*sizeof(double));
    fftResultArray2 = (double*)malloc(2*fftSize2*sizeof(double));
    currentDataPtr = 1;
}

void SpectrumMonitor::stop()
{
    qv1_x.clear();
    qv1_y.clear();
    qv2_x.clear();
    qv2_y.clear();
    qv3_x.clear();
    qv3_y.clear();
    qv4_x.clear();
    qv4_y.clear();
    qDebug() << "Receiver shutdown.";
}

void SpectrumMonitor::run()
{
    int i,ogFFTSize;
    double real_val;
    double imag_val;
    double abs_val, max_val, min_val,tmp_max_val;
    short* ogData;
    double* ogFFTResult;
    FFTProcessor fftProc;

    ogData = data1;   
    ogFFTSize = fftSize1;

    max_val = 20;

    qDebug() << "SDR Receiver has been created...";

    while(threadActive)
    {
        if (currentDataPtr == 1)
        {
            ogData = data1;
            ogFFTResult = fftResultArray1;
            ogFFTSize = fftSize1;
        }
        else
        {
            ogData = data2;
            ogFFTResult = fftResultArray2;
            ogFFTSize = fftSize2;
        }

        // Do the FFT
        fftProc.fft(ogData, ogFFTResult, ogFFTSize);

        qv1_x.clear();
        qv1_y.clear();
        qv2_x.clear();
        qv2_y.clear();
        qv3_x.clear();
        qv3_y.clear();
        qv4_x.clear();
        qv4_y.clear();

        //Plot FFT Results
        //Prepare plot vectors
        for (i = 0; i < ogFFTSize; i++)
        {
            real_val = REAL(ogFFTResult,i);
            imag_val = IMAG(ogFFTResult,i);
            abs_val = sqrt(real_val*real_val+imag_val*imag_val)/ogFFTSize;

            if (i<ogFFTSize/2)
            {
                qv1_x.append((MHZ(currentFC)-MHZ(currentFS)/2) + (MHZ(currentFS)/ogFFTSize)*(i+ogFFTSize/2));
            }
            else if(i>=ogFFTSize/2)
            {
                qv1_x.append((MHZ(currentFC)-MHZ(currentFS)/2) + (MHZ(currentFS)/ogFFTSize)*(i-ogFFTSize/2));
            }

            qv1_y.append(abs_val);
        }

        // Set plot range
        lmw->plot->xAxis->setRange((MHZ(currentFC)-MHZ(currentFS)/2),(MHZ(currentFC)+MHZ(currentFS)/2));

        tmp_max_val = *std::max_element(qv1_y.constBegin(), qv1_y.constEnd());
        min_val = 0;

        if(tmp_max_val > max_val)
        {
            max_val = tmp_max_val;
        }

        lmw->plot->yAxis->setRange(min_val,max_val);

        qv2_x.append(MHZ(currentFC));
        qv2_y.append(max_val);
        qv2_x.append(MHZ(currentFC));
        qv2_y.append(min_val);

        qv3_x.append((MHZ(currentFC)-MHZ(currentBW)/2));
        qv3_y.append(max_val);
        qv3_x.append((MHZ(currentFC)-MHZ(currentBW)/2));
        qv3_y.append(min_val);

        qv4_x.append((MHZ(currentFC)+MHZ(currentBW)/2));
        qv4_y.append(max_val);
        qv4_x.append((MHZ(currentFC)+MHZ(currentBW)/2));
        qv4_y.append(min_val);

        // Plot vectors
        lmw->plot->graph(0)->setData(qv1_x,qv1_y);
        lmw->plot->graph(1)->setData(qv2_x,qv2_y);
        lmw->plot->graph(2)->setData(qv3_x,qv3_y);
        lmw->plot->graph(3)->setData(qv4_x,qv4_y);


        emit valueUpdate();

        QThread::usleep(40000);
    }
}

void SpectrumMonitor::setBW(double bwval)
{
    currentBW = bwval;
}

void SpectrumMonitor::setFS(double fsval)
{
    currentFS = fsval;
}

void SpectrumMonitor::setFC(double fcval)
{
    currentFC = fcval;
}

void SpectrumMonitor::setFFTSize(int fftSizeVal)
{
   if(currentDataPtr == 1)
   {
       fftSize2 = fftSizeVal;
       free(data2);
       data2 = (short*)malloc(2*fftSize2*sizeof(short));
       free(fftResultArray2);
       fftResultArray2 = (double*)malloc(2*fftSize2*sizeof(double));
       currentDataPtr = 2;       
   }
   else if(currentDataPtr == 2)
   {
       fftSize1 = fftSizeVal;
       free(data1);
       data1 = (short*)malloc(2*fftSize1*sizeof(short));
       free(fftResultArray1);
       fftResultArray1 = (double*)malloc(2*fftSize1*sizeof(double));
       currentDataPtr = 1;
   }
}

void SpectrumMonitor::fftValueChanged(int newFFTVal)
{
    setFFTSize(newFFTVal);
}

void SpectrumMonitor::bwValueChanged(double newBWVal)
{
    setBW(newBWVal);
}

void SpectrumMonitor::fsValueChanged(double newFSVal)
{
    setFS(newFSVal);
}

void SpectrumMonitor::fcValueChanged(double newFCVal)
{
    setFC(newFCVal);
}

void SpectrumMonitor::stopThread()
{
    threadActive = false;
    quit();
    stop();
}

void SpectrumMonitor::fillBuffer(short* newData, int newSize)
{
    bufferSize = newSize;

    if(currentDataPtr == 1)
    {
        memcpy(data1, newData, 2*fftSize1*sizeof(short));
    }
    else if(currentDataPtr == 2)
    {
        memcpy(data2, newData, 2*fftSize2*sizeof(short));
    }
}
