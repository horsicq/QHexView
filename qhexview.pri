INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qhexview.h \
    $$PWD/qhexviewwidget.h \
    $$PWD/dialoggotoaddress.h

SOURCES += \
    $$PWD/qhexview.cpp \
    $$PWD/qhexviewwidget.cpp \
    $$PWD/dialoggotoaddress.cpp

FORMS += \
    $$PWD/qhexviewwidget.ui \
    $$PWD/dialoggotoaddress.ui

!contains(XCONFIG, qlineedithex) {
    XCONFIG += qlineedithex
    include(../Controls/qlineedithex.pri)
}

!contains(XCONFIG, dialogdump) {
    XCONFIG += dialogdump
    include(../FormatDialogs/dialogdump.pri)
}
