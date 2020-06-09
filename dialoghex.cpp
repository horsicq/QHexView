// copyright (c) 2017-2020 hors<horsicq@gmail.com>
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
#include "dialoghex.h"
#include "ui_dialoghex.h"

DialogHex::DialogHex(QWidget *parent,QIODevice *pDevice,QHexView::OPTIONS *pOptions) :
    QDialog(parent),
    ui(new Ui::DialogHex)
{
    ui->setupUi(this);

    connect(ui->widgetHex,SIGNAL(editState(bool)),this,SIGNAL(setEdited(bool)));

    ui->widgetHex->enableHeader(true);
    ui->widgetHex->enableReadOnly(true);

    ui->widgetHex->setData(pDevice,pOptions);
}

DialogHex::~DialogHex()
{
    delete ui;
}

void DialogHex::on_pushButtonClose_clicked()
{
    this->close();
}
