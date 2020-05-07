#include "bufferReader.h"
#include <algorithm>

void BufferReader::stop()
{
    rcvr.shutdown();
    qDebug() << "Receiver shutdown.";
}

void BufferReader::run()
{
    int ogBufferSize;
    short* ogData;

    data1 = (short*)malloc(2*bufferSize1*sizeof(short));
    data2 = (short*)malloc(2*bufferSize2*sizeof(short));

    ogData = data1;
    currentDataPtr = 1;

    rcvr.setStartupParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
    rcvr.CreateReceiver(URI,bufferSize1);
    rcvr.updateBufferSize(bufferSize1);

    qDebug() << "SDR Receiver has been created...";

    while(threadActive)
    {
        if (currentDataPtr == 1)
        {
            if(data1)
            {
                ogData = data1;
                ogBufferSize = bufferSize1;
            }
            else
            {
                qDebug() << " Data 1 array doesn't exist!";
            }
        }
        else
        {
            if(data2)
            {
                ogData = data2;
                ogBufferSize = bufferSize2;
            }
            else
            {
                qDebug() << " Data 2 array doesn't exist!";
            }
        }

        // Read receive buffer
        rcvr.pollRXBuffer(ogData);
        emit sendToDemod(ogData,ogBufferSize);
        emit sendToSpectrum(ogData,ogBufferSize);
    }
}

void BufferReader::setBW(double bwval)
{
    currentBW = bwval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}

void BufferReader::setFS(double fsval)
{
    currentFS = fsval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}

void BufferReader::setFC(double fcval)
{
    currentFC = fcval;
    rcvr.updateParams(MHZ(currentBW),MHZ(currentFS),MHZ(currentFC));
}


void BufferReader::setURI(char* urival)
{
    strncpy(URI,urival,sizeof(&urival));
}

void BufferReader::bwValueChanged(double newBWVal)
{
    setBW(newBWVal);
}

void BufferReader::fsValueChanged(double newFSVal)
{
    setFS(newFSVal);
}

void BufferReader::fcValueChanged(double newFCVal)
{
    setFC(newFCVal);
}

void BufferReader::bufferSizeChanged(int newBufSize)
{
    if(currentDataPtr == 1)
    {
        bufferSize2 = newBufSize;
        free(data2);
        data2 = (short*)malloc(2*bufferSize2*sizeof(short));
        rcvr.updateBufferSize(bufferSize2);
        currentDataPtr = 2;
    }
    else if(currentDataPtr == 2)
    {
        bufferSize1 = newBufSize;
        free(data1);
        data1 = (short*)malloc(2*bufferSize1*sizeof(short));
        rcvr.updateBufferSize(bufferSize1);
        currentDataPtr = 1;
    }
}

void BufferReader::stopThread()
{
    threadActive = false;

    if(data1)
        free(data1);

    if(data2)
        free(data2);

    quit();
    stop();
}
