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
#ifndef QHEXVIEWWIDGET_H
#define QHEXVIEWWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QMenu>
#include <QShortcut>
#include <QFileDialog>
#include "qhexview.h"
#include "dialoggotoaddress.h"
#include "dialogsearch.h"
#include "dialogdumpprocess.h"
#include "dialogsearchprocess.h"
#include "xshortcuts.h"
#include "dialoghexsignature.h"

namespace Ui
{
class QHexViewWidget;
}

class QHexViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QHexViewWidget(QWidget *pParent=nullptr);
    ~QHexViewWidget();
    void setData(QIODevice *pDevice,QHexView::OPTIONS *pOptions=nullptr);
    void setBackupFileName(QString sBackupFileName);
    void setSaveDirectory(QString sSaveDirectory);
    void enableHeader(bool bState);
    void enableReadOnly(bool bState);
    bool setReadonly(bool bState);
    void reload();
    bool isEdited();
    void setEdited(bool bState);
    qint64 getBaseAddress();
    void setSelection(qint64 nAddress,qint64 nSize);
    void goToAddress(qint64 nAddress);
    void goToOffset(qint64 nOffset);

protected:
    bool eventFilter(QObject *pObj,QEvent *pEvent) override;

signals:
    void editState(bool bState);

private slots:
    void on_pushButtonGoTo_clicked();
    void on_checkBoxReadonly_toggled(bool bChecked);
    void _getState();
    void _goToAddress();
    void _dumpToFile();
    void _find();
    void _findNext();
    void _selectAll();
    void _copyAsHex();
    void _signature();
    void _customContextMenu(const QPoint &pos);
    void _errorMessage(QString sText);
    QString getDumpName();
    void registerShortcuts(bool bState);

private:
    Ui::QHexViewWidget *ui;
    SearchProcess::SEARCHDATA searchData;
    QShortcut *scGoToAddress;
    QShortcut *scDumpToFile;
    QShortcut *scSelectAll;
    QShortcut *scCopyAsHex;
    QShortcut *scFind;
    QShortcut *scFindNext;
    QShortcut *scSignature;

    QString sSaveDirectory;
};

#endif // QHEXVIEWWIDGET_H
