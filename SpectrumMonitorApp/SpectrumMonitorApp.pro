QT += charts multimedia widgets printsupport

HEADERS += \
    SpectrumMonitorThread.h \
    adrv9009_receiver.h \
    fft.h \
    mainwindow.h \
    pluto_receiver.h \
    qcustomplot.h \
    xyseriesiodevice.h

SOURCES += \
    SpectrumMonitorThread.cpp \
    adrv9009_receiver.cpp \
    fft.cpp \
    main.cpp\
    mainwindow.cpp \
    pluto_receiver.cpp \
    qcustomplot.cpp \
    xyseriesiodevice.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/charts/audio
INSTALLS += target

unix|win32: LIBS += -lgsl

unix:!macx: LIBS += -lcblas

unix:!macx: LIBS += -latlas

unix:!macx: LIBS += -lm

unix:!macx: LIBS += -liio

unix:!macx: LIBS += -lad9361

FORMS += \
    mainwindow.ui
