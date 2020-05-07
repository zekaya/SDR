QT += charts multimedia widgets printsupport

HEADERS += \
    adrv9009_receiver.h \
    bufferReader.h \
    demodulator.h \
    fft.h \
    mainwindow.h \
    pluto_receiver.h \
    qcustomplot.h \
    spectrumMonitor.h

SOURCES += \
    adrv9009_receiver.cpp \
    bufferReader.cpp \
    demodulator.cpp \
    fft.cpp \
    main.cpp\
    mainwindow.cpp \
    pluto_receiver.cpp \
    qcustomplot.cpp \
    spectrumMonitor.cpp

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -lgsl

unix:!macx: LIBS += -lcblas

unix:!macx: LIBS += -latlas

unix:!macx: LIBS += -lm

unix:!macx: LIBS += -liio

unix:!macx: LIBS += -lad9361

FORMS += \
    mainwindow.ui
