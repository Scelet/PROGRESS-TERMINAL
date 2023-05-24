#include "networkdialog.h"
#include "ui_networkdialog.h"
#include "editpdpdialog.h"

NetworkDialog::NetworkDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NetworkDialog),
    m_edit(new EditPdpDialog)
{
    m_ui->setupUi(this);
    m_ui->selectOpPushButton->setEnabled(false);
    m_ui->editPushButton->setEnabled(false);
    m_ui->adPushButton->setEnabled(false);

    connect(m_edit, &EditPdpDialog::finished, [this](){this->setEnabled(true);});

    connect(m_edit, &EditPdpDialog::sendData, this, &NetworkDialog::on_edit);
    connect(this, &NetworkDialog::toEdit, m_edit, &EditPdpDialog::on_start);
}

NetworkDialog::~NetworkDialog()
{
    delete m_ui;
}

void NetworkDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void NetworkDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "networkdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString NetworkDialog::getAnswer(const QString &data)
{
    QString infoText;
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(600);
        if(textData.indexOf("OK")>=0)
        {
            infoText = QString::fromStdString(textData.toStdString());
            //qDebug() << "dataconnectdialog get data0" << infoText;
            infoText.resize(infoText.size()-6);// cut "OK" from answer
            if(infoText.size()>2)infoText=infoText.sliced(2);
            //qDebug() << "dataconnectdialog get data1" << infoText;
            return infoText;
            break;
        }
        if(textData.indexOf("ERROR")>=0)
        {
            infoText = QString::fromStdString(textData.toStdString());
            return infoText;
            break;
        }
    }//wait for response
    //return infoText = QString::fromStdString(textData.toStdString());
}

void NetworkDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

void NetworkDialog::on_refreshInfoPushButton_clicked()
{
    m_ui->refreshInfoPushButton->setEnabled(false);

    QString infoText = getAnswer("AT+COPS?\r\n");
    //qDebug() << "networkdialog get COPS" << infoText;
    if(infoText.indexOf("+COPS:")>=0)
    {
        QString cops = infoText.sliced(infoText.indexOf(":")+2);
        QStringList listCops;
        QString oper;
        QString format;

        qDebug() << "networkdialog get cops" << cops;
        if(cops.indexOf(",")>=0)
        {
            listCops = cops.split(",");
            cops = listCops[0];
            if(listCops[2].size()>1) m_ui->operatorLabel->setText(tr("Oper.: ") + listCops[2]);
        }

        if(cops.toInt()==0)m_ui->opSelModeLabel->setText(tr("Op. sel. mode: auto"));
        else if(cops.toInt()==1)
        {
            oper = listCops[2];
            m_ui->opSelModeLabel->setText(tr("Op. sel. mode: manual ") +oper);
        }
        else if(cops.toInt()==2)m_ui->opSelModeLabel->setText(tr("Op. sel. mode: dereg."));
        else if(cops.toInt()==3)
        {
            format = listCops[1];
            m_ui->opSelModeLabel->setText(tr("Op. sel. mode: set only ") +format);
        }
        else if(cops.toInt()==4)
        {
            oper = listCops[2];
            m_ui->opSelModeLabel->setText(tr("Op. sel. mode: manual ")+oper+" /auto");
        }
        else qDebug() << "networkdialog uncnown Op. sel. mode";
    }
    infoText = getAnswer("AT+CSQ\r\n");
    //qDebug() << "networkdialog get CSQ" << infoText;
    if(infoText.indexOf("+CSQ:")>=0)
    {
        QString csq = infoText.sliced(infoText.indexOf(":")+2);
        //qDebug() << "networkdialog get csq" << csq;
        QStringList listCsq = csq.split(",");
        int rssiPar = listCsq[0].toInt();
        int rssi = rssiPar*2 - 113;
        if(rssiPar==0) m_ui->rssiLabel->setText(tr("RSSI: -113 dBm or less"));
        else if(rssiPar>0 && rssiPar<31) m_ui->rssiLabel->setText(tr("RSSI: ")+QString::number(rssi)+tr(" dBm"));
        else if(rssiPar==31) m_ui->rssiLabel->setText(tr("RSSI: -51 dBm or greater"));
        else if(rssiPar==99) m_ui->rssiLabel->setText(tr("RSSI: ???"));
        else
        {
            m_ui->rssiLabel->setText(tr("RSSI: "));
            qDebug() << "networkdialog wrong csq rssiPar" << rssiPar;
        }
    }
    infoText = getAnswer("AT+CEREG?\r\n");
    //qDebug() << "networkdialog get CEREG" << infoText;
    if(infoText.indexOf("+CEREG:")>=0)
    {
        QString cereg = infoText.sliced(infoText.indexOf(":")+2);
        qDebug() << "networkdialog get cereg" << cereg;
        QStringList listCereg = cereg.split(",");
        if(listCereg[1].toInt()==0)m_ui->regStatusLabel->setText(tr("Reg. status: not registered, not searching"));
        else if(listCereg[1].toInt()==1)m_ui->regStatusLabel->setText(tr("Reg. status: registered, home"));
        else if(listCereg[1].toInt()==2)m_ui->regStatusLabel->setText(tr("Reg. status: not registered, but searching"));
        else if(listCereg[1].toInt()==3)m_ui->regStatusLabel->setText(tr("Reg. status: registration denied"));
        else if(listCereg[1].toInt()==4)m_ui->regStatusLabel->setText(tr("Reg. status: ???"));
        else if(listCereg[1].toInt()==5)m_ui->regStatusLabel->setText(tr("Reg. status: registered, roaming"));
        else if(listCereg[1].toInt()==11)m_ui->regStatusLabel->setText(tr("Reg. status: emergency service available"));
        else m_ui->regStatusLabel->setText(tr("Reg. status: uncnown stat = ") +listCereg[1]);
    }
    m_ui->refreshInfoPushButton->setEnabled(true);
}

