// copyright (c) 2019-2022 hors<horsicq@gmail.com>
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
#include "qhexview.h"

QHexView::QHexView(QWidget *pParent) : QAbstractScrollArea(pParent)
{
    g_pDevice = nullptr;

    g_bReadonly = true;
    g_bIsEdited = false;

    g_bMouseSelection = false;
    g_nDataSize = 0;
    g_bBlink = false;
    g_nBytesProLine = 0;

    g_nStartOffset = 0;
    g_nStartOffsetDelta = 0;

    setBytesProLine(16);
    _initSelection(-1);
    g_nLineDelta = 4;  // mb 3
    g_posInfo.cursorPosition.nOffset = 0;
    g_posInfo.cursorPosition.type = CT_HIWORD;

#ifdef Q_OS_WIN
    setFont(QFont("Courier", 10));
#endif
#ifdef Q_OS_LINUX
    setFont(QFont("Monospace", 10));
#endif
#ifdef Q_OS_OSX
    setFont(QFont("Courier", 10));  // TODO Check "Menlo"
#endif

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScroll()));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(horisontalScroll()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(_customContextMenu(QPoint)));

    connect(&g_timerCursor, SIGNAL(timeout()), this, SLOT(updateBlink()));

    g_timerCursor.setInterval(500);
    g_timerCursor.start();
}

// QHexView::~QHexView()
//{

//}

QIODevice *QHexView::getDevice() const
{
    return g_pDevice;
}

void QHexView::setData(QIODevice *pDevice, OPTIONS *pOptions)
{
    this->g_pDevice = pDevice;

    if (pOptions) {
        this->g_sBackupFileName = pOptions->sBackupFileName;
        this->g_memoryMap = pOptions->memoryMap;
    }

    if (this->g_memoryMap.listRecords.count() == 0) {
        XBinary binary(pDevice);
        this->g_memoryMap = binary.getMemoryMap();
    }

    init();

    adjust();
    viewport()->update();

    if (pOptions) {
        goToAddress(pOptions->nStartAddress);

        if (pOptions->nSizeOfSelection) {
            if (isAddressValid(pOptions->nStartSelectionAddress)) {
                _initSelection(addressToOffset(pOptions->nStartSelectionAddress));
                _setSelection(addressToOffset(pOptions->nStartSelectionAddress) + pOptions->nSizeOfSelection - 1);
            }
        }
    }

    emit cursorPositionChanged();
}

void QHexView::setBackupFileName(QString sBackupFileName)
{
    this->g_sBackupFileName = sBackupFileName;
}

