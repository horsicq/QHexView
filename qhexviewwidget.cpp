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
#include "qhexviewwidget.h"
#include "ui_qhexviewwidget.h"

QHexViewWidget::QHexViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QHexViewWidget)
{
    ui->setupUi(this);

    ui->lineEditCursorAddress->setReadOnly(true);
    ui->lineEditSelectionAddress->setReadOnly(true);
    ui->lineEditSelectionSize->setReadOnly(true);

    connect(ui->scrollAreaHex,SIGNAL(cursorPositionChanged()),this,SLOT(_getState()));
    connect(ui->scrollAreaHex,SIGNAL(errorMessage(QString)),this,SLOT(_errorMessage(QString)));
    connect(ui->scrollAreaHex,SIGNAL(customContextMenu(const QPoint &)),this,SLOT(_customContextMenu(const QPoint &)));

    new QShortcut(QKeySequence(KS_GOTOADDRESS),this,SLOT(_goToAddress()));
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
    DialogGoToAddress da(this,ui->scrollAreaHex);
    da.exec();

    ui->scrollAreaHex->setFocus();
    ui->scrollAreaHex->reload();
}

void QHexViewWidget::_dumpToFile()
{
    QString sFilter;
    sFilter+=QString("%1 (*.bin)").arg(tr("Raw data"));
    QString sSaveFileName="Result"; // TODO default directory
    QString sFileName=QFileDialog::getSaveFileName(this,tr("Save dump"),sSaveFileName,sFilter);

    if(!sFileName.isEmpty())
    {
        QHexView::STATE state=ui->scrollAreaHex->getState();

        DialogDump dd(this);

        dd.dumpToFile(ui->scrollAreaHex->getDevice(),state.nSelectionOffset,state.nSelectionSize,sFileName);

        dd.exec();
    }
}

void QHexViewWidget::_selectAll()
{
    ui->scrollAreaHex->selectAll();
}

void QHexViewWidget::_customContextMenu(const QPoint &pos)
{

    QHexView::STATE state=ui->scrollAreaHex->getState();

    QMenu contextMenu(this);

    QAction actionGoToAddress(tr("Go to Address"),this);
    actionGoToAddress.setShortcut(QKeySequence(KS_GOTOADDRESS));
    connect(&actionGoToAddress,SIGNAL(triggered()),this,SLOT(_goToAddress()));
    contextMenu.addAction(&actionGoToAddress);

    if(state.nSelectionSize)
    {
        QAction actionDumpToFile(tr("Dump to File"),this);
        actionDumpToFile.setShortcut(QKeySequence(KS_DUMPTOFILE));
        connect(&actionDumpToFile,SIGNAL(triggered()),this,SLOT(_dumpToFile()));
        contextMenu.addAction(&actionDumpToFile);
    }

    QMenu menuSelect(tr("Select"),this);

    QAction actionSelectAll(tr("Select All"),this);
    connect(&actionSelectAll,SIGNAL(triggered()),this,SLOT(_selectAll()));

    menuSelect.addAction(&actionSelectAll);
    contextMenu.addMenu(&menuSelect);

    // TODO Search
    // TODO Select (All/reset)

    contextMenu.exec(pos);
}

void QHexViewWidget::_errorMessage(QString sText)
{
    QMessageBox::critical(this,tr("Error"),sText);
}
