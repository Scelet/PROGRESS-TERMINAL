#include "mobiledialog.h"
#include "ui_mobiledialog.h"

MobileDialog::MobileDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MobileDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->pushButton, &QPushButton::clicked, this, &MobileDialog::showModemInfo);
    m_ui->voiceInputSlider->setRange(0,11);
    m_ui->voiceInputSlider->setSingleStep(1);
    m_ui->voiceOutputSlider->setRange(0,11);
    m_ui->voiceOutputSlider->setSingleStep(1);
    //QString vol1= getAnswer("AT+CMVLC?\r\n");
    //vol1 = vol1.sliced(vol1.indexOf(":")+2,vol1.size()-2-vol1.indexOf(":"));
    //m_ui->voiceInputSlider->setValue(vol1.sliced(vol1.indexOf(":")+2,vol1.size()-2-vol1.indexOf(":")).toInt());

}

MobileDialog::~MobileDialog()
{
    delete m_ui;
}

void MobileDialog::delay(int n)//time in msec
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void MobileDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "mobiledialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString MobileDialog::getAnswer(const QString &data)
{
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
    QString infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "mobiledialog get data0" << infoText;
    infoText.resize(infoText.size()-8);// cut "OK" from answer
    infoText=infoText.sliced(2);
    //qDebug() << "mobiledialog get data1" << infoText;
    return infoText;
}

void MobileDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

void MobileDialog::showModemInfo()
{
    m_ui->pushButton->setEnabled(false);
    m_ui->modemInfoLabel->clear();//erase label
    textData = "";//erase console output buffer
    QString infoText = getAnswer("ATI\r\n");
    qDebug() << "mobiledialog get data" << infoText;
    m_ui->modemInfoLabel->setText(infoText);    
    QString vol1= getAnswer("AT+CMVLC?\r\n");
    m_ui->voiceInputSlider->setValue(vol1.sliced(vol1.indexOf(":")+2,vol1.size()-2-vol1.indexOf(":")).toInt());
    vol1= getAnswer("AT+CLVLC?\r\n");
    m_ui->voiceOutputSlider->setValue(vol1.sliced(vol1.indexOf(":")+2,vol1.size()-2-vol1.indexOf(":")).toInt());
    m_ui->pushButton->setEnabled(true);
}

void MobileDialog::on_voiceInputSlider_valueChanged(int value)
{
    emit getData(("AT+CMVLC="+QString::number(value)+"\r\n").toLocal8Bit());//send data to port
}

void MobileDialog::on_voiceOutputSlider_valueChanged(int value)
{
    emit getData(("AT+CLVLC="+QString::number(value)+"\r\n").toLocal8Bit());//send data to port
}

void MobileDialog::on_dialPpushButton_clicked()
{
    emit getData(("ATD"+m_ui->dialNumberLineEdit->text()+";\r\n").toLocal8Bit());//send data to port
}

void MobileDialog::on_hangPushButton_clicked()
{
    emit getData("ATH\r\n");//send data to port
}

void MobileDialog::on_answerPushButton_clicked()
{
    emit getData("ATA\r\n");//send data to port
}

void MobileDialog::on_sendPushButton_clicked()
{
    emit getData("AT+CMGF=1\r\n");//send data to port
    vaitForOk();
    emit getData(("AT+CMGS="+m_ui->dialNumberLineEdit->text()+"\r\n").toLocal8Bit());//send data to port
    vaitForOk();
    emit getData((m_ui->smsPlainTextEdit->toPlainText()+"\x1A\r\n").toLocal8Bit());//send data to port
    vaitForOk();
}
