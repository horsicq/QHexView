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
#include "qhexviewwidget.h"
#include "ui_qhexviewwidget.h"

QHexViewWidget::QHexViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QHexViewWidget)
{
    ui->setupUi(this);

    ui->scrollAreaHex->installEventFilter(this);

    ui->lineEditCursorAddress->setReadOnly(true);
    ui->lineEditSelectionAddress->setReadOnly(true);
    ui->lineEditSelectionSize->setReadOnly(true);

    connect(ui->scrollAreaHex,SIGNAL(cursorPositionChanged()),this,SLOT(_getState()));
    connect(ui->scrollAreaHex,SIGNAL(errorMessage(QString)),this,SLOT(_errorMessage(QString)));
    connect(ui->scrollAreaHex,SIGNAL(customContextMenu(const QPoint &)),this,SLOT(_customContextMenu(const QPoint &)));
    connect(ui->scrollAreaHex,SIGNAL(editState(bool)),this,SIGNAL(editState(bool)));

    scGoToAddress=nullptr;
    scDumpToFile=nullptr;
    scSelectAll=nullptr;
    scCopyAsHex=nullptr;
    scFind=nullptr;
    scFindNext=nullptr;

    ui->scrollAreaHex->setFocus();

    searchData={};
    searchData.nResult=-1;
}

QHexViewWidget::~QHexViewWidget()
{
    delete ui;
}

void QHexViewWidget::setData(QIODevice *pDevice,QHexView::OPTIONS *pOptions)
{
    ui->scrollAreaHex->setData(pDevice,pOptions);
    ui->checkBoxReadonly->setChecked(true);
    ui->checkBoxReadonly->setEnabled(pDevice->isWritable());
}

void QHexViewWidget::setBackupFileName(QString sBackupFileName)
{
    ui->scrollAreaHex->setBackupFileName(sBackupFileName);
}

void QHexViewWidget::setSaveDirectory(QString sSaveDirectory)
{
    this->sSaveDirectory=sSaveDirectory;
}

void QHexViewWidget::enableHeader(bool bState)
{
    if(bState)
    {
        ui->widgetHeader->show();
    }
    else
    {
        ui->widgetHeader->hide();
    }
}

void QHexViewWidget::enableReadOnly(bool bState)
{
    if(bState)
    {
        ui->checkBoxReadonly->show();
    }
    else
    {
        ui->checkBoxReadonly->hide();
    }
}

bool QHexViewWidget::setReadonly(bool bState)
{
    return ui->scrollAreaHex->setReadonly(bState);
}

void QHexViewWidget::reload()
{
    ui->scrollAreaHex->reload();
}

bool QHexViewWidget::isEdited()
{
    return ui->scrollAreaHex->isEdited();
}

void QHexViewWidget::setEdited(bool bState)
{
    ui->scrollAreaHex->setEdited(bState);
}

qint64 QHexViewWidget::getBaseAddress()
{
    return ui->scrollAreaHex->getBaseAddress();
}

void QHexViewWidget::setSelection(qint64 nAddress, qint64 nSize)
{
    ui->scrollAreaHex->setSelection(nAddress,nSize);
}

void QHexViewWidget::goToAddress(qint64 nAddress)
{
    ui->scrollAreaHex->goToAddress(nAddress);
    ui->scrollAreaHex->reload();
}

void QHexViewWidget::goToOffset(qint64 nOffset)
{
    ui->scrollAreaHex->goToOffset(nOffset);
    ui->scrollAreaHex->reload();
}