void QHexView::paintEvent(QPaintEvent *pEvent)
{
    //    QElapsedTimer timer;
    //    timer.start();

    QPainter painter(viewport());
    QFont fontNormal = painter.font();
    QFont fontBold = fontNormal;
    fontBold.setBold(true);

    painter.setPen(viewport()->palette().color(QPalette::WindowText));

    if (g_rectCursor != pEvent->rect()) {
        //    qDebug("void QHexView::paintEvent(QPaintEvent *event)");
        qint32 topLeftY = pEvent->rect().topLeft().y();
        qint32 topLeftX = pEvent->rect().topLeft().x() - g_nXOffset;

        painter.setPen(QPen(Qt::gray));

        for (qint32 i = 0; i < g_nLinesProPage; i++) {
            qint64 nLineAddress = XBinary::offsetToAddress(&g_memoryMap, g_nStartOffset + i * g_nBytesProLine);

            if (nLineAddress != -1) {
                qint32 nLinePosition = topLeftY + (i + 1) * g_nLineHeight;
                QString sLineAddress = QString("%1").arg(nLineAddress, g_nAddressWidthCount, 16, QChar('0'));
                painter.drawText(topLeftX + g_nAddressPosition, nLinePosition, sLineAddress);
            }
        }

        painter.setPen(viewport()->palette().color(QPalette::WindowText));
        painter.setBackgroundMode(Qt::TransparentMode);

        // HEX
        bool bIsSelected = false;
        QColor color = viewport()->palette().color(QPalette::Base);

        qint32 nDataBufferSize = g_baDataBuffer.size();

        for (qint32 i = 0; i < g_nLinesProPage; i++) {
            qint32 nLinePosition = topLeftY + (i + 1) * g_nLineHeight;

            for (qint32 j = 0; j < g_nBytesProLine; j++) {
                bool _bIsSelected = false;

                qint32 nBytePositionHEX = topLeftX + g_nHexPosition + j * g_nCharWidth * 3;
                qint32 nBytePositionANSI = topLeftX + g_nAnsiPosition + j * g_nCharWidth;
                qint32 nIndex = (j + i * g_nBytesProLine);
                QString sHex = g_baDataHexBuffer.mid(nIndex * 2, 2);
                char ch = ' ';

                if (nIndex < nDataBufferSize) {
                    ch = g_baDataBuffer.at(nIndex);
                    ch = convertANSI(ch);
                }

                ST st = getSelectType(g_nStartOffset + nIndex);

                _bIsSelected = !(st == ST_NOTSELECTED);

                if (bIsSelected != _bIsSelected) {
                    bIsSelected = _bIsSelected;

                    if (st == ST_NOTSELECTED) {
                        color = viewport()->palette().color(QPalette::Base);
                        //                        painter.setPen(viewport()->palette().color(QPalette::WindowText));
                    } else {
                        color = viewport()->palette().color(QPalette::Highlight);
                        //                        painter.setPen(viewport()->palette().color(QPalette::WindowText));
                    }
                }

                int nCount = 3;

                if ((st == ST_END) || (st == ST_ONEBYTE) || ((st != ST_NOTSELECTED) && (j == g_nBytesProLine - 1))) {
                    nCount = 2;
                }

                // TODO flag changed if selected
                QRect rect;
                rect.setRect(nBytePositionHEX, nLinePosition - g_nLineHeight + g_nLineDelta, g_nCharWidth * nCount, g_nLineHeight);  // mb TODO fix 3
                painter.fillRect(rect, color);
                rect.setRect(nBytePositionANSI, nLinePosition - g_nLineHeight + g_nLineDelta, g_nCharWidth, g_nLineHeight);
                painter.fillRect(rect, color);

                bool bBold = (sHex != "00");

                if (bBold) {
                    painter.setFont(fontBold);
                }

                painter.drawText(nBytePositionHEX, nLinePosition, sHex);
                painter.drawText(nBytePositionANSI, nLinePosition, QChar(ch));

                if (bBold) {
                    // Restore
                    painter.setFont(fontNormal);
                }
            }
        }

        painter.setBackgroundMode(Qt::TransparentMode);
        painter.setPen(viewport()->palette().color(QPalette::WindowText));
    }

    if (g_posInfo.cursorPosition.nOffset != -1) {
        if (g_bBlink && hasFocus()) {
            painter.setPen(viewport()->palette().color(QPalette::Highlight));
            painter.fillRect(g_rectCursor, this->palette().color(QPalette::WindowText));
        } else {
            painter.setPen(viewport()->palette().color(QPalette::WindowText));
            painter.fillRect(g_rectCursor, this->palette().color(QPalette::Base));
        }

        qint32 nRelOffset = g_posInfo.cursorPosition.nOffset - g_nStartOffset;

        if (g_posInfo.cursorPosition.type == CT_ANSI) {
            char ch = ' ';

            if (nRelOffset < g_baDataBuffer.size()) {
                ch = g_baDataBuffer.at(nRelOffset);
                ch = convertANSI(ch);
            }

            painter.drawText(g_rectCursor.x(), g_rectCursor.y() + g_nLineHeight - g_nLineDelta, QChar(ch));
        } else if (g_posInfo.cursorPosition.type == CT_HIWORD) {
            painter.drawText(g_rectCursor.x(), g_rectCursor.y() + g_nLineHeight - g_nLineDelta, g_baDataHexBuffer.mid(nRelOffset * 2, 1));
        } else if (g_posInfo.cursorPosition.type == CT_LOWORD) {
            painter.drawText(g_rectCursor.x(), g_rectCursor.y() + g_nLineHeight - g_nLineDelta, g_baDataHexBuffer.mid(nRelOffset * 2 + 1, 1));
        }
    }

    //    qDebug("QHexView::paintEvent: %d msec",timer.elapsed());
}

void QHexView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if (g_bMouseSelection) {
        viewport()->update();

        int nPos = getCursorPosition(pEvent->pos()).nOffset;

        if (nPos >= 0) {
            _setSelection(nPos);
            emit cursorPositionChanged();
            //        _rectCursor=QRect(0,0,100,100);
        }
    }
}

void QHexView::mousePressEvent(QMouseEvent *pEvent)
{
    g_bMouseSelection = false;

    if (pEvent->button() == Qt::LeftButton) {
        //        viewport()->update();
        CURSOR_POSITION cp = getCursorPosition(pEvent->pos());

        if (cp.nOffset >= 0) {
            g_posInfo.cursorPosition = cp;
            _initSelection(cp.nOffset);
            //        _rectCursor=QRect(0,0,100,100);
            g_bMouseSelection = true;
        }

        adjust();
        g_bBlink = true;
        viewport()->update();  // mb TODO
    }
}