void NetworkDialog::on_deregPushButton_clicked()
{
    m_ui->deregPushButton->setEnabled(false);
    emit getData("AT+COPS=2\r\n");//send data to port
    vaitForOk();
    m_ui->deregPushButton->setEnabled(true);
}

void NetworkDialog::on_automodePushButton_clicked()
{
    m_ui->automodePushButton->setEnabled(false);
    emit getData("AT+COPS=0\r\n");//send data to port
    vaitForOk();
    m_ui->automodePushButton->setEnabled(true);
}

void NetworkDialog::on_searchPushButton_clicked()
{
    this->setEnabled(false);
    m_ui->searchPushButton->setEnabled(false);
    QString infoText ="";

    textData = "";//erase console output buffer
    emit getData("AT+COPS=?\r\n");//send data to port
    for(int i=0; i<300; i++){
        delay(600);
        if(textData.indexOf("COPS:")>0) break;
    }//wait for response
    if(textData.indexOf("COPS:")<0)
    {
        //qDebug() << "networkdialog cant get COPS";
        //qDebug() << "networkdialog textData"<< textData;
        m_ui->searchPushButton->setEnabled(true);
        this->setEnabled(true);
        return;
    }
    infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "networkdialog get data0" << infoText;
    if(textData.indexOf("OK")>=0)
    {
        infoText.resize(infoText.size()-8);// cut "OK" from answer
        infoText=infoText.sliced(2);
        //qDebug() << "networkdialog get data1" << infoText;
    }

    infoText = infoText.sliced(6,infoText.indexOf(",,"));
    //qDebug() << "networkdialog get infoText" << infoText;
    QStringList operList = infoText.split(",,");
    operList = operList[0].split("),");
    //qDebug() << "networkdialog get operList" << operList;

    //create columns
    QList<QString> operNames;
    QList<QString> operIds;
    QList<QString> operStatus;
    QList<QString> operAct;
    QStringList header;
    QTableWidgetItem* test=0;

    header<<"Name"<<"ID"<<"Status"<<"Act";

    m_ui->netOpListableWidget->setColumnCount(4);
    m_ui->netOpListableWidget->setRowCount(operList.size());
    m_ui->netOpListableWidget->setHorizontalHeaderLabels(header);

    for(int i=0; i<operList.size();i++)//i == row
    {
        QStringList operL = operList[i].split(",");
        //qDebug() << "networkdialog get operL" << operL;
        operNames.append(operL[1]);
        test = new QTableWidgetItem(operL[1]);
        m_ui->netOpListableWidget->setItem(i,0,test);
        operIds.append(operL[3]);
        test = new QTableWidgetItem(operIds.last());
        m_ui->netOpListableWidget->setItem(i,1,test);

        if(operL[0].indexOf("(")>=0)operL[0]=operL[0].sliced(operL[0].indexOf("(")+1);
        if(operL[0].toInt()==0) operStatus.append(tr("unknown"));
        else if(operL[0].toInt()==1) operStatus.append(tr("available"));
        else if(operL[0].toInt()==2) operStatus.append(tr("current"));
        else if(operL[0].toInt()==3) operStatus.append(tr("forbidden"));
        else  qDebug() << "networkdialog get operList status uncnown" << operL[0];
        test = new QTableWidgetItem(operStatus.last());
        m_ui->netOpListableWidget->setItem(i,2,test);

        if(operL[4].indexOf(")")>=0)operL[4]=operL[4].sliced(0,operL[4].indexOf(")"));
        if(operL[4].toInt()==0) operAct.append("GSM");
        else if(operL[4].toInt()==1) operAct.append("GSM Compact");
        else if(operL[4].toInt()==2) operAct.append("UTRAN");
        else if(operL[4].toInt()==3) operAct.append("GSM w/EGPRS");
        else if(operL[4].toInt()==4) operAct.append("UTRAN w/HSDPA");
        else if(operL[4].toInt()==5) operAct.append("UTRAN w/HSUPA");
        else if(operL[4].toInt()==6) operAct.append("UTRAN w/HSDPA and HSUPA");
        else if(operL[4].toInt()==7) operAct.append("E-UTRAN");
        else  qDebug() << "networkdialog get operList act uncnown" << operL[4];
        test = new QTableWidgetItem(operAct.last());
        m_ui->netOpListableWidget->setItem(i,3,test);
    }

    m_ui->netOpListableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->netOpListableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->netOpListableWidget->show();
    m_ui->searchPushButton->setEnabled(true);
    this->setEnabled(true);
}