bool QHexViewWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if(event->type()==QEvent::FocusIn)
    {
        scGoToAddress   =new QShortcut(QKeySequence(XShortcuts::GOTOADDRESS),   this,SLOT(_goToAddress()));
        scDumpToFile    =new QShortcut(QKeySequence(XShortcuts::DUMPTOFILE),    this,SLOT(_dumpToFile()));
        scSelectAll     =new QShortcut(QKeySequence(XShortcuts::SELECTALL),     this,SLOT(_selectAll()));
        scCopyAsHex     =new QShortcut(QKeySequence(XShortcuts::COPYASHEX),     this,SLOT(_copyAsHex()));
        scFind          =new QShortcut(QKeySequence(XShortcuts::FIND),          this,SLOT(_find()));
        scFindNext      =new QShortcut(QKeySequence(XShortcuts::FINDNEXT),      this,SLOT(_findNext()));
        scSignature     =new QShortcut(QKeySequence(XShortcuts::SIGNATURE),     this,SLOT(_signature()));
    }
    else if(event->type()==QEvent::FocusOut)
    {
        if(scGoToAddress)   delete scGoToAddress;
        if(scDumpToFile)    delete scDumpToFile;
        if(scSelectAll)     delete scSelectAll;
        if(scCopyAsHex)     delete scCopyAsHex;
        if(scFind)          delete scFind;
        if(scFindNext)      delete scFindNext;
        if(scSignature)     delete scSignature;
    }

    return false;
}

void QHexViewWidget::_getState()
{
    QHexView::STATE state=ui->scrollAreaHex->getState();

    ui->lineEditCursorAddress->setValue32_64((quint64)state.nCursorAddress);
    ui->lineEditSelectionAddress->setValue32_64((quint64)state.nSelectionAddress);
    ui->lineEditSelectionSize->setValue32_64((quint64)state.nSelectionSize);
}

void QHexViewWidget::on_pushButtonGoTo_clicked()
{
    _goToAddress();
}

void QHexViewWidget::on_checkBoxReadonly_toggled(bool checked)
{
    ui->scrollAreaHex->setReadonly(checked);
}

void QHexViewWidget::_goToAddress()
{
    DialogGoToAddress da(this,ui->scrollAreaHex->getMemoryMap(),DialogGoToAddress::TYPE_ADDRESS);
    if(da.exec()==QDialog::Accepted)
    {
        ui->scrollAreaHex->goToAddress(da.getValue());
        ui->scrollAreaHex->setFocus();
        ui->scrollAreaHex->reload();
    }
}

void QHexViewWidget::_dumpToFile()
{
    QString sFilter;
    sFilter+=QString("%1 (*.bin)").arg(tr("Raw data"));
    QString sSaveFileName=getDumpName();
    QString sFileName=QFileDialog::getSaveFileName(this,tr("Save dump"),sSaveFileName,sFilter);

    if(!sFileName.isEmpty())
    {
        QHexView::STATE state=ui->scrollAreaHex->getState();

        DialogDumpProcess dd(this,ui->scrollAreaHex->getDevice(),state.nSelectionOffset,state.nSelectionSize,sFileName,DumpProcess::DT_OFFSET);

        dd.exec();
    }
}

void QHexViewWidget::_find()
{
    QHexView::STATE state=ui->scrollAreaHex->getState();

    searchData={};
    searchData.nResult=-1;
    searchData.nCurrentOffset=state.nCursorOffset;

    DialogSearch dialogSearch(this,ui->scrollAreaHex->getDevice(),&searchData);

    if(dialogSearch.exec()==QDialog::Accepted)
    {
        ui->scrollAreaHex->goToOffset(searchData.nResult);
        ui->scrollAreaHex->setFocus();
        ui->scrollAreaHex->reload();
    }
}

void QHexViewWidget::_findNext()
{
    if(searchData.bInit)
    {
        searchData.nCurrentOffset=searchData.nResult+1;
        searchData.startFrom=SearchProcess::SF_CURRENTOFFSET;

        DialogSearchProcess dialogSearch(this,ui->scrollAreaHex->getDevice(),&searchData);

        if(dialogSearch.exec()==QDialog::Accepted)
        {
            ui->scrollAreaHex->goToOffset(searchData.nResult);
            ui->scrollAreaHex->setFocus();
            ui->scrollAreaHex->reload();
        }
    }
}

