#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "spectrumMonitor.h"
#include "demodulator.h"
#include "bufferReader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void fftValueToDisplayThread(int);
    void bwValueToDisplayThread(double);
    void fsValueToDisplayThread(double);
    void fcValueToDisplayThread(double);
    void stopDisplayThread();
    void stopBufferThread();
    void startDemodulation();
    void stopDemodulation();
    void demodTypeToDemodThread(demodTypes);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void demodulateButtonClicked();
    void fftSizeChanged();
    void bwValueChanged();
    void fsValueChanged();
    void fcValueChanged();
    void displayThreadValueUpdate();
    void demodTypeChanged();

private:
    Ui::MainWindow *ui;
    SpectrumMonitor sm;
    Demodulator demod;
    BufferReader br;

    bool isDemodulating = false;
    QPen pen1;
    QPen pen2;

};

#endif // MAINWINDOW_H