quint32 QHexView::getBytesProLine() const
{
    return g_nBytesProLine;
}

void QHexView::setBytesProLine(const quint32 nBytesProLine)
{
    g_nBytesProLine = nBytesProLine;
}

// qint64 QHexView::getBaseAddress() const
//{
//     return _nBaseAddress;
// }

void QHexView::setFont(const QFont &font)
{
    const QFontMetricsF fm(font);
    g_nCharWidth = fm.boundingRect('2').width();
    g_nCharWidth = qMax(fm.boundingRect('W').width(), (qreal)g_nCharWidth);
    g_nCharHeight = fm.height();

    QAbstractScrollArea::setFont(font);

    adjust();
    viewport()->update();
}

bool QHexView::isAddressValid(qint64 nAddress)
{
    //    return ((_nBaseAddress<=nAddress)&&(nAddress<_nDataSize+_nBaseAddress));
    return XBinary::isAddressValid(&g_memoryMap, nAddress);
}

bool QHexView::isRelAddressValid(qint64 nRelAddress)
{
    return XBinary::isRelAddressValid(&g_memoryMap, nRelAddress);
}

bool QHexView::isOffsetValid(qint64 nOffset)
{
    //    return ((0<=nOffset)&&(nOffset<_nDataSize));
    return XBinary::isOffsetValid(&g_memoryMap, nOffset);
}

void QHexView::reload()
{
    adjust();
    viewport()->update();
}

QHexView::STATE QHexView::getState()
{
    STATE state = STATE();

    state.nCursorOffset = g_posInfo.cursorPosition.nOffset;
    state.nCursorAddress = offsetToAddress(g_posInfo.cursorPosition.nOffset);

    if (state.nCursorAddress == -1) {
        state.nCursorAddress = 0;
    }

    if (g_posInfo.nSelectionStartOffset != -1) {
        state.nSelectionOffset = g_posInfo.nSelectionStartOffset;
        state.nSelectionAddress = offsetToAddress(g_posInfo.nSelectionStartOffset);
        state.nSelectionSize = g_posInfo.nSelectionEndOffset - g_posInfo.nSelectionStartOffset + 1;
    } else {
        //        state.nCursorAddress=0;
        state.nSelectionOffset = 0;
        state.nSelectionAddress = 0;
        state.nSelectionSize = 0;
    }

    return state;
}

bool QHexView::setReadonly(bool bState)
{
    bool bResult = false;

    if (g_pDevice) {
        if ((bState) || ((!bState) && (g_pDevice->isWritable()))) {
            g_bReadonly = bState;
            bResult = true;
        }
    }

    return bResult;
}

QByteArray QHexView::readArray(qint64 nOffset, qint64 nSize)
{
    QByteArray baResult;

    if (g_pDevice->seek(nOffset) && (nOffset + nSize <= g_pDevice->size())) {
        baResult.resize((qint32)nSize);
        qint64 _nSize = g_pDevice->read(baResult.data(), nSize);

        if (_nSize != nSize) {
            baResult.resize((qint32)_nSize);
        }
    }

    return baResult;
}

bool QHexView::isEdited()
{
    return g_bIsEdited;
}

void QHexView::setEdited(bool bState)
{
    this->g_bIsEdited = bState;
}

qint64 QHexView::getBaseAddress()
{
    return this->getMemoryMap()->nModuleAddress;
}

char QHexView::convertANSI(char cByte)
{
    if ((cByte < 0x20) || (cByte > 0x7e)) {
        cByte = '.';
    }

    return cByte;
}

QString QHexView::getFontName()
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = "Courier";
#endif
#ifdef Q_OS_LINUX
    sResult = "Monospace";
#endif
#ifdef Q_OS_OSX
    sResult = "Courier";  // TODO Check "Menlo"
#endif
    return sResult;
}

void QHexView::goToAddress(qint64 nAddress)
{
    if ((isAddressValid(nAddress)) && (g_nBytesProLine)) {
        qint64 nOffset = addressToOffset(nAddress);

        goToOffset(nOffset);
    }
}

void QHexView::goToRelAddress(qint64 nRelAddress)
{
    if ((isRelAddressValid(nRelAddress)) && (g_nBytesProLine)) {
        qint64 nOffset = relAddressToOffset(nRelAddress);

        goToOffset(nOffset);
    }
}

