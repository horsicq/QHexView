INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qhexview.h \
    $$PWD/qhexviewwidget.h

SOURCES += \
    $$PWD/qhexview.cpp \
    $$PWD/qhexviewwidget.cpp

FORMS += \
    $$PWD/qhexviewwidget.ui

!contains(XCONFIG, xlineedithex) {
    XCONFIG += xlineedithex
    include($$PWD/../Controls/xlineedithex.pri)
}

!contains(XCONFIG, dialogdump) {
    XCONFIG += dialogdump
    include($$PWD/../FormatDialogs/dialogdump.pri)
}

!contains(XCONFIG, dialogsearch) {
    XCONFIG += dialogsearch
    include($$PWD/../FormatDialogs/dialogsearch.pri)
}

!contains(XCONFIG, dialoggotoaddress) {
    XCONFIG += dialoggotoaddress
    include($$PWD/../FormatDialogs/dialoggotoaddress.pri)
}

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include($$PWD/../Formats/xbinary.pri)
}

#!contains(XCONFIG, formatwidget) {
#    XCONFIG += formatwidget
#    include($$PWD/../FormatWidgets/formatwidget.pri)
#}