void NetworkDialog::on_netOpListableWidget_cellClicked(int row, int column)
{
    //qDebug() << "networkdialog table cli row="<<row<<" column="<<column;
    m_ui->selectOpPushButton->setEnabled(true);
    opSelectedRow=row;
}

void NetworkDialog::on_selectOpPushButton_clicked()
{
    m_ui->selectOpPushButton->setEnabled(false);
    QString check;
    QTableWidgetItem *item = m_ui->netOpListableWidget->item(opSelectedRow,1);
    if (NULL != item) {
       check = item->text();
    }
    //check = check.sliced(check.indexOf("\"")+1,check.lastIndexOf("\"")-check.indexOf("\"")-1);
    //qDebug() << "networkdialog table cli row="<<opSelectedRow<<" id="<< check;
    //AT+COPS=1,2,"25020"
    check = "AT+COPS=1,2,"+ check +"\r\n";
    emit getData(check.toLocal8Bit());//send data to port
    m_ui->selectOpPushButton->setEnabled(true);
}


void NetworkDialog::on_refreshPsdPushButton_clicked()
{
    m_ui->refreshPsdPushButton->setEnabled(false);
    textData = "";//erase console output buffer
    int quantPdp = 0;
    QString infoText = getAnswer("AT+CGDCONT=?\r\n");//get quantity of PDP profiles
    if(infoText.indexOf("CGDCONT:")>=0)
    {
        infoText = infoText.sliced(infoText.indexOf("-")+1,infoText.indexOf(")")-infoText.indexOf("-")-1);
        quantPdp = infoText.toInt();
    } else
    {
        m_ui->refreshPsdPushButton->setEnabled(true);
        qDebug() << "networkdialog wrong answer on CGDCONT = "<<infoText;
        return;
    }

    //create columns
    QStringList header;
    QStringList pdpList;
    QStringList pdpStList;
    QTableWidgetItem* cell=0;

    header<<tr("State")<<tr("Type")<<tr("APN")<<tr("Addr")<<tr("D.Compr.")<<tr("H.Compr.");

    m_ui->psdTableWidget->setColumnCount(6);
    m_ui->psdTableWidget->setRowCount(quantPdp);
    m_ui->psdTableWidget->setHorizontalHeaderLabels(header);

    infoText ="";
    {
        textData = "";//erase console output buffer
        emit getData("AT+CGDCONT?\r\n");//send data to port
        for(int i=0; i<100; i++){
            delay(600);
            if(textData.indexOf("CGDCONT:")>=0) break;
        }//wait for response
        if(textData.indexOf("CGDCONT:")<0)
        {
            qDebug() << "networkdialog cant get 'CGDCONT?'";
            m_ui->refreshPsdPushButton->setEnabled(true);
            return;
        }
        infoText = QString::fromStdString(textData.toStdString());
        //qDebug() << "networkdialog get data0" << infoText;
        if(textData.indexOf("OK")>=0)
        {
            infoText.resize(infoText.size()-8);// cut "OK" from answer
            infoText=infoText.sliced(2);
            //qDebug() << "networkdialog get data1" << infoText;
        }
    }
    pdpList=infoText.split(':');//all pdp profiles ([0] - only)
    //qDebug() << "networkdialog get pdpList" << pdpList;

    infoText ="";
    {
        textData = "";//erase console output buffer
        emit getData("AT+CGACT?\r\n");//send data to port
        for(int i=0; i<100; i++){
            delay(600);
            if(textData.indexOf("CGACT:")>=0) break;
        }//wait for response
        if(textData.indexOf("CGACT:")<0)
        {
            qDebug() << "networkdialog cant get 'CGACT?'";
            m_ui->refreshPsdPushButton->setEnabled(true);
            return;
        }
        infoText = QString::fromStdString(textData.toStdString());
        //qDebug() << "networkdialog get data0" << infoText;
        if(textData.indexOf("OK")>=0)
        {
            infoText.resize(infoText.size()-8);// cut "OK" from answer
            infoText=infoText.sliced(2);
            //qDebug() << "networkdialog get data1" << infoText;
        }
    }
    pdpStList=infoText.split(':');//all pdp profiles status ([0] - only)
    //qDebug() << "networkdialog get pdpStList" << pdpStList;

    if(pdpStList.size()!=pdpList.size()) qDebug() << "networkdialog incompatible size CGACT & CGDCONT";

    for(int i=1;i<pdpList.size();i++)
    {
        QStringList pdp = pdpList[i].split(',');
        //qDebug() << "networkdialog get pdp" << pdp;
        QStringList st = pdpStList[i].split(',');
        //qDebug() << "networkdialog get st" << st;
        int statusPdp = st[1].sliced(0,1).toInt();
        if(statusPdp == 0)
        {
            cell = new QTableWidgetItem(tr("inactive"));
            m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,0,cell);
        }else if(statusPdp == 1)
        {
            cell = new QTableWidgetItem(tr("active"));
            m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,0,cell);
        }else
        {
            qDebug() << "networkdialog wrong status of pdp" << st[1];
        }

        cell = new QTableWidgetItem(pdp[1]);
        m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,1,cell);
        cell = new QTableWidgetItem(pdp[2]);
        m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,2,cell);
        if(pdp.size()>3)
        {
            cell = new QTableWidgetItem(pdp[3]);
            m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,3,cell);
        }
        if(pdp.size()>4)
        {
        cell = new QTableWidgetItem(pdp[4]);
        m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,4,cell);
        }
        if(pdp.size()>5)
        {
        cell = new QTableWidgetItem(pdp[5]);
        m_ui->psdTableWidget->setItem(pdp[0].toInt()-1,5,cell);
        }
    }

    m_ui->psdTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents);
    m_ui->psdTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->netOpListableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->psdTableWidget->show();

    m_ui->refreshPsdPushButton->setEnabled(true);
}


