#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "SpectrumMonitorThread.h"

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

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void fftSizeChanged();
    void bwValueChanged();
    void fsValueChanged();
    void fcValueChanged();
    void displayThreadValueUpdate();

private:
    Ui::MainWindow *ui;
    SpectrumMonitorThread dt;

};

#endif // MAINWINDOW_H
