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
#include "dialoggotoaddress.h"
#include "ui_dialoggotoaddress.h"

DialogGoToAddress::DialogGoToAddress(QWidget *parent, QList<XBinary::MEMORY_MAP> *pListMM) :
    QDialog(parent),
    ui(new Ui::DialogGoToAddress)
{
    ui->setupUi(this);
    this->pListMM=pListMM;
    nAddress=0;
}

DialogGoToAddress::~DialogGoToAddress()
{
    delete ui;
}

qint64 DialogGoToAddress::getAddress()
{
    return nAddress;
}

void DialogGoToAddress::on_pushButtonCancel_clicked()
{
    reject();
}

void DialogGoToAddress::on_pushButtonOK_clicked()
{
    qint64 nAddress=(qint64)ui->lineEditAddress->getValue();

    if(XBinary::isAddressValid(pListMM,nAddress))
    {
        this->nAddress=nAddress;
        accept();
    }
    else
    {
        ui->labelStatus->setText(tr("Invalid"));
    }
}
