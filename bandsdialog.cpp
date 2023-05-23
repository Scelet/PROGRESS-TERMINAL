#include "bandsdialog.h"
#include "ui_bandsdialog.h"

BandsDialog::BandsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::BandsDialog)
{
    m_ui->setupUi(this);
    m_ui->BandsActivePushButton->setEnabled(false);
}

BandsDialog::~BandsDialog()
{
    delete m_ui;
}


void BandsDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void BandsDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "bandsdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString BandsDialog::getAnswer(const QString &data)
{
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(600);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
    QString infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "bandsdialog get data0" << infoText;
    infoText.resize(infoText.size()-8);// cut "OK" from answer
    infoText=infoText.sliced(2);
    //qDebug() << "bandsdialog get data1" << infoText;
    return infoText;
}

void BandsDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

QString BandsDialog::hexToName(const QString &group, const QString &data)
{
    if(group=="GW" && data == "80") return "1800";
    else if(group=="GW" && data == "300") return "900";
    else if(group=="LTE" && data == "1") return "1";
    else if(group=="LTE" && data == "4") return "3";
    else if(group=="LTE" && data == "10") return "5";
    else if(group=="LTE" && data == "40") return "7";
    else if(group=="LTE" && data == "80") return "8";
    else if(group=="LTE" && data == "80000") return "20";
    else if(group=="LTE" && data == "8000000") return "28";
    else if(group=="LTE" && data == "2000000000") return "38";
    else if(group=="LTE" && data == "8000000000") return "40";
    else if(group=="LTE" && data == "10000000000") return "41";
    else if(data.isEmpty()) return "";
    else
    {
        qDebug()<< "Uncnown band hex->name transform. Group:" <<group<< " Band:"<<data;
        return "";
    }
}

QString BandsDialog::nameToHex(const QString &group, const QString &data)
{
    if(group=="GW" && data == "1800") return "80";
    else if(group=="GW" && data == "900") return "300";
    else if(group=="LTE" && data == "1") return "1";
    else if(group=="LTE" && data == "3") return "4";
    else if(group=="LTE" && data == "5") return "10";
    else if(group=="LTE" && data == "7") return "40";
    else if(group=="LTE" && data == "8") return "80";
    else if(group=="LTE" && data == "20") return "80000";
    else if(group=="LTE" && data == "28") return "8000000";
    else if(group=="LTE" && data == "38") return "2000000000";
    else if(group=="LTE" && data == "40") return "8000000000";
    else if(group=="LTE" && data == "41") return "10000000000";
    else if(data.isEmpty()) return "";
    else
    {
        qDebug()<< "Uncnown band name->hex transform. Group:" <<group<< " Band:"<<data;
        return "";
    }
}

void BandsDialog::on_bandRefreshPushButton_clicked()
{
    this->setEnabled(false);
    QList<QStringList> allB = getAllBands();
    //create columns
    QStringList header;
    QStringList groups = allB[0];
    QStringList bands = allB[1];
    QTableWidgetItem* cell=0;

    header<<tr("Group")<<tr("Bands")<<tr("State");

    m_ui->bandsTableWidget->setColumnCount(3);
    m_ui->bandsTableWidget->setRowCount(groups.size());
    m_ui->bandsTableWidget->setHorizontalHeaderLabels(header);

    for(int j=0; j<groups.size();j++)
    {
        cell = new QTableWidgetItem(groups[j]);
        m_ui->bandsTableWidget->setItem(j,0,cell);
        cell = new QTableWidgetItem(hexToName(groups[j],bands[j]));
        m_ui->bandsTableWidget->setItem(j,1,cell);
    }
    //qDebug() << "allB" << allB;
    QList<QStringList> allC = getCurBands();
    //qDebug() << "allB" << allB;
    QStringList curGroups = allC[0];
    QStringList curBands = allC[1];
//    qDebug() << "groups" << groups;
//    qDebug() << "curGroups" << curGroups;
//    qDebug() << "bands" << bands;
//    qDebug() << "curBands" << curBands;

    for(int i=0; i<curGroups.size();i++)
    {
        for(int j=0; j<groups.size();j++)
        {
            if(groups[j] == curGroups[i] && hexToName(groups[j],bands[j]) == curBands[i] && !bands[j].isEmpty() && !curBands[i].isEmpty())
            {
                cell = new QTableWidgetItem("active");
                m_ui->bandsTableWidget->setItem(j,2,cell);
            }
        }
    }
    this->setEnabled(true);
}

QList<QStringList> BandsDialog::getAllBands()
{
    QString infoText;
    infoText= getAnswer("AT+ZBAND=?\r\n");
    infoText = infoText.sliced(infoText.indexOf("ZBAND")+8);
    QStringList bandsList = infoText.split("\r\n");    
    QStringList bandGr;
    QStringList groups;
    QStringList bands;
    QStringList band;
    int quantBands=0;
    //qDebug() << "bandsList" << bandsList;
    for(int i=0; i<bandsList.size();i++)
    {
        QStringList testB = bandsList[i].split(":");
        bandGr.append(testB[0]);
        band.append(testB[1].sliced(1,testB[1].size()-2));

        if(testB[1].indexOf(",")>0)
        {
            QStringList testB2 = testB[1].split(",");            
            if(testB2.size()>quantBands) quantBands += testB2.size();
        } else quantBands++;
    }
    //qDebug() << "bandGr" << bandGr;
    //qDebug() << "band" << band;
    for(int i=0; i<bandGr.size();i++)
    {
        if(!band[i].isEmpty()) {
            QStringList testC = band[i].split(",");

            for(int j=0; j<testC.size();j++)
            {
                testC[j] = testC[j].simplified();
                groups.append(bandGr[i]);
                bands.append(testC[j]);
            }
        }
        else
        {
            groups.append(bandGr[i]);
            bands.append("");
        }
    }
    //qDebug() << "groups" << groups;
    //qDebug() << "bands" << bands;
    QList<QStringList> result;
    result.append(groups);
    result.append(bands);    
    return result;
}


