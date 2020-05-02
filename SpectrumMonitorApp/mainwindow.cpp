#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SpectrumMonitorThread.h"

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
    ui->setupUi(this);
    ui->plot->addGraph();
//    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsLine);
    ui->plot->xAxis->setRange(0,255);
    ui->plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->plot->yAxis->setRange(0,500);

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

    connect(&dt,SIGNAL(valueUpdate()),this,SLOT(displayThreadValueUpdate()));
    connect(this,SIGNAL(fftValueToDisplayThread(int)),&dt,SLOT(fftValueChanged(int)));
    connect(this,SIGNAL(stopDisplayThread()),&dt,SLOT(stopThread()));
    connect(this,SIGNAL(fsValueToDisplayThread(double)),&dt,SLOT(fsValueChanged(double)));
    connect(this,SIGNAL(bwValueToDisplayThread(double)),&dt,SLOT(bwValueChanged(double)));
    connect(this,SIGNAL(fcValueToDisplayThread(double)),&dt,SLOT(fcValueChanged(double)));

    dt.setURI(buffer);
    dt.initialize(ui);
    dt.start();
}

void MainWindow::on_pushButton_2_clicked()
{
    emit stopDisplayThread();
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
}