void QHexView::goToOffset(qint64 nOffset)
{
    if ((isOffsetValid(nOffset)) && (g_nBytesProLine)) {
        verticalScrollBar()->setValue((nOffset) / g_nBytesProLine);  // TODO check for large files
        g_nStartOffsetDelta = (nOffset) % g_nBytesProLine;

        //        posInfo.cursorPosition.nOffset+=(addressToOffset(nAddress))%_nBytesProLine;
        //        posInfo.cursorPosition.nOffset=addressToOffset(nAddress);
        //        qDebug(QString::number(posInfo.cursorPosition.nOffset,16).toLatin1().data());
        g_posInfo.cursorPosition.nOffset = nOffset;
        g_posInfo.cursorPosition.type = CT_HIWORD;
        //        qDebug(QString::number(posInfo.cursorPosition.nOffset,16).toLatin1().data());
    }
}

void QHexView::_goToOffset(qint64 nOffset)
{
    if ((isOffsetValid(nOffset)) && (g_nBytesProLine)) {
        verticalScrollBar()->setValue((nOffset) / g_nBytesProLine);
        g_nStartOffsetDelta = (nOffset) % g_nBytesProLine;
    }
}

void QHexView::setSelection(qint64 nAddress, qint64 nSize)
{
    if (nSize) {
        if (isAddressValid(nAddress)) {
            qint64 nOffset = addressToOffset(nAddress);
            _initSelection(nOffset);
            _setSelection(nOffset + nSize - 1);

            viewport()->update();
        }
    }
}

void QHexView::selectAll()
{
    setSelection(g_memoryMap.nModuleAddress, g_nDataSize);
}

XBinary::_MEMORY_MAP *QHexView::getMemoryMap()
{
    return &g_memoryMap;
}

void QHexView::verticalScroll()
{
    //    _nStartOffsetDelta=0;
    adjust();
}

void QHexView::horisontalScroll()
{
    adjust();
}

QHexView::ST QHexView::getSelectType(qint64 nOffset)
{
    ST result = ST_NOTSELECTED;

    if (nOffset == g_posInfo.nSelectionStartOffset) {
        if (nOffset == g_posInfo.nSelectionEndOffset) {
            result = ST_ONEBYTE;
        } else {
            result = ST_BEGIN;
        }
    } else if (nOffset == g_posInfo.nSelectionEndOffset) {
        result = ST_END;
    } else if ((nOffset > g_posInfo.nSelectionStartOffset) && (nOffset < g_posInfo.nSelectionEndOffset)) {
        result = ST_MID;
    }

    return result;
}

qint64 QHexView::addressToOffset(qint64 nAddress)
{
    return XBinary::addressToOffset(&g_memoryMap, nAddress);
}

qint64 QHexView::relAddressToOffset(qint64 nRelAddress)
{
    return XBinary::relAddressToOffset(&g_memoryMap, nRelAddress);
}

qint64 QHexView::offsetToAddress(qint64 nOffset)
{
    //    qint64 nResult=-1;

    //    if(nOffset!=-1)
    //    {
    //        nResult=nOffset+_nBaseAddress;
    //    }

    //    return nResult;
    return XBinary::offsetToAddress(&g_memoryMap, nOffset);
}

QPoint QHexView::cursorToPoint(QHexView::CURSOR_POSITION cp)
{
    QPoint result;

    qint64 nRelOffset = cp.nOffset - g_nStartOffset;

    if (cp.type != CT_NONE) {
        result.setY((nRelOffset / g_nBytesProLine) * g_nLineHeight);
    }

    if (cp.type == CT_ANSI) {
        result.setX(g_nAnsiPosition + (nRelOffset % g_nBytesProLine) * g_nCharWidth);
    } else if (cp.type == CT_HIWORD) {
        result.setX(g_nHexPosition + (nRelOffset % g_nBytesProLine) * g_nCharWidth * 3);
    } else if (cp.type == CT_LOWORD) {
        result.setX(g_nHexPosition + (nRelOffset % g_nBytesProLine) * g_nCharWidth * 3 + g_nCharWidth);
    }

    return result;
}

bool QHexView::readByte(qint64 nOffset, quint8 *pByte)
{
    int nCount = 0;

    if (g_pDevice->seek(nOffset)) {
        nCount = (int)g_pDevice->read((char *)pByte, 1);
    }

    return (nCount == 1);
}

bool QHexView::writeByte(qint64 nOffset, quint8 *pByte)
{
    int nCount = 0;

    if (g_pDevice->seek(nOffset)) {
        nCount = (int)g_pDevice->write((char *)pByte, 1);
    }

    return (nCount == 1);
}

void QHexView::_customContextMenu(const QPoint &pos)
{
    // TODO
    emit customContextMenu(mapToGlobal(pos));
}