QList<QStringList> BandsDialog::getCurBands()
{
    QString infoText;
    infoText= getAnswer("AT+ZBAND?\r\n");
    infoText = infoText.sliced(infoText.indexOf("ZBAND")+9);
    QStringList bandsList = infoText.split("\r\n");
    QStringList bandGr;
    QStringList groups;
    QStringList bands;
    QStringList band;
    int quantBands=0;
    //qDebug() << "bandsList" << bandsList;
    for(int i=0; i<bandsList.size();i++)
    {
        QStringList testB = bandsList[i].split(":");
        bandGr.append(testB[0]);
        band.append(testB[1]);
        if(testB[1].indexOf(",")>0)
        {
            QStringList testB2 = testB[1].split(",");
            if(testB2.size()>quantBands) quantBands += testB2.size();
        } else quantBands++;
    }
    //qDebug() << "bandGr" << bandGr;
    //qDebug() << "band" << band;
    for(int i=0; i<bandGr.size();i++)
    {
        if(!band[i].isEmpty()) {
            QStringList testC = band[i].split(",");

            for(int j=0; j<testC.size();j++)
            {
                testC[j] = testC[j].simplified();
                groups.append(bandGr[i]);
                bands.append(testC[j]);
            }
        }
        else
        {
            groups.append(bandGr[i]);
            bands.append("");
        }
    }
    //qDebug() << "groups" << groups;
    //qDebug() << "bands" << bands;
    QList<QStringList> result;
    result.append(groups);
    result.append(bands);
    return result;
}

void BandsDialog::on_BandsActivePushButton_clicked()
{
    this->setEnabled(false);
    QTableWidgetItem* cell=0;
    QList<QStringList> allC = getCurBands();
    QStringList curGroups = allC[0];
    QStringList curBands = allC[1];
    QStringList hexBands;
    hexBands.append("0");//GW
    hexBands.append("0");//CDMA
    hexBands.append("0");//TDS
    hexBands.append("0");//LTE
    if(m_ui->bandsTableWidget->item(selectedRow,2)==0)
    {
        QList<QStringList> allB = getAllBands();
        QStringList groups = allB[0];
        QStringList bands = allB[1];
        for(int i=0; i<curGroups.size();i++)
        {
            if(curBands[i] == "")continue;
            curBands[i] = nameToHex(curGroups[i],curBands[i]);
            if(curGroups[i]=="GW") hexBands[0] = QString::number(hexBands[0].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="CDMA") hexBands[1] = QString::number(hexBands[1].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="TDS") hexBands[2] = QString::number(hexBands[2].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="LTE") hexBands[3] = QString::number(hexBands[3].toInt() + curBands[i].toInt());
        }
        if(groups[selectedRow]=="GW") hexBands[0] = QString::number(hexBands[0].toInt() + bands[selectedRow].toInt());
        else if(groups[selectedRow]=="CDMA") hexBands[1] = QString::number(hexBands[1].toInt() + bands[selectedRow].toInt());
        else if(groups[selectedRow]=="TDS") hexBands[2] = QString::number(hexBands[2].toInt() + bands[selectedRow].toInt());
        else if(groups[selectedRow]=="LTE") hexBands[3] = QString::number(hexBands[3].toInt() + bands[selectedRow].toInt());
        qDebug()<<"hexBands"<<hexBands;
        getAnswer("AT+ZBAND="+hexBands[0]+","+hexBands[1]+","+hexBands[2]+","+hexBands[3]+"\r\n");
        cell = new QTableWidgetItem("active");
        m_ui->bandsTableWidget->setItem(selectedRow,2,cell);
    }
    else
    {
        for(int i=0; i<curGroups.size();i++)
        {
            if(curBands[i]==m_ui->bandsTableWidget->item(selectedRow,1)->text() || curBands[i] == "")continue;
            curBands[i] = nameToHex(curGroups[i],curBands[i]);
            if(curGroups[i]=="GW") hexBands[0] = QString::number(hexBands[0].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="CDMA") hexBands[1] = QString::number(hexBands[1].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="TDS") hexBands[2] = QString::number(hexBands[2].toInt() + curBands[i].toInt());
            else if(curGroups[i]=="LTE") hexBands[3] = QString::number(hexBands[3].toInt() + curBands[i].toInt());
        }
        qDebug()<<"hexBands"<<hexBands;
        getAnswer("AT+ZBAND="+hexBands[0]+","+hexBands[1]+","+hexBands[2]+","+hexBands[3]+"\r\n");
        m_ui->bandsTableWidget->setItem(selectedRow,2,nullptr);
    }
    this->setEnabled(true);
}


void BandsDialog::on_bandsTableWidget_cellClicked(int row, int column)
{
    if(m_ui->bandsTableWidget->item(row,1)->text() == "")m_ui->BandsActivePushButton->setEnabled(false);
    else
    {
        if(m_ui->bandsTableWidget->item(row,2)==0) m_ui->BandsActivePushButton->setText(tr("Activate"));
        else m_ui->BandsActivePushButton->setText(tr("Deactivate"));
        m_ui->BandsActivePushButton->setEnabled(true);
    }

    selectedRow=row;
}

