#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spectrumMonitor.h"

#include <iostream>
#include <cmath>
#include <QThread>
#include <QtDebug>
#include <QString>
#include <QObject>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{    
    pen1.setStyle(Qt::DotLine);
    pen1.setWidth(2);
    pen1.setColor(Qt::red);

    pen2.setStyle(Qt::DotLine);
    pen2.setWidth(2);
    pen2.setColor(Qt::green);

    ui->setupUi(this);
    ui->plot->addGraph();
    ui->plot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsLine);
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(pen1);
    ui->plot->addGraph();
    ui->plot->graph(2)->setPen(pen2);
    ui->plot->addGraph();
    ui->plot->graph(3)->setPen(pen2);

    ui->plot->xAxis->setRange(0,255);
//    ui->plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->plot->yAxis->setRange(0,500);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->xAxis->setSubTicks(true);
    ui->plot->xAxis->setTickLabels(false);

    ui->fftComboBox->addItem("128");
    ui->fftComboBox->addItem("256");
    ui->fftComboBox->addItem("512");
    ui->fftComboBox->addItem("1024");
    ui->fftComboBox->addItem("2048");
    ui->fftComboBox->addItem("4096");
    ui->fftComboBox->addItem("8192");
    ui->fftComboBox->addItem("16384");
    ui->fftComboBox->addItem("32768");
    ui->fftComboBox->addItem("65536");
    ui->fftComboBox->setCurrentText("4096");

    ui->demodComboBox->addItem("FM");
    ui->demodComboBox->addItem("AM");
    ui->demodComboBox->addItem("SSB");

    // MainWindow to SpectrumMonitor connections
    connect(this,SIGNAL(fftValueToDisplayThread(int)),&sm,SLOT(fftValueChanged(int)));
    connect(this,SIGNAL(stopDisplayThread()),&sm,SLOT(stopThread()));
    connect(this,SIGNAL(bwValueToDisplayThread(double)),&sm,SLOT(bwValueChanged(double)));
    connect(this,SIGNAL(fcValueToDisplayThread(double)),&sm,SLOT(fcValueChanged(double)));
    connect(this,SIGNAL(fsValueToDisplayThread(double)),&sm,SLOT(fsValueChanged(double)));
    connect(this,SIGNAL(startfChanged(double)),&sm,SLOT(scanStartChanged(double)));
    connect(this,SIGNAL(stopfChanged(double)),&sm,SLOT(scanStopChanged(double)));
    connect(this,SIGNAL(scChanged(bool)),&sm,SLOT(scanChanged(bool)));

    // MainWindows to BufferReader connections
    connect(this,SIGNAL(stopBufferThread()),&br,SLOT(stopThread()));
    connect(this,SIGNAL(fsValueToDisplayThread(double)),&br,SLOT(fsValueChanged(double)));
    connect(this,SIGNAL(bwValueToDisplayThread(double)),&br,SLOT(bwValueChanged(double)));
    connect(this,SIGNAL(fcValueToDisplayThread(double)),&br,SLOT(fcValueChanged(double)));

    // MainWindows to Demodulator
    connect(this,SIGNAL(startDemodulation()),&demod,SLOT(demodStart()));
    connect(this,SIGNAL(stopDemodulation()),&demod,SLOT(demodStop()));
    connect(this,SIGNAL(demodTypeToDemodThread(demodTypes)),&demod,SLOT(demodTypeChanged(demodTypes)));

    // BufferReader to Demodulator connections
    connect(&br,SIGNAL(sendToDemod(short*,int)),&demod,SLOT(fillBuffer(short*,int)));

    // BufferReader to Demodulator connections
    connect(&br,SIGNAL(sendToSpectrum(short*,int)),&sm,SLOT(fillBuffer(short*,int)));

    // SpectrumMonitor to MainWindow connections
    connect(&sm,SIGNAL(valueUpdate()),this,SLOT(displayThreadValueUpdate()));

    // SpectrumMonitor to BufferReader connections
    connect(&sm,SIGNAL(updateFc(double)),&br,SLOT(fcValueChanged(double)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString uristring = ui->uriLineEdit->text();
    QByteArray array = uristring.toLocal8Bit();
    char* buffer = array.data();

    br.setURI(buffer);
    br.start();
    demod.start();
    sm.initialize(ui);
    sm.start();
}

void MainWindow::on_pushButton_2_clicked()
{
    emit stopDisplayThread();
    emit stopBufferThread();
}

void MainWindow::fftSizeChanged()
{
    int newFFTVal = ui->fftComboBox->currentText().toInt();
    qDebug() << "FFT Size has been changed: "<<newFFTVal;
    emit fftValueToDisplayThread(newFFTVal);
}

void MainWindow::fsValueChanged()
{
    double newFSVal = ui->spinbox_fs->value();
    qDebug() << "Sampling frequency has been changed: "<<newFSVal;
    emit fsValueToDisplayThread(newFSVal);
}

void MainWindow::bwValueChanged()
{
    double newBWVal = ui->spinbox_bw->value();
    qDebug() << "Bandwidth has been changed: "<<newBWVal;
    emit bwValueToDisplayThread(newBWVal);
}

void MainWindow::fcValueChanged()
{
    double newFCVal = ui->spinbox_fc->value();
    qDebug() << "Center frequency has been changed: "<<newFCVal;
    emit fcValueToDisplayThread(newFCVal);
}

void MainWindow::displayThreadValueUpdate()
{
    ui->plot->replot();
    ui->plot->repaint();
    ui->plot->rescaleAxes();
}

void MainWindow::demodulateButtonClicked()
{
    if(isDemodulating == false)
    {
        qDebug() << "Start demodulation...";
        isDemodulating = true;
        ui->pushButton_4->setText("Demodulating");
        emit startDemodulation();
    }
    else
    {
        qDebug() << "Stop demodulation...";
        isDemodulating = false;
        ui->pushButton_4->setText("Demodulate");
        emit stopDemodulation();
    }

}

void MainWindow::demodTypeChanged()
{
   demodTypes newDemodType = FM;
   QString demodText;
   demodText = ui->demodComboBox->currentText();

   if(demodText == "FM")
   {
        newDemodType = FM;
   }
   else if(demodText == "AM")
   {
        newDemodType = AM;
   }
   else if(demodText == "SSB")
   {
        newDemodType = SSB;
   }

   emit demodTypeToDemodThread(newDemodType);
}

void MainWindow::startFreqChanged()
{
    emit startfChanged(ui->spinbox_fstart->value());
}

void MainWindow::stopFreqChanged()
{
    emit stopfChanged(ui->spinbox_fstop->value());
}

void MainWindow::scanChanged()
{
    emit scChanged(ui->checkBox_scan->isChecked());
}