void QHexView::adjust()
{
    int nHeight = viewport()->height();
    g_nLineHeight = g_nCharHeight + 5;
    g_nLinesProPage = (nHeight) / g_nLineHeight;  // mb nHeight-4
    g_nDataBlockSize = g_nLinesProPage * g_nBytesProLine;

    g_nAddressPosition = g_nCharWidth;
    g_nAddressWidthCount = 8;

    if (g_pDevice) {
        if ((g_pDevice->size() + g_memoryMap.nModuleAddress) >= 0xFFFFFFFF) {
            g_nAddressWidthCount = 16;
        }
    }

    g_nAddressWidth = (g_nAddressWidthCount + 3) * g_nCharWidth;  // TODO set addresswidth
    g_nHexPosition = g_nAddressPosition + g_nAddressWidth;
    g_nHexWidth = (g_nBytesProLine + 1) * g_nCharWidth * 3;
    g_nAnsiPosition = g_nHexPosition + g_nHexWidth;
    g_nAnsiWidth = (g_nBytesProLine + 1) * g_nCharWidth;

    horizontalScrollBar()->setRange(0, g_nAnsiPosition + g_nAnsiWidth - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());

    verticalScrollBar()->setRange(0, g_nTotalLineCount - g_nLinesProPage);
    verticalScrollBar()->setPageStep(g_nLinesProPage);

    g_nStartOffset = verticalScrollBar()->value() * g_nBytesProLine + g_nStartOffsetDelta;
    g_nXOffset = horizontalScrollBar()->value();

    // TODO update
    if (g_pDevice) {
        if (g_pDevice->seek(g_nStartOffset)) {
            g_baDataBuffer.resize(g_nDataBlockSize);
            int nCount = (int)g_pDevice->read(g_baDataBuffer.data(), g_nDataBlockSize);
            g_baDataBuffer.resize(nCount);
            g_baDataHexBuffer = QByteArray(g_baDataBuffer.toHex());
        } else {
            g_baDataBuffer.clear();
            g_baDataHexBuffer.clear();
        }
    }

    qint64 nRelOffset = g_posInfo.cursorPosition.nOffset - g_nStartOffset;

    if (nRelOffset < 0) {
        nRelOffset = g_posInfo.cursorPosition.nOffset % g_nBytesProLine;
        g_posInfo.cursorPosition.nOffset = g_nStartOffset + nRelOffset;
    } else if (nRelOffset >= g_nBytesProLine * g_nLinesProPage) {
        nRelOffset = g_posInfo.cursorPosition.nOffset % g_nBytesProLine;
        g_posInfo.cursorPosition.nOffset = g_nStartOffset + g_nBytesProLine * (g_nLinesProPage - 1) + nRelOffset;
    }

    if (g_posInfo.cursorPosition.nOffset > g_nDataSize - 1) {
        g_posInfo.cursorPosition.nOffset = g_nDataSize - 1;
    }

    if ((g_posInfo.cursorPosition.nOffset != -1) && (g_posInfo.cursorPosition.type != CT_NONE)) {
        QPoint point = cursorToPoint(g_posInfo.cursorPosition);
        g_rectCursor.setRect(point.x() - horizontalScrollBar()->value(), point.y() + g_nLineDelta, g_nCharWidth, g_nLineHeight);
    }

    emit cursorPositionChanged();
}

void QHexView::init()
{
    g_nStartOffset = 0;
    g_nStartOffsetDelta = 0;
    g_nDataSize = 0;

    if (g_pDevice) {
        g_nDataSize = g_pDevice->size();
    }

    g_nTotalLineCount = g_nDataSize / g_nBytesProLine + 1;
    verticalScrollBar()->setValue(0);
}

QHexView::CURSOR_POSITION QHexView::getCursorPosition(QPoint pos)
{
    CURSOR_POSITION result = CURSOR_POSITION();
    result.nOffset = -1;

    result.type = CT_NONE;
    quint64 nRelOffset = -1;

    int nX = pos.x() + horizontalScrollBar()->value();
    int nY = pos.y();
    int nDeltaX = 0;
    int nDeltaY = 0;

    if ((nX > g_nAddressPosition) && (nX < g_nAddressPosition + g_nAddressWidth)) {
        nDeltaX = 0;
        nDeltaY = (nY - g_nLineDelta) / g_nLineHeight;

        result.type = CT_HIWORD;

        nRelOffset = nDeltaY * g_nBytesProLine + nDeltaX;
    } else if ((nX > g_nHexPosition) && (nX < g_nHexPosition + g_nHexWidth)) {
        nDeltaX = (nX - g_nHexPosition) / (g_nCharWidth * 3);
        nDeltaY = (nY - g_nLineDelta) / g_nLineHeight;

        if ((nX - g_nHexPosition) % (g_nCharWidth * 3) <= g_nCharWidth) {
            result.type = CT_HIWORD;
        } else {
            result.type = CT_LOWORD;
        }

        nRelOffset = nDeltaY * g_nBytesProLine + nDeltaX;
        // TODO !!!
    } else if ((nX > g_nAnsiPosition) && (nX < g_nAnsiPosition + g_nAnsiWidth)) {
        nDeltaX = (nX - g_nAnsiPosition) / g_nCharWidth;
        nDeltaY = (nY - g_nLineDelta) / g_nLineHeight;  // mb TODO LindeDelta

        result.type = CT_ANSI;
        nRelOffset = nDeltaY * g_nBytesProLine + nDeltaX;
    }

    if (nRelOffset != (quint64)-1) {
        result.nOffset = g_nStartOffset + nRelOffset;

        if (!isOffsetValid(result.nOffset)) {
            result.nOffset = -1;
        }
    }

    return result;
}

