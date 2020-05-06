#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <QObject>

class demodulator : public QObject
{
    Q_OBJECT
public:
    explicit demodulator(QObject *parent = nullptr);

signals:

};

#endif // DEMODULATOR_H
