// copyright (c) 2019 hors<horsicq@gmail.com>
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

#include <QWidget>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QIODevice>
#include <QPainter>
#include <QPaintEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QApplication>
#include <QClipboard>

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    struct OPTIONS
    {
        qint64 nBaseAddress;
        qint64 nStartAddress;
        qint64 nStartSelectionAddress;
        qint64 nSizeOfSelection;
        QString sBackupFileName;
    };

    enum CURSOR_TYPE
    {
        CT_NONE=0,
        CT_HIWORD,
        CT_LOWORD,
        CT_ANSI
    };

    struct CURSOR_POSITION
    {
        qint64 nOffset;
        CURSOR_TYPE type;
    };

    struct STATE
    {
        qint64 nCursorAddress;
        qint64 nSelectionAddress;
        qint64 nSelectionOffset;
        qint64 nSelectionSize;
    };

    struct POS_INFO
    {
        qint64 nSelectionInitOffset;
        qint64 nSelectionStartOffset;
        qint64 nSelectionEndOffset;
        CURSOR_POSITION cursorPosition;
    };

    QHexView(QWidget *parent=nullptr);
    //    ~QHexView();
    QIODevice *getDevice() const;
    void setData(QIODevice *pDevice,OPTIONS *pOptions=nullptr);
    quint32 getBytesProLine() const;
    void setBytesProLine(const quint32 nBytesProLine);
    qint64 getBaseAddress() const;
    void setFont(const QFont &font);
    bool isAddressValid(qint64 nAddress);
    bool isOffsetValid(qint64 nOffset);
    void reload();
    STATE getState();
    bool setReadonly(bool bState);
    QByteArray readArray(qint64 nOffset,qint64 nSize);
    bool isEdited();
    void setEdited(bool bState);

private:
    enum SELECT_TYPE
    {
        ST_NOTSELECTED=0,
        ST_ONEBYTE,
        ST_BEGIN,
        ST_MID,
        ST_END
    };

    static char convertANSI(char cByte);
    static QString getFontName();

public slots:
    void goToAddress(qint64 nAddress);
    void _goToOffset(qint64 nOffset);
    void setSelection(qint64 nAddress,qint64 nSize);
    void selectAll();
    void setWidgetResizable(bool resizable); // hack
    void setWidget(QWidget *widget); // hack

private slots:
    void adjust();
    void init();
    CURSOR_POSITION getCursorPosition(QPoint pos);
    void updateBlink();
    void _initSelection(qint64 nOffset);
    void _setSelection(qint64 nOffset);
    SELECT_TYPE getSelectType(qint64 nOffset);
    qint64 addressToOffset(qint64 nAddress);
    qint64 offsetToAddress(qint64 nOffset);
    QPoint cursorToPoint(CURSOR_POSITION cp);
    bool readByte(qint64 nOffset,quint8 *pByte);
    bool writeByte(qint64 nOffset,quint8 *pByte);
    void _customContextMenu(const QPoint &pos);

signals:
    void cursorPositionChanged();
    void errorMessage(QString sText);
    void customContextMenu(const QPoint &pos);
    void editState(bool bState);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    QIODevice *pDevice;
    qint64 _nBaseAddress;
    qint32 _nXOffset;
    qint32 _nBytesProLine;
    qint32 _nCharWidth;
    qint32 _nCharHeight;
    qint32 _nLinesProPage;
    qint32 _nDataBlockSize;
    qint64 _nStartOffset;
    qint32 _nLineHeight;
    qint32 _nAddressPosition;
    qint32 _nAddressWidth;
    qint32 _nAddressWidthCount;
    qint32 _nHexPosition;
    qint32 _nHexWidth;
    qint32 _nAnsiPosition;
    qint32 _nAnsiWidth;
    qint32 _nTotalLineCount;
    qint64 _nDataSize;
    QByteArray _baDataBuffer;
    QByteArray _baDataHexBuffer;

    qint32 _nLineDelta;

    bool _bBlink;
    QTimer timerCursor;
    QRect _rectCursor;

    POS_INFO posInfo;

    bool _bMouseSelection;

    bool bReadonly;
    bool bIsEdited;
    QString sBackupFileName; // TODO options
};

#endif // QHEXVIEW_H
