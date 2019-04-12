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

!contains(XCONFIG, xlineedithex) {
    XCONFIG += xlineedithex
    include(../Controls/xlineedithex.pri)
}

!contains(XCONFIG, dialogdump) {
    XCONFIG += dialogdump
    include(../FormatDialogs/dialogdump.pri)
}
