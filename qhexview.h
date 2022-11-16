// copyright (c) 2019-2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef QHEXVIEW_H
#define QHEXVIEW_H

#include <QAbstractScrollArea>
#include <QApplication>
#include <QClipboard>
#include <QElapsedTimer>
#include <QIODevice>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QWidget>

#include "xbinary.h"

class QHexView : public QAbstractScrollArea {
    Q_OBJECT

public:
    struct OPTIONS {
        qint64 nStartAddress;
        qint64 nStartSelectionAddress;
        qint64 nSizeOfSelection;
        QString sBackupFileName;
        XBinary::_MEMORY_MAP memoryMap;
    };

    enum CURSOR_TYPE {
        CT_NONE = 0,
        CT_HIWORD,
        CT_LOWORD,
        CT_ANSI
    };

    struct CURSOR_POSITION {
        qint64 nOffset;
        CURSOR_TYPE type;
    };

    struct STATE {
        qint64 nCursorAddress;
        qint64 nCursorOffset;
        qint64 nSelectionAddress;
        qint64 nSelectionOffset;
        qint64 nSelectionSize;
    };

    struct POS_INFO {
        qint64 nSelectionInitOffset;
        qint64 nSelectionStartOffset;
        qint64 nSelectionEndOffset;
        CURSOR_POSITION cursorPosition;
    };

    QHexView(QWidget *pParent = nullptr);
    //    ~QHexView();
    QIODevice *getDevice() const;
    void setData(QIODevice *pDevice, OPTIONS *pOptions = nullptr);
    void setBackupFileName(QString sBackupFileName);
    quint32 getBytesProLine() const;
    void setBytesProLine(const quint32 nBytesProLine);
    //    qint64 getBaseAddress() const;
    void setFont(const QFont &font);
    bool isAddressValid(qint64 nAddress);
    bool isRelAddressValid(qint64 nRelAddress);
    bool isOffsetValid(qint64 nOffset);
    void reload();
    STATE getState();
    bool setReadonly(bool bState);
    QByteArray readArray(qint64 nOffset, qint64 nSize);
    bool isEdited();
    void setEdited(bool bState);
    qint64 getBaseAddress();

private:
    enum ST {
        ST_NOTSELECTED = 0,
        ST_ONEBYTE,
        ST_BEGIN,
        ST_MID,
        ST_END
    };

    static char convertANSI(char cByte);
    static QString getFontName();

public slots:
    void goToAddress(qint64 nAddress);
    void goToRelAddress(qint64 nRelAddress);
    void goToOffset(qint64 nOffset);
    void _goToOffset(qint64 nOffset);
    void setSelection(qint64 nAddress, qint64 nSize);
    void selectAll();
    void setWidgetResizable(bool resizable)
    {
        Q_UNUSED(resizable)
    }  // hack TODO Check
    void setWidget(QWidget *widget)
    {
        Q_UNUSED(widget)
    }  // hack TODO Check
    XBinary::_MEMORY_MAP *getMemoryMap();

private slots:
    void verticalScroll();
    void horisontalScroll();
    void adjust();
    void init();
    CURSOR_POSITION getCursorPosition(QPoint pos);
    void updateBlink();
    void _initSelection(qint64 nOffset);
    void _setSelection(qint64 nOffset);
    ST getSelectType(qint64 nOffset);
    qint64 addressToOffset(qint64 nAddress);
    qint64 relAddressToOffset(qint64 nRelAddress);
    qint64 offsetToAddress(qint64 nOffset);
    QPoint cursorToPoint(CURSOR_POSITION cp);
    bool readByte(qint64 nOffset, quint8 *pByte);
    bool writeByte(qint64 nOffset, quint8 *pByte);
    void _customContextMenu(const QPoint &pos);

signals:
    void cursorPositionChanged();
    void errorMessage(QString sText);
    void customContextMenu(const QPoint &pos);
    void editState(bool bState);

protected:
    virtual void paintEvent(QPaintEvent *pEvent);
    virtual void mouseMoveEvent(QMouseEvent *pEvent);
    virtual void mousePressEvent(QMouseEvent *pEvent);
    virtual void resizeEvent(QResizeEvent *pEvent);
    virtual void keyPressEvent(QKeyEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);

private:
    QIODevice *g_pDevice;
    qint32 g_nXOffset;
    qint32 g_nBytesProLine;
    qint32 g_nCharWidth;
    qint32 g_nCharHeight;
    qint32 g_nLinesProPage;
    qint32 g_nDataBlockSize;
    qint64 g_nStartOffset;
    qint64 g_nStartOffsetDelta;
    qint32 g_nLineHeight;
    qint32 g_nAddressPosition;
    qint32 g_nAddressWidth;
    qint32 g_nAddressWidthCount;
    qint32 g_nHexPosition;
    qint32 g_nHexWidth;
    qint32 g_nAnsiPosition;
    qint32 g_nAnsiWidth;
    qint32 g_nTotalLineCount;
    qint64 g_nDataSize;
    QByteArray g_baDataBuffer;
    QByteArray g_baDataHexBuffer;
    qint32 g_nLineDelta;
    bool g_bBlink;
    QTimer g_timerCursor;
    QRect g_rectCursor;
    POS_INFO g_posInfo;
    bool g_bMouseSelection;
    bool g_bReadonly;
    bool g_bIsEdited;
    QString g_sBackupFileName;
    XBinary::_MEMORY_MAP g_memoryMap;
};

#endif  // QHEXVIEW_H
