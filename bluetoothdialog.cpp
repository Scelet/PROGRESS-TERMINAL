#include "bluetoothdialog.h"
#include "ui_bluetoothdialog.h"

BluetoothDialog::BluetoothDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::BluetoothDialog)
{
    m_ui->setupUi(this);
}

BluetoothDialog::~BluetoothDialog()
{
    delete m_ui;
}

void BluetoothDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void BluetoothDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "bluetoothdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString BluetoothDialog::getAnswer(const QString &data)
{
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
    QString infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "bluetoothdialog get data0" << infoText;
    infoText.resize(infoText.size()-8);// cut "OK" from answer
    infoText=infoText.sliced(2);
    //qDebug() << "bluetoothdialog get data1" << infoText;
    return infoText;
}

void BluetoothDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

void BluetoothDialog::on_bltPowerPushButton_clicked()
{
    this->setEnabled(false);
    textData = "";//erase console output buffer
    if(m_ui->bltPowerPushButton->text() == tr("Turn On"))
    {
        emit getData("AT+BTTEST=bt,open\r\n");//send data to port
        vaitForOk();
        for(int i=0; i<20; i++){
            delay(500);
            if(textData.indexOf("+BT:")>0) break;
        }//wait for response
        m_ui->bltPowerPushButton->setText(tr("Turn Off"));
    }
    else if (m_ui->bltPowerPushButton->text() == tr("Turn Off"))
    {
        emit getData("AT+BTTEST=bt,close\r\n");//send data to port
        vaitForOk();
        for(int i=0; i<20; i++){
            delay(500);
            if(textData.indexOf("+BT:")>0) break;
        }//wait for response
        m_ui->bltPowerPushButton->setText(tr("Turn On"));
    }
    this->setEnabled(true);
}


void BluetoothDialog::on_visPushButton_clicked()
{
    m_ui->visPushButton->setEnabled(false);
    textData = "";//erase console output buffer
    if(m_ui->visPushButton->text() == tr("Set visible"))
    {
        emit getData("AT+BTTEST=bt,visiable,1\r\n");//send data to port
        vaitForOk();
        m_ui->visPushButton->setText(tr("Set invisible"));
    }
    else if (m_ui->visPushButton->text() == tr("Set invisible"))
    {
        emit getData("AT+BTTEST=bt,visiable,0\r\n");//send data to port
        vaitForOk();
        m_ui->visPushButton->setText(tr("Set visible"));
    }
    m_ui->visPushButton->setEnabled(true);
}

void BluetoothDialog::on_searchPushButton_clicked()
{
    m_ui->searchPushButton->setEnabled(false);
    textData = "";//erase console output buffer
    if(m_ui->searchPushButton->text() == tr("Search"))
    {
        emit getData("AT+BTTEST=bt,inquiry\r\n");//send data to port
        vaitForOk();
        m_ui->searchPushButton->setText(tr("Stop search"));
    }
    else if (m_ui->searchPushButton->text() == tr("Stop search"))
    {
        emit getData("AT+BTTEST=bt,inquirycancel\r\n");//send data to port
        vaitForOk();
        m_ui->searchPushButton->setText(tr("Search"));
    }
    m_ui->searchPushButton->setEnabled(true);
}