void QHexViewWidget::_selectAll()
{
    ui->scrollAreaHex->selectAll();
    _getState();
}

void QHexViewWidget::_copyAsHex()
{
    QHexView::STATE state=ui->scrollAreaHex->getState();

    qint64 nSize=qMin(state.nSelectionSize,(qint64)0x10000);

    QByteArray baData=ui->scrollAreaHex->readArray(state.nSelectionOffset,nSize);

    QApplication::clipboard()->setText(baData.toHex());
}

void QHexViewWidget::_signature()
{
    QHexView::STATE state=ui->scrollAreaHex->getState();

    DialogHexSignature dsh(this,ui->scrollAreaHex->getDevice(),state.nSelectionOffset,state.nSelectionSize);

    dsh.exec();
}

void QHexViewWidget::_customContextMenu(const QPoint &pos)
{
    QHexView::STATE state=ui->scrollAreaHex->getState();

    QMenu contextMenu(this);

    QAction actionGoToAddress(tr("Go to address"),this);
    actionGoToAddress.setShortcut(QKeySequence(XShortcuts::GOTOADDRESS));
    connect(&actionGoToAddress,SIGNAL(triggered()),this,SLOT(_goToAddress()));
    contextMenu.addAction(&actionGoToAddress);

    QAction actionDumpToFile(tr("Dump to file"),this);
    actionDumpToFile.setShortcut(QKeySequence(XShortcuts::DUMPTOFILE));
    connect(&actionDumpToFile,SIGNAL(triggered()),this,SLOT(_dumpToFile()));
    contextMenu.addAction(&actionDumpToFile);

    QAction actionSignature(tr("Signature"),this);
    actionSignature.setShortcut(QKeySequence(XShortcuts::SIGNATURE));
    connect(&actionSignature,SIGNAL(triggered()),this,SLOT(_signature()));
    contextMenu.addAction(&actionSignature);

    if(state.nSelectionSize)
    {
        contextMenu.addAction(&actionDumpToFile);
        contextMenu.addAction(&actionSignature);
    }

    QAction actionFind(tr("Find"),this);
    actionFind.setShortcut(QKeySequence(XShortcuts::FIND));
    connect(&actionFind,SIGNAL(triggered()),this,SLOT(_find()));
    contextMenu.addAction(&actionFind);

    QAction actionFindNext(tr("Find next"),this);
    actionFindNext.setShortcut(QKeySequence(XShortcuts::FINDNEXT));
    connect(&actionFindNext,SIGNAL(triggered()),this,SLOT(_findNext()));
    contextMenu.addAction(&actionFindNext);

    QMenu menuSelect(tr("Select"),this);

    QAction actionSelectAll(tr("Select all"),this);
    actionSelectAll.setShortcut(QKeySequence(XShortcuts::SELECTALL));
    connect(&actionSelectAll,SIGNAL(triggered()),this,SLOT(_selectAll()));

    menuSelect.addAction(&actionSelectAll);
    contextMenu.addMenu(&menuSelect);

    QMenu menuCopy(tr("Copy"),this);

    QAction actionCopyAsHex(tr("Copy as hex"),this);
    actionCopyAsHex.setShortcut(QKeySequence(XShortcuts::COPYASHEX));
    connect(&actionCopyAsHex,SIGNAL(triggered()),this,SLOT(_copyAsHex()));

    menuCopy.addAction(&actionCopyAsHex);
    contextMenu.addMenu(&menuCopy);

    // TODO reset select

    contextMenu.exec(pos);
}

void QHexViewWidget::_errorMessage(QString sText)
{
    QMessageBox::critical(this,tr("Error"),sText);
}

QString QHexViewWidget::getDumpName()
{
    QString sResult;

    if(sSaveDirectory!="")
    {
        sResult+=sSaveDirectory;
        sResult+=QDir::separator();
    }

    sResult+="dump.bin";

    return sResult;
}