void NetworkDialog::on_psdTableWidget_cellClicked(int row, int column)
{
    //qDebug() << "networkdialog table cli row="<<row<<" column="<<column;

    QTableWidgetItem *item = m_ui->psdTableWidget->item(row,0);
    if (NULL != item) {
       m_ui->adPushButton->setEnabled(true);
    } else m_ui->adPushButton->setEnabled(false);

    m_ui->editPushButton->setEnabled(true);
    pdpSelectedRow=row;
}


void NetworkDialog::on_adPushButton_clicked()
{
    m_ui->adPushButton->setEnabled(false);
    //AT+CGACT=0,[2]
    int selPdp = pdpSelectedRow+1;
    QString selected = QString::number(selPdp);
    //qDebug() << "networkdialog edit row № "<<selPdp;


    QString check;
    QTableWidgetItem *item = m_ui->psdTableWidget->item(pdpSelectedRow,0);
    if (NULL != item) {
       check = item->text();
    }

    if(check=="active")
    {
        QByteArray text = "AT+CGACT=0,\r\n";
        text = text.insert(11, selected.toLocal8Bit());
        //qDebug() << "networkdialog edit row text "<<text;
        emit getData(text);//send data to port
    }
    else if(check=="inactive")
    {
        QByteArray text = "AT+CGACT=1,\r\n";
        text = text.insert(11, selected.toLocal8Bit());
        //qDebug() << "networkdialog edit row text "<<text;
        emit getData(text);//send data to port
    }
    else qDebug() << "networkdialog get wrong pdp state from table "<<check;

    //vaitForOk();
    m_ui->adPushButton->setEnabled(true);
}

void NetworkDialog::on_editPushButton_clicked()
{
    //m_ui->editPushButton->setEnabled(false);
    this->setEnabled(false);
    m_edit->show();
    QString check = "";
    for(int i=1; i<6; i++)
    {
        QTableWidgetItem *item = m_ui->psdTableWidget->item(pdpSelectedRow,i);
        if (NULL != item) {
            check.append(item->text());
            check.append(",");
        }
    }
    emit toEdit(check);
    //m_edit->on_setPushButton_clicked();
}

void NetworkDialog::on_edit(const QString &data)
{
    //qDebug() << "networkdialog get data from edit "<<data;
    int selPdp = pdpSelectedRow+1;
    QString selected = QString::number(selPdp);
    QByteArray text = "AT+CGDCONT=";
    text.append(selected.toLocal8Bit());
    text.append(',');
    text.append(data.toLocal8Bit());
    text.append('\r\n');
    //qDebug() << "networkdialog send data from edit "<<text;
    emit getData(text);//send data to port
    m_edit->close();
    this->setEnabled(true);
}