void QHexView::updateBlink()
{
    g_bBlink = (bool)(!g_bBlink);
    viewport()->update(g_rectCursor);
}

void QHexView::_initSelection(qint64 nOffset)
{
    if (!isOffsetValid(nOffset)) {
        nOffset = -1;
    }

    g_posInfo.nSelectionInitOffset = nOffset;
    g_posInfo.nSelectionStartOffset = -1;
    g_posInfo.nSelectionEndOffset = -1;
}

void QHexView::_setSelection(qint64 nOffset)
{
    if (isOffsetValid(nOffset)) {
        if (nOffset > g_posInfo.nSelectionInitOffset) {
            g_posInfo.nSelectionStartOffset = g_posInfo.nSelectionInitOffset;
            g_posInfo.nSelectionEndOffset = nOffset;
        } else {
            g_posInfo.nSelectionStartOffset = nOffset;
            g_posInfo.nSelectionEndOffset = g_posInfo.nSelectionInitOffset;
        }
    }
}

void QHexView::resizeEvent(QResizeEvent *pEvent)
{
    Q_UNUSED(pEvent)

    adjust();
}

void QHexView::keyPressEvent(QKeyEvent *pEvent)
{
    // Move commands
    if (pEvent->matches(QKeySequence::MoveToNextChar) || pEvent->matches(QKeySequence::MoveToPreviousChar) || pEvent->matches(QKeySequence::MoveToNextLine) ||
        pEvent->matches(QKeySequence::MoveToPreviousLine) || pEvent->matches(QKeySequence::MoveToStartOfLine) || pEvent->matches(QKeySequence::MoveToEndOfLine) ||
        pEvent->matches(QKeySequence::MoveToNextPage) || pEvent->matches(QKeySequence::MoveToPreviousPage) || pEvent->matches(QKeySequence::MoveToStartOfDocument) ||
        pEvent->matches(QKeySequence::MoveToEndOfDocument)) {
        if (pEvent->matches(QKeySequence::MoveToNextChar) || pEvent->matches(QKeySequence::MoveToPreviousChar)) {
            if (g_posInfo.cursorPosition.type == CT_ANSI) {
                if (pEvent->matches(QKeySequence::MoveToNextChar)) {
                    g_posInfo.cursorPosition.nOffset++;
                } else if (pEvent->matches(QKeySequence::MoveToPreviousChar)) {
                    g_posInfo.cursorPosition.nOffset--;
                }
            } else if (g_posInfo.cursorPosition.type == CT_HIWORD) {
                if (pEvent->matches(QKeySequence::MoveToNextChar)) {
                    g_posInfo.cursorPosition.type = CT_LOWORD;
                } else if (pEvent->matches(QKeySequence::MoveToPreviousChar)) {
                    g_posInfo.cursorPosition.nOffset--;
                    g_posInfo.cursorPosition.type = CT_LOWORD;
                }
            } else if (g_posInfo.cursorPosition.type == CT_LOWORD) {
                if (pEvent->matches(QKeySequence::MoveToNextChar)) {
                    g_posInfo.cursorPosition.nOffset++;
                    g_posInfo.cursorPosition.type = CT_HIWORD;
                } else if (pEvent->matches(QKeySequence::MoveToPreviousChar)) {
                    g_posInfo.cursorPosition.type = CT_HIWORD;
                }
            }
        } else if (pEvent->matches(QKeySequence::MoveToNextLine) || pEvent->matches(QKeySequence::MoveToPreviousLine)) {
            if (pEvent->matches(QKeySequence::MoveToNextLine)) {
                g_posInfo.cursorPosition.nOffset += g_nBytesProLine;
            } else if (pEvent->matches(QKeySequence::MoveToPreviousLine)) {
                g_posInfo.cursorPosition.nOffset -= g_nBytesProLine;
            }
        } else if (pEvent->matches(QKeySequence::MoveToNextPage) || pEvent->matches(QKeySequence::MoveToPreviousPage)) {
            if (pEvent->matches(QKeySequence::MoveToNextPage)) {
                g_posInfo.cursorPosition.nOffset += g_nBytesProLine * g_nLinesProPage;
            } else if (pEvent->matches(QKeySequence::MoveToPreviousPage)) {
                g_posInfo.cursorPosition.nOffset -= g_nBytesProLine * g_nLinesProPage;
            }
        } else if (pEvent->matches(QKeySequence::MoveToStartOfLine) || pEvent->matches(QKeySequence::MoveToEndOfLine) ||
                   pEvent->matches(QKeySequence::MoveToStartOfDocument) || pEvent->matches(QKeySequence::MoveToEndOfDocument)) {
            if ((g_posInfo.cursorPosition.type == CT_HIWORD) || (g_posInfo.cursorPosition.type == CT_LOWORD)) {
                g_posInfo.cursorPosition.type = CT_HIWORD;
            }

            if (pEvent->matches(QKeySequence::MoveToStartOfLine)) {
                g_posInfo.cursorPosition.nOffset = (g_posInfo.cursorPosition.nOffset / g_nBytesProLine) * g_nBytesProLine;
            } else if (pEvent->matches(QKeySequence::MoveToEndOfLine)) {
                g_posInfo.cursorPosition.nOffset = ((g_posInfo.cursorPosition.nOffset / g_nBytesProLine) + 1) * g_nBytesProLine - 1;
            } else if (pEvent->matches(QKeySequence::MoveToStartOfDocument)) {
                g_posInfo.cursorPosition.nOffset = 0;
            } else if (pEvent->matches(QKeySequence::MoveToEndOfDocument)) {
                g_posInfo.cursorPosition.nOffset = g_nDataSize - 1;
            }
        }

        if (g_posInfo.cursorPosition.type != CT_NONE) {
            if (pEvent->matches(QKeySequence::MoveToNextChar) || pEvent->matches(QKeySequence::MoveToPreviousChar) || pEvent->matches(QKeySequence::MoveToNextLine) ||
                pEvent->matches(QKeySequence::MoveToPreviousLine) || pEvent->matches(QKeySequence::MoveToStartOfLine) || pEvent->matches(QKeySequence::MoveToEndOfLine)) {
                if (g_posInfo.cursorPosition.nOffset < 0) {
                    g_posInfo.cursorPosition.nOffset = 0;
                    g_nStartOffsetDelta = 0;

                    if (g_posInfo.cursorPosition.type != CT_ANSI) {
                        g_posInfo.cursorPosition.type = CT_HIWORD;
                    }
                } else if (g_posInfo.cursorPosition.nOffset > g_nDataSize - 1) {
                    g_posInfo.cursorPosition.nOffset = g_nDataSize - 1;
                    g_nStartOffsetDelta = 0;

                    if (g_posInfo.cursorPosition.type != CT_ANSI) {
                        g_posInfo.cursorPosition.type = CT_LOWORD;
                    }
                }

                qint64 nRelOffset = g_posInfo.cursorPosition.nOffset - g_nStartOffset;

                if (nRelOffset >= g_nBytesProLine * g_nLinesProPage) {
                    _goToOffset(g_nStartOffset + g_nBytesProLine);
                } else if (nRelOffset < 0) {
                    _goToOffset(g_nStartOffset - g_nBytesProLine);
                }
            } else if (pEvent->matches(QKeySequence::MoveToNextPage) || pEvent->matches(QKeySequence::MoveToPreviousPage)) {
                if (g_posInfo.cursorPosition.nOffset < 0) {
                    g_posInfo.cursorPosition.nOffset += g_nBytesProLine * g_nLinesProPage;
                } else if (g_posInfo.cursorPosition.nOffset > g_nDataSize - 1) {
                    g_posInfo.cursorPosition.nOffset -= g_nBytesProLine * g_nLinesProPage;
                } else {
                    if (pEvent->matches(QKeySequence::MoveToNextPage)) {
                        _goToOffset(g_nStartOffset + g_nBytesProLine * g_nLinesProPage);
                    } else if (pEvent->matches(QKeySequence::MoveToPreviousPage)) {
                        _goToOffset(g_nStartOffset - g_nBytesProLine * g_nLinesProPage);
                    }
                }
            } else if (pEvent->matches(QKeySequence::MoveToStartOfDocument)) {
                _goToOffset(0);
            } else if (pEvent->matches(QKeySequence::MoveToEndOfDocument)) {
                qint64 nEndPageOffset = 0;
                nEndPageOffset = (g_nDataSize - (g_nDataSize) % g_nBytesProLine - g_nBytesProLine * (g_nLinesProPage - 1));

                if (nEndPageOffset < 0) {
                    nEndPageOffset = 0;
                }

                _goToOffset(nEndPageOffset);
            }

            adjust();
            viewport()->update();
        }
    } else if (pEvent->matches(QKeySequence::SelectAll))  // TODO select chars
    {
        _initSelection(0);
        _setSelection(g_nDataSize - 1);

        adjust();
        viewport()->update();
    } else {
        if (!g_bReadonly) {
            if ((!(pEvent->modifiers() & Qt::AltModifier)) && (!(pEvent->modifiers() & Qt::ControlModifier)) && (!(pEvent->modifiers() & Qt::MetaModifier))) {
                quint8 nByte = 0;
                quint8 nChar = 0;
                int nKey = pEvent->key();
                bool bSuccess = false;

                if (readByte(g_posInfo.cursorPosition.nOffset, &nByte)) {
                    if (g_posInfo.cursorPosition.type == CT_ANSI) {
                        if ((nKey >= Qt::Key_Space) && (nKey <= Qt::Key_AsciiTilde)) {
                            nChar = (quint8)((unsigned char *)(pEvent->text().toLatin1().data()))[0];
                            bSuccess = true;
                        }
                    } else if ((g_posInfo.cursorPosition.type == CT_HIWORD) || (g_posInfo.cursorPosition.type == CT_LOWORD)) {
                        if ((nKey >= Qt::Key_0) && (nKey <= Qt::Key_9)) {
                            nChar = (quint8)(nKey - Qt::Key_0);
                            bSuccess = true;
                        } else if ((nKey >= Qt::Key_A) && (nKey <= Qt::Key_F)) {
                            nChar = (quint8)(nKey - Qt::Key_A + 10);
                            bSuccess = true;
                        }
                    }

                    if (bSuccess) {
                        if (g_posInfo.cursorPosition.type == CT_HIWORD) {
                            nChar = (nByte & 0x0F) + (nChar << 4);
                        } else if (g_posInfo.cursorPosition.type == CT_LOWORD) {
                            nChar = (nByte & 0xF0) + nChar;
                        }

                        bool bSave = true;

                        if (!g_bIsEdited) {
                            // TODO Check
                            // Save backup
                            if (g_sBackupFileName != "") {
                                if (!QFile::exists(g_sBackupFileName)) {
                                    if (g_pDevice->metaObject()->className() == QString("QFile")) {
                                        QString sFileName = ((QFile *)g_pDevice)->fileName();

                                        if (!QFile::copy(sFileName, g_sBackupFileName)) {
                                            bSave = false;
                                            emit errorMessage(tr("Cannot save file") + QString(": %1").arg(g_sBackupFileName));
                                        }
                                    }
                                    // TODO if not file/ Create file/ Write data
                                }
                            }
                        }

                        if (bSave) {
                            if (writeByte(g_posInfo.cursorPosition.nOffset, &nChar)) {
                                g_bIsEdited = true;

                                emit editState(g_bIsEdited);

                                if (g_posInfo.cursorPosition.type == CT_ANSI) {
                                    g_posInfo.cursorPosition.nOffset++;
                                } else if (g_posInfo.cursorPosition.type == CT_HIWORD) {
                                    g_posInfo.cursorPosition.type = CT_LOWORD;
                                } else if (g_posInfo.cursorPosition.type == CT_LOWORD) {
                                    g_posInfo.cursorPosition.nOffset++;
                                    g_posInfo.cursorPosition.type = CT_HIWORD;
                                }

                                if (g_posInfo.cursorPosition.nOffset > g_nDataSize - 1) {
                                    g_posInfo.cursorPosition.nOffset = g_nDataSize - 1;

                                    if (g_posInfo.cursorPosition.type != CT_ANSI) {
                                        g_posInfo.cursorPosition.type = CT_LOWORD;
                                    }
                                }

                                adjust();
                                viewport()->update();
                            }
                        }
                    }
                }
            }
        }

        QAbstractScrollArea::keyPressEvent(pEvent);
    }
}

void QHexView::wheelEvent(QWheelEvent *pEvent)
{
    if ((g_nStartOffsetDelta) && (pEvent->angleDelta().y() > 0)) {
        if (verticalScrollBar()->value() == 0) {
            g_nStartOffsetDelta = 0;
            adjust();
            viewport()->update();
        }
    }

    QAbstractScrollArea::wheelEvent(pEvent);
}
