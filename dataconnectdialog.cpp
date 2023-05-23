#include "dataconnectdialog.h"
#include "ui_dataconnectdialog.h"
#include <QFileDialog>

DataConnectDialog::DataConnectDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DataConnectDialog)
{
    m_ui->setupUi(this);
    m_ui->tcpIpPushButton->setEnabled(false);
    m_ui->tcpIpSendPushButton->setEnabled(false);
    m_ui->tcpTransferPushButton->setEnabled(false);
    m_ui->tcpServerPushButton->setEnabled(false);
    m_ui->sendFilePushButton->setEnabled(false);
    m_ui->getFtpPushButton->setEnabled(false);
    m_ui->ipByNamePushButton->setEnabled(false);
    m_ui->ftpConnectPushButton->setEnabled(false);
    m_ui->fotaStartPushButton->setEnabled(false);
    m_ui->fotaProgressBar->setValue(0);

    m_ui->tcpServerPortlineEdit->setValidator(new QIntValidator(1, 65535, this));
    m_ui->tcpServerTimeoutLineEdit->setValidator(new QIntValidator(1, 3600, this));
    m_ui->httpTimeoutLineEdit->setValidator(new QIntValidator(5, 120, this));
    m_ui->tcpIpPortlineEdit->setValidator(new QIntValidator(1, 65535, this));
    m_ui->tcpIpAddrLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
    m_ui->ftpLoginLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
    m_ui->ftpPasswordLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols

    m_ui->fotaFtpPasswordLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
    m_ui->fotaFtpPathLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
    m_ui->fotaFtpPortLineEdit->setValidator(new QIntValidator(1, 65535, this));
    m_ui->fotaFtpServerUrlLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
    m_ui->fotaFtpUsernameLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols

    //connect(m_ui->ftpFileNameLineEdit, &QLineEdit::textChanged, [this](){m_ui->getFtpPushButton->setEnabled(true);});
}

DataConnectDialog::~DataConnectDialog()
{
    delete m_ui;
}

void DataConnectDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void DataConnectDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "dataconnectdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString DataConnectDialog::getAnswer(const QString &data)
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
            if(textData.indexOf("+CME ERROR:")>=0)
            {
                QString codeErr = QString::fromStdString(textData.sliced(textData.indexOf(":")).toStdString());
                m_ui->statusLabel->setText(cmeErrCode(codeErr));
            }
            infoText = QString::fromStdString(textData.toStdString());
            return infoText;
            break;
        }
    }//wait for response
    //return infoText = QString::fromStdString(textData.toStdString());
}

void DataConnectDialog::vaitForOk()
{
    textData = "";//erase console output buffer
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

void DataConnectDialog::on_connectPushButton_clicked()
{
    if(m_ui->connectPushButton->text() == tr("Connect"))
    {
        m_ui->connectPushButton->setEnabled(false);
        textData = "";//erase console output buffer
        emit getData("AT+ZIPCALL=1\r\n");//send data to port
        vaitForOk();
        for(int i=0; i<20; i++){
            delay(500);
            if(textData.indexOf("ZIPCALL")>0)
            {
                QString curip = QString::fromStdString(textData.toStdString());
                QStringList ip = curip.split(",");
                curip = ip[1].sliced(0,ip[1].indexOf("\r"));
                qDebug() << "dataconnectdialog ip" << curip;
                m_ui->tcpIpPushButton->setEnabled(true);
                m_ui->ipByNamePushButton->setEnabled(true);
                m_ui->tcpIpSendPushButton->setEnabled(true);
                m_ui->tcpServerPushButton->setEnabled(true);
                m_ui->ftpConnectPushButton->setEnabled(true);
                m_ui->fotaStartPushButton->setEnabled(true);
                m_ui->ipLabel->setText(curip);
                break;
            }
            if(textData.indexOf("ERROR")>=0)
            {
                m_ui->connectPushButton->setText(tr("Disconnect"));
                m_ui->connectPushButton->setEnabled(true);
                return;
            }
        }//wait for response
        m_ui->connectPushButton->setText(tr("Disconnect"));
    }
    else if (m_ui->connectPushButton->text() == tr("Disconnect"))
    {
        textData = "";//erase console output buffer
        emit getData("AT+ZIPCALL=0\r\n");//send data to port
        vaitForOk();
        for(int i=0; i<20; i++){
            delay(500);
            if(textData.indexOf("OK")>0)
            {
                m_ui->tcpIpPushButton->setEnabled(false);
                m_ui->ipByNamePushButton->setEnabled(false);
                m_ui->tcpIpSendPushButton->setEnabled(false);
                m_ui->tcpServerPushButton->setEnabled(false);
                m_ui->ftpConnectPushButton->setEnabled(false);
                m_ui->fotaStartPushButton->setEnabled(false);
                m_ui->ipLabel->setText("0.0.0.0");
                break;
            }
            if(textData.indexOf("ERROR")>=0)
            {
                m_ui->connectPushButton->setText(tr("Connect"));
                m_ui->connectPushButton->setEnabled(true);
                return;
            }
        }//wait for response
        m_ui->connectPushButton->setText(tr("Connect"));
    }
    m_ui->connectPushButton->setEnabled(true);
}


void DataConnectDialog::on_refreshPushButton_clicked()
{
    m_ui->refreshPushButton->setEnabled(false);

    QString gmrAnswer = getAnswer("AT+GMR\r\n");
    if(gmrAnswer.indexOf("PR1803")>=0)
    {
        model = 1;
        m_ui->ftpAccountPushButton->setHidden(true);
    }
    else if(gmrAnswer.indexOf("PR1603")>=0)
    {
        model = 0;
        m_ui->ftpAccountPushButton->setHidden(false);
    }
    else m_ui->statusLabel->setText("Answer on 'AT+GMR' ="+ gmrAnswer);

    textData = "";//erase console output buffer
    emit getData("AT+ZIPCALL?\r\n");
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>=0) break;
        else if(textData.indexOf("ERROR")>=0)
        {
            m_ui->refreshPushButton->setEnabled(true);
            qDebug() << "dataconnectdialog wrong ZIPCALL?" << textData;
            return;
        }
    }//wait for response
    if(textData.indexOf("OK")<0 && textData.indexOf("ERROR")<0)
    {
        m_ui->statusLabel->setText("Answer on 'AT+ZIPCALL?' ="+ textData);
        m_ui->refreshPushButton->setEnabled(true);
        return;
    }

    QString connState = QString::fromStdString(textData.toStdString());
    QStringList state = connState.split(":");
    connState = state[1].sliced(1,1);
    //qDebug() << "dataconnectdialog zipcall" << connState;
    if(connState == "0")
    {
        m_ui->ipLabel->setText("0.0.0.0");
        m_ui->tcpIpPushButton->setEnabled(false);
        m_ui->ipByNamePushButton->setEnabled(false);
        m_ui->tcpServerPushButton->setEnabled(false);
        m_ui->ftpConnectPushButton->setEnabled(false);
        m_ui->fotaStartPushButton->setEnabled(false);
        m_ui->connectPushButton->setText(tr("Connect"));
    }
    else if (connState == "1")
    {
        m_ui->ipLabel->setText(state[1].sliced(3,state[1].indexOf("\r")-2));
        m_ui->tcpIpPushButton->setEnabled(true);
        m_ui->ipByNamePushButton->setEnabled(true);
        m_ui->tcpServerPushButton->setEnabled(true);
        m_ui->ftpConnectPushButton->setEnabled(true);
        m_ui->fotaStartPushButton->setEnabled(true);
        m_ui->connectPushButton->setText(tr("Disconnect"));
        QString answer = getAnswer("AT+ZIPSTAT=1\r\n");
        //qDebug() << "dataconnectdialog ZIPSTAT" << answer;
        if(answer.last(1)=="1")
        {
            m_ui->tcpIpPushButton->setText(tr("TCP disconnect"));
            m_ui->tcpIpSendPushButton->setEnabled(true);
            m_ui->tcpTransferPushButton->setEnabled(true);
        }
        else
        {
            m_ui->tcpIpPushButton->setText(tr("TCP connect"));
            m_ui->tcpIpSendPushButton->setEnabled(false);
            m_ui->tcpTransferPushButton->setEnabled(false);
        }
    }

    if(model == 0)
    {
        QString answer = getAnswer("AT+ZFTPCFG=account\r\n");
        if(answer.isEmpty())
        {
            m_ui->statusLabel->setText("No answer on AT+ZFTPCFG=account?");
            m_ui->refreshPushButton->setEnabled(true);
        }
        else if(answer.indexOf("ERROR")>=0)
        {
            m_ui->statusLabel->setText("ERROR answer on AT+ZFTPCFG=account?");
            m_ui->refreshPushButton->setEnabled(true);
        }
        else
        {
            QStringList logPass = answer.split(",");
            if(logPass[1].size()>=3) m_ui->ftpLoginLineEdit->setText(logPass[1].sliced(1,logPass[1].size()-2));
            else m_ui->ftpLoginLineEdit->setText("");
            logPass[2]=logPass[2].trimmed();
            if(logPass[2].size()>=3) m_ui->ftpPasswordLineEdit->setText(logPass[2].sliced(1,logPass[2].size()-2));
            else m_ui->ftpPasswordLineEdit->setText("");
        }
    }

    QString servState;
    textData = "";//erase console output buffer
    emit getData("AT+ZIPSLCFG?\r\n");
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("ZIPSLCFG: 0")>0)
        {
            servState = QString::fromStdString(textData.toStdString());
            break;
        }
        if(textData.indexOf("ERROR")>0)
        {
            m_ui->refreshPushButton->setEnabled(true);
            qDebug() << "dataconnectdialog wrong ZIPSLCFG?" << textData;
            return;
        }
    }//wait for response
    //qDebug() << "dataconnectdialog servState" << servState;
    if(servState.indexOf("ZIPSLCFG: 0")>0)
    {
        servState= servState.sliced(servState.indexOf(","));
        qDebug() << "dataconnectdialog state 1" << servState;
        servState= servState.sliced(0,servState.indexOf("+")).trimmed();
        qDebug() << "dataconnectdialog state 2" << servState;
        QStringList state = servState.split(",");
        qDebug() << "dataconnectdialog state" << state;
        m_ui->tcpServerPortlineEdit->setText(state[1]);
        m_ui->tcpServerTimeoutLineEdit->setText(state[2]);
        if(state.size()>3 && state[3]=="1")
        {
            m_ui->tcpSerAsciiCheckBox->setChecked(false);
        }
        else if(state.size()>3 && state[3]=="2")
        {
            m_ui->tcpSerAsciiCheckBox->setChecked(true);
        }
        if(state.size()>4 && state[4]=="1")
        {
            m_ui->tcpSerBufferCheckBox->setChecked(false);
        }
        else if(state.size()>4 && state[4]=="2")
        {
            m_ui->tcpSerBufferCheckBox->setChecked(true);
        }
    }

    //qDebug() << "dataconnectdialog logPass" << logPass;
    m_ui->refreshPushButton->setEnabled(true);
}

QString DataConnectDialog::httpResultCode(const QString &data)
{
    if(data=="0") return "success";
    else if(data=="1") return "Failed to open URL";
    else if(data=="2") return "Failed to add request header";
    else if(data=="3") return "Failed to add request body in AT+ZHTTPPOST";
    else if(data=="4") return "URL not set";
    else if(data=="5") return "Failed to execute request method";
    else if(data=="6") return "Failed to take over the request message";
    else if(data=="7") return "Failed to create socket";
    else return "wrong http err code";
}

QString DataConnectDialog::cmeErrCode(const QString &data)
{
    if(data=="0") return "Phone failure";
    else if(data=="1") return "No connection to phone";
    else if(data=="2") return "phone-adaptor link reserved";
    else if(data=="3") return "Operation not allowed";
    else if(data=="4") return "Operation not allowed";
    else if(data=="5") return "PH-SIM PIN required";
    else if(data=="6") return "PH-FSIM PIN required";
    else if(data=="7") return "PH-FSIM PUK required";
    else if(data=="10") return "SIM not inserted";
    else if(data=="11") return "SIM PIN required";
    else if(data=="12") return "SIM PUK required";
    else if(data=="13") return "SIM failure";
    else if(data=="14") return "SIM busy";
    else if(data=="15") return "SIM wrong";
    else if(data=="16") return "Incorrect password";
    else if(data=="17") return "SIM PIN2 required";
    else if(data=="18") return "SIM PUK2 required";
    else if(data=="20") return "Memory full";
    else if(data=="21") return "Invalid index";
    else if(data=="22") return "Not found";
    else if(data=="23") return "Memory failure";
    else if(data=="24") return "Text string too long";
    else if(data=="25") return "Invalid characters in text string";
    else if(data=="26") return "Dial string too long";
    else if(data=="27") return "Invalid characters in dial string";
    else if(data=="30") return "No network service";
    else if(data=="31") return "Network timer-out";
    else if(data=="32") return "Network not allowed -emergency calls only";
    else if(data=="40") return "Network personalization PIN required";
    else if(data=="41") return "Network personalization PUK required";
    else if(data=="42") return "Network subset personalization PIN required";
    else if(data=="43") return "Network subset personalization PUK required";
    else if(data=="44") return "Service provider personalization PIN required";
    else if(data=="45") return "Service provider personalization PUK required";
    else if(data=="46") return "Corporate personalization PIN required";
    else if(data=="47") return "corporate personalization PUK required";
    else if(data=="48") return "Hidden key required (NOTE: This key is required when accessing hidden phonebook";
    else if(data=="49") return "EAP method not supported";
    else if(data=="50") return "Incorrect parameters";
    else if(data=="100") return "unknown";
    //GPRS-related errors
    //Errors related to a failure to perform an Attach
    else if(data=="103") return "Illegal MS (#3)";
    else if(data=="106") return "Illegal ME (#6)";
    else if(data=="107") return "GPRS services not allowed (#7)";
    else if(data=="111") return "PLMN not allowed (#11)";
    else if(data=="112") return "Location area not allowed (#12)";
    else if(data=="113") return "Roaming not allowed in this location area (#13)";
    else if(data=="132") return "service option not supported (#32)";
    else if(data=="133") return "requested service option not subscribed (#33)";
    else if(data=="134") return "service option temporarily out of order (#34)";
    else if(data=="149") return "PDP authentication failure Other GPRS errors";
    //Other GPRS errors
    else if(data=="150") return "invalid mobile class";
    else if(data=="148") return "unspecified GPRS error";
    //VBS / VGCS and eMLPP -related errors
    else if(data=="151") return "VBS/VGCS not supported by the network";
    else if(data=="152") return "No service subscription on SIM";
    else if(data=="153") return "No subscription for group ID";
    else if(data=="154") return "Group Id not activated on SIM";
    else if(data=="155") return "No matching notification";
    else if(data=="156") return "VBS/VGCS call already present";
    else if(data=="157") return "Congestion";
    else if(data=="158") return "Network failure";
    else if(data=="159") return "Uplink busy";
    else if(data=="160") return "No access rights for SIM file";
    else if(data=="161") return "No subscription for priority";
    else if(data=="162") return "operation not applicable or not possible";
    else return "wrong +CME err code";
}

QString DataConnectDialog::ftpResultCode(const QString &data)
{
    QString protocol_err;
    QString serv_err;

    if(data.indexOf("ZFTP")>0)
    {
        QString tempD = data.sliced(data.indexOf(":")+1).trimmed();
        QStringList ftpOpenState = tempD.split(",");

        if(ftpOpenState[1].toInt()==0) protocol_err = "Execute success";
        else if(ftpOpenState[1].toInt()==230) protocol_err = "User login";
        else if(ftpOpenState[1].toInt()==421) protocol_err = "Unable to provide service, close control connection";
        else if(ftpOpenState[1].toInt()==425) protocol_err = "Can't open data connection";
        else if(ftpOpenState[1].toInt()==426) protocol_err = "Close the connection and abort the transmission";
        else if(ftpOpenState[1].toInt()==450) protocol_err = "The requested file operation was not performed";
        else if(ftpOpenState[1].toInt()==451) protocol_err = "Abort the requested operation: there is a local error";
        else if(ftpOpenState[1].toInt()==452) protocol_err = "The requested operation was not performed: insufficient system storage";
        else if(ftpOpenState[1].toInt()==500) protocol_err = "Incorrect format, command is not recognized";
        else if(ftpOpenState[1].toInt()==501) protocol_err = "Parameter syntax error";
        else if(ftpOpenState[1].toInt()==502) protocol_err = "Command not implemented";
        else if(ftpOpenState[1].toInt()==503) protocol_err = "Command sequence is wrong";
        else if(ftpOpenState[1].toInt()==504) protocol_err = "The command function with this parameter is not implemented";
        else if(ftpOpenState[1].toInt()== 530) protocol_err = "Not logged in";
        else if(ftpOpenState[1].toInt()== 532) protocol_err = "Account information is required to store files";
        else if(ftpOpenState[1].toInt()== 550) protocol_err = "The requested operation was not performed";
        else if(ftpOpenState[1].toInt()== 551) protocol_err = "Request aborted: page type unknown ";
        else if(ftpOpenState[1].toInt()== 552) protocol_err = "Requested file operation aborted, storage allocation overflow";
        else if(ftpOpenState[1].toInt()== 553) protocol_err = "The requested operation was not performed: the file name is invalid";
        else protocol_err = "!!! invalid error code:" +ftpOpenState[1].toInt();


        if(ftpOpenState[0].toInt()==603) serv_err = "Execute success";
        else if(ftpOpenState[0].toInt()==604) serv_err = "DNS resolution failed";
        else if(ftpOpenState[0].toInt()==605) serv_err = "Network error";
        else if(ftpOpenState[0].toInt()==606) serv_err = "Control link is closed";
        else if(ftpOpenState[0].toInt()==607) serv_err = "Data link is closed";
        else if(ftpOpenState[0].toInt()==608) serv_err = "Closed by the peer";
        else if(ftpOpenState[0].toInt()==609) serv_err = "Timeout error";
        else if(ftpOpenState[0].toInt()==610) serv_err = "FTP parameter error";
        else if(ftpOpenState[0].toInt()==611) serv_err = "Failed to open file";
        else if(ftpOpenState[0].toInt()==612) serv_err = "Failed to write file";
        else if(ftpOpenState[0].toInt()==613) serv_err = "Failed to operate file";
        else if(ftpOpenState[0].toInt()==614) serv_err = "The server is unavailable";
        else if(ftpOpenState[0].toInt()==615) serv_err = "Failed to open data connection";
        else if(ftpOpenState[0].toInt()==616) serv_err = "The transmission was aborted";
        else if(ftpOpenState[0].toInt()==625) serv_err = "Not login";
        else if(ftpOpenState[0].toInt()==626) serv_err = "File account required";
        else if(ftpOpenState[0].toInt()==627) serv_err = "Operation not completed";
        else if(ftpOpenState[0].toInt()==629) serv_err = "File operation was aborted";
        else if(ftpOpenState[0].toInt()==630) serv_err = "Invalid file name";
        else if(ftpOpenState[0].toInt()==0) serv_err = "";
        else serv_err = "!!! invalid error code:" +ftpOpenState[0].toInt();
    }
    return "protocol: "+protocol_err+"\r\nserv: " + serv_err;
}

void DataConnectDialog::on_tcpIpPushButton_clicked()
{
    m_ui->tcpIpPushButton->setEnabled(false);
    if(m_ui->tcpIpPushButton->text()==tr("TCP connect"))
    {
        textData = "";//erase console output buffer
        emit getData("AT+ZIPOPEN=1,0,"+ m_ui->tcpIpAddrLineEdit->text().toLocal8Bit()+","+ m_ui->tcpIpPortlineEdit->text().toLocal8Bit()+"\r\n");
        for(int i=0; i<50; i++){
            delay(600);
            if(textData.indexOf("+ZIPRECV:")>=0)//tcp server answered somth
            {
                m_ui->tcpIpPushButton->setText(tr("TCP disconnect"));
                m_ui->tcpIpPushButton->setEnabled(true);
                m_ui->tcpIpSendPushButton->setEnabled(true);
                m_ui->tcpTransferPushButton->setEnabled(true);
                return;
            }
            else if(textData.indexOf("ZIPSTAT: 1,0")>=0)
            {
                m_ui->tcpIpPushButton->setEnabled(true);
                qDebug() << "dataconnectdialog wrong ZIPCALL?" << textData;
                return;
            }
        }//wait for response
        m_ui->statusLabel->setText("Answer on 'AT+ZIPOPEN' ="+ textData);
        m_ui->tcpIpPushButton->setEnabled(true);
        return;
    }
    else if (m_ui->tcpIpPushButton->text()==tr("TCP disconnect"))
    {
        textData = "";//erase console output buffer
        getAnswer("AT+ZIPCLOSE=1\r\n");
        m_ui->tcpIpPushButton->setText(tr("TCP connect"));
        m_ui->tcpIpSendPushButton->setEnabled(false);
        m_ui->tcpTransferPushButton->setEnabled(false);

    }
    m_ui->tcpIpPushButton->setEnabled(true);
}


void DataConnectDialog::on_tcpIpSendPushButton_clicked()
{
    m_ui->tcpIpSendPushButton->setEnabled(false);
    QString answer;
    if(m_ui->tcpIpHexSendCheckBox->isChecked())
    {
        QString mess = m_ui->tcpIpSendLineEdit->text();
        mess = mess.toLocal8Bit().toHex();
        //mess.append('0D0A');
        qDebug() << "dataconnectdialog hex" << mess;
        answer = getAnswer("AT+ZIPSEND=1,"+ mess+"0D0A\r\n");
    }
    else
    {
        answer = getAnswer("AT+ZIPSEND=1,"+ m_ui->tcpIpSendLineEdit->text()+"\r\n");
    }
    m_ui->tcpIpSendPushButton->setEnabled(true);
}


void DataConnectDialog::on_ftpAccountPushButton_clicked()
{
    QString login = m_ui->ftpLoginLineEdit->text();
    QString passwd = m_ui->ftpPasswordLineEdit->text();
    getAnswer("AT+ZFTPCFG=account,"+login+","+passwd+"\r\n");
}


void DataConnectDialog::on_ftpConnectPushButton_clicked()
{
    m_ui->ftpConnectPushButton->setEnabled(false);
    if(m_ui->ftpConnectPushButton->text()==tr("FTP connect"))
    {
        if(model==0)
        {
            textData = "";//erase console output buffer
            if(m_ui->tcpIpPortlineEdit->text().size()>0) emit getData("AT+ZFTPOPEN="+ m_ui->tcpIpAddrLineEdit->text().toLocal8Bit()+","+ m_ui->tcpIpPortlineEdit->text().toLocal8Bit()+"\r\n");
            else emit getData("AT+ZFTPOPEN="+ m_ui->tcpIpAddrLineEdit->text().toLocal8Bit()+"\r\n");
            for(int i=0; i<500; i++){
                delay(500);
                if(textData.indexOf("ZFTPOPEN")>0) break;
            }//wait for response
            QString infoText = QString::fromStdString(textData.toStdString());
            if(infoText.indexOf("ZFTPOPEN")>0)
            {
                if(infoText.indexOf("ZFTPOPEN: 0,230")>0)
                {
                    m_ui->statusLabel->clear();
                    m_ui->statusLabel->setText("protocol: User login");
                    m_ui->ftpConnectPushButton->setText(tr("FTP disconnect"));
                    m_ui->sendFilePushButton->setEnabled(true);
                    m_ui->getFtpPushButton->setEnabled(true);
                }
                else
                {
                    m_ui->sendFilePushButton->setEnabled(false);
                    m_ui->getFtpPushButton->setEnabled(false);
                    infoText = infoText.sliced(infoText.indexOf("ZFTPOPEN")+10).trimmed();
                    QStringList ftpOpenState = infoText.split(",");
                    QString protocol_err;
                    if(ftpOpenState[1].toInt()==0) protocol_err = "Execute success";
                    else if(ftpOpenState[1].toInt()==230) protocol_err = "User login";
                    else if(ftpOpenState[1].toInt()==421) protocol_err = "Unable to provide service, close control connection";
                    else if(ftpOpenState[1].toInt()==425) protocol_err = "Can't open data connection";
                    else if(ftpOpenState[1].toInt()==426) protocol_err = "Close the connection and abort the transmission";
                    else if(ftpOpenState[1].toInt()==450) protocol_err = "The requested file operation was not performed";
                    else if(ftpOpenState[1].toInt()==451) protocol_err = "Abort the requested operation: there is a local error";
                    else if(ftpOpenState[1].toInt()==452) protocol_err = "The requested operation was not performed: insufficient system storage";
                    else if(ftpOpenState[1].toInt()==500) protocol_err = "Incorrect format, command is not recognized";
                    else if(ftpOpenState[1].toInt()==501) protocol_err = "Parameter syntax error";
                    else if(ftpOpenState[1].toInt()==502) protocol_err = "Command not implemented";
                    else if(ftpOpenState[1].toInt()==503) protocol_err = "Command sequence is wrong";
                    else if(ftpOpenState[1].toInt()==504) protocol_err = "The command function with this parameter is not implemented";
                    else if(ftpOpenState[1].toInt()== 530) protocol_err = "Not logged in";
                    else if(ftpOpenState[1].toInt()== 532) protocol_err = "Account information is required to store files";
                    else if(ftpOpenState[1].toInt()== 550) protocol_err = "The requested operation was not performed";
                    else if(ftpOpenState[1].toInt()== 551) protocol_err = "Request aborted: page type unknown ";
                    else if(ftpOpenState[1].toInt()== 552) protocol_err = "Requested file operation aborted, storage allocation overflow";
                    else if(ftpOpenState[1].toInt()== 553) protocol_err = "The requested operation was not performed: the file name is invalid";
                    else protocol_err = "!!! invalid error code:" +ftpOpenState[1].toInt();

                    QString serv_err;
                    if(ftpOpenState[0].toInt()==603) serv_err = "Execute success";
                    else if(ftpOpenState[0].toInt()==604) serv_err = "DNS resolution failed";
                    else if(ftpOpenState[0].toInt()==605) serv_err = "Network error";
                    else if(ftpOpenState[0].toInt()==606) serv_err = "Control link is closed";
                    else if(ftpOpenState[0].toInt()==607) serv_err = "Data link is closed";
                    else if(ftpOpenState[0].toInt()==608) serv_err = "Closed by the peer";
                    else if(ftpOpenState[0].toInt()==609) serv_err = "Timeout error";
                    else if(ftpOpenState[0].toInt()==610) serv_err = "FTP parameter error";
                    else if(ftpOpenState[0].toInt()==611) serv_err = "Failed to open file";
                    else if(ftpOpenState[0].toInt()==612) serv_err = "Failed to write file";
                    else if(ftpOpenState[0].toInt()==613) serv_err = "Failed to operate file";
                    else if(ftpOpenState[0].toInt()==614) serv_err = "The server is unavailable";
                    else if(ftpOpenState[0].toInt()==615) serv_err = "Failed to open data connection";
                    else if(ftpOpenState[0].toInt()==616) serv_err = "The transmission was aborted";
                    else if(ftpOpenState[0].toInt()==625) serv_err = "Not login";
                    else if(ftpOpenState[0].toInt()==626) serv_err = "File account required";
                    else if(ftpOpenState[0].toInt()==627) serv_err = "Operation not completed";
                    else if(ftpOpenState[0].toInt()==629) serv_err = "File operation was aborted";
                    else if(ftpOpenState[0].toInt()==630) serv_err = "Invalid file name";
                    else serv_err = "!!! invalid error code:" +ftpOpenState[0].toInt();

                    m_ui->statusLabel->clear();
                    m_ui->statusLabel->setText("protocol_err: "+protocol_err+"\r\nserv_err: " + serv_err);
                }
            }
            else
            {
                qDebug()<<"err connect:"<<infoText;
            }
        }
        else if (model==1)
        {
            QString answer = getAnswer("AT$ZFTPCFG="+m_ui->tcpIpAddrLineEdit->text()+","+m_ui->ftpLoginLineEdit->text()+","+m_ui->ftpPasswordLineEdit->text()+"\r\n");
        }
    }
    else if (m_ui->ftpConnectPushButton->text()==tr("FTP disconnect"))
    {
        textData = "";//erase console output buffer
        emit getData("AT+ZFTPCLOSE\r\n");
        for(int i=0; i<100; i++){
            delay(500);
            if(textData.indexOf("ZFTPCLOSE")>0) break;
        }//wait for response
        if(textData.indexOf("ZFTPCLOSE")>0)
        {
            QString tempStr = ftpResultCode(textData);
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText(tempStr);
        }
        m_ui->ftpConnectPushButton->setText(tr("FTP connect"));
    }
    m_ui->ftpConnectPushButton->setEnabled(true);
}


void DataConnectDialog::on_sendFilePushButton_clicked()
{
    m_ui->sendFilePushButton->setEnabled(false);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    QStringList name =  fileName.split("/");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    //QDataStream in(&file);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray textFromFile = file.readAll();
    int sizeTextFromFile = textFromFile.size();
    if (sizeTextFromFile>2048)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("File owersize (max = 2048 bytes), size ="+sizeTextFromFile);
        return;
    }
    QString appendFlag = "0";
    if(m_ui->appendFileCheckBox->isChecked()) appendFlag = "1";
    textData = "";//erase console output buffer
    QString size = QString::number(sizeTextFromFile);
    emit getData("AT+ZFTPPUT=" + name.last().toLocal8Bit() + "," + size.toLocal8Bit() + "," + appendFlag.toLocal8Bit() + "\r\n");
    for(int i=0; i<500; i++){
        delay(500);
        if(textData.indexOf("CONNECT")>=0) break;
    }//wait for response
    if(textData.indexOf("CONNECT")<0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Can't connect for upload");
        return;
    }
    else if(textData.indexOf("ZFTPPUT")>0)
    {
        QString tempStr = ftpResultCode(textData);
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText(tempStr);
        return;
    }
    textData = "";//erase console output buffer
    emit getData(textFromFile);
    for(int i=0; i<500; i++){
        delay(500);
        if(textData.indexOf("ZFTPPUT")>0) break;
    }//wait for response
    if(textData.indexOf("ZFTPPUT")<0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Can't get answer after uploading file");
        return;
    }
    //QString tempStr = ftpResultCode(textData);
    //m_ui->statusLabel->setText(tempStr);
    m_ui->sendFilePushButton->setEnabled(true);
}


void DataConnectDialog::on_getFtpPushButton_clicked()
{
    //m_ui->getFtpPushButton->setEnabled(false);
    QString fileName = m_ui->ftpFileNameLineEdit->text();
    if(fileName.isEmpty())
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Type file name on FTP for downloading");
        return;
    }
    //download file
    textData = "";//erase console output buffer
    emit getData("AT+ZFTPGET="+ fileName.toLocal8Bit() + "\r\n");
    for(int i=0; i<500; i++){
        delay(500);
        if(textData.indexOf("CONNECT")>=0) break;
    }//wait for response
    if(textData.indexOf("CONNECT")<0)
    {
        if(textData.indexOf("ZFTPGET")>0)
        {
            QString tempStr = ftpResultCode(textData);
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText(tempStr);
        }
    }
    else
    {
        for(int i=0; i<500; i++){
            delay(500);
            if(textData.indexOf("ZFTPGET")>=0) break;
        }//wait for response
        if(textData.indexOf("OK")<0 || textData.indexOf("ZFTPGET")<0)
        {
            qDebug()<<"can't get end file";
            return;
        }
        else if (textData.indexOf("ZFTPGET: 0,")>0)
        {
            m_ui->getFtpPushButton->setEnabled(false);
            QString infoText = QString::fromStdString(textData.toStdString());
            QStringList downloadedFile = infoText.split("\r\n");
            downloadedFile.remove(1);
            downloadedFile.remove(downloadedFile.indexOf("OK"),downloadedFile.size()-downloadedFile.indexOf("OK"));
            qDebug()<<"downloadedFile"<< downloadedFile;
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("ATCommand File (*.par *.txt)"));
            QFile file(fileName);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            for(int i=1; i<downloadedFile.size(); i++)
            file.write(downloadedFile[i].toLocal8Bit() +"\r\n");
            m_ui->getFtpPushButton->setEnabled(true);
        }
        else if(textData.indexOf("ZFTPGET")>0)
        {
            QString tempStr = ftpResultCode(textData);
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText(tempStr);
            return;
        }
    }
}

void DataConnectDialog::on_tcpTransferPushButton_clicked()
{
    m_ui->tcpTransferPushButton->setEnabled(false);
    if(m_ui->tcpTransferPushButton->text()==tr("Start transfer"))
    {
        textData = "";//erase console output buffer
        emit getData("AT+ZIPCREATE=1\r\n");//send data to port
        for(int i=0; i<10; i++){
            delay(600);
            if (textData.indexOf("CONNECT")>=0)//tcp server answered somth
            {
                m_ui->tcpTransferPushButton->setText(tr("Stop transfer"));
                break;
            }
            else if ((textData.indexOf("E")>=0))
            {
                m_ui->statusLabel->clear();
                m_ui->statusLabel->setText("Can't start transfer");
                break;
            }
        }//wait for response
    }
    else if (m_ui->tcpTransferPushButton->text()==tr("Stop transfer"))
    {
        emit getData("+");
        delay(10);
        emit getData("+");
        delay(10);
        emit getData("+");
        vaitForOk();
        m_ui->tcpTransferPushButton->setText(tr("Start transfer"));
    }
    m_ui->tcpTransferPushButton->setEnabled(true);
}


void DataConnectDialog::on_ipByNamePushButton_clicked()
{
    m_ui->ipByNamePushButton->setEnabled(false);
    static QRegularExpression numIp ("\\b[a-zA-Z0-9_]+\\.[a-z]{2,4}\\b");
    if(!numIp.match(m_ui->tcpIpAddrLineEdit->text()).hasMatch())
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Need IP name like ioprst.net");
        m_ui->ipByNamePushButton->setEnabled(true);
        return;
    }
    m_ui->statusLabel->clear();
    QString tempAns = getAnswer("AT+ZDNSGETIP=" + m_ui->tcpIpAddrLineEdit->text().toLocal8Bit()+ "\r\n");
    if(tempAns.indexOf("ZDNSGETIP")>0)
    {
        qDebug()<<"tempAns"<<tempAns;
        m_ui->ipFromNamelabel->setText(tempAns.sliced(tempAns.indexOf(":")+1).trimmed());
    }
    else if(tempAns.indexOf("ERROR")>0)
    {
        qDebug()<<"resolve ERROR";
    }

    m_ui->ipByNamePushButton->setEnabled(true);
}


void DataConnectDialog::on_tcpServerPushButton_clicked()
{
    m_ui->tcpServerPushButton->setEnabled(false);
    QString answer;
    if(m_ui->tcpServerPushButton->text()==tr("Listen"))
    {
        answer = getAnswer("AT+ZIPLISTEN=0,1\r\n");
        if (answer.indexOf("OK")>=0) m_ui->tcpServerPushButton->setText(tr("Stop"));
    }
    else if (m_ui->tcpServerPushButton->text()==tr("Stop"))
    {
        answer = getAnswer("AT+ZIPLISTEN=0,0\r\n");
        if (answer.indexOf("OK")>=0)m_ui->tcpServerPushButton->setText(tr("Listen"));
    }
    m_ui->tcpServerPushButton->setEnabled(true);
}


void DataConnectDialog::on_tcpSerSettingsPushButton_clicked()
{
    m_ui->tcpSerSettingsPushButton->setEnabled(false);

    QString ascii = "1";
    QString buffer = "1";
    if(m_ui->tcpSerAsciiCheckBox->isChecked())ascii = "2";
    if(m_ui->tcpSerBufferCheckBox->isChecked())buffer = "2";

    QString answer = getAnswer("AT+ZIPSLCFG=0,"+ m_ui->tcpServerPortlineEdit->text()+","+ m_ui->tcpServerTimeoutLineEdit->text()+","+ascii+","+buffer+"\r\n");
    if (answer.indexOf("E")>=0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Can't set server settings");
    }

    m_ui->tcpSerSettingsPushButton->setEnabled(true);
}


void DataConnectDialog::on_httpSetUrlPushButton_clicked()
{
    m_ui->httpUrlLineEdit->setEnabled(false);

    QString answer = getAnswer("AT+ZHTTPURL=" + m_ui->httpUrlLineEdit->text() + "\r\n");
    qDebug()<<"answer"<<answer;
    if (answer.indexOf("E")>=0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText("Can't set http url");
    }

    m_ui->httpUrlLineEdit->setEnabled(true);
}


void DataConnectDialog::on_httpGetPushButton_clicked()
{
    m_ui->httpGetPushButton->setEnabled(false);

    QString infoText;
    textData = "";//erase console output buffer
    emit getData("AT+ZHTTPGET\r\n");
    for(int i=0; i<500; i++){
        delay(500);
        if(textData.indexOf("ZHTTPGET")>=0)
        {
            infoText = QString::fromStdString(textData.toStdString());
            break;
        }
        if(textData.indexOf("ERROR")>=0)
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Can't set GET request");
            break;
        }
    }//wait for response
    if(infoText.indexOf("ZHTTPGET")>=0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText(httpResultCode(infoText.at(infoText.indexOf("ZHTTPGET")+10)));
    }

    m_ui->httpGetPushButton->setEnabled(true);
}


void DataConnectDialog::on_httpPostPushButton_clicked()
{
    m_ui->httpPostPushButton->setEnabled(false);

    QString infoText;
    textData = "";//erase console output buffer
    emit getData("AT+ZHTTPPOST=10240,"+m_ui->httpTimeoutLineEdit->text().toLocal8Bit()+"\r\n");
    for(int i=0; i<500; i++){
        delay(500);
        if(textData.indexOf("CONNECT")>=0)
        {
            infoText = QString::fromStdString(textData.toStdString());
            break;
        }
        if(textData.indexOf("ERROR")>=0)
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Can't set POST request");
            break;
        }
    }//wait for response
    if(infoText.indexOf("ZHTTPGET")>=0)
    {
        m_ui->statusLabel->clear();
        m_ui->statusLabel->setText(httpResultCode(infoText.at(infoText.indexOf("ZHTTPGET")+10)));
    }

    m_ui->httpPostPushButton->setEnabled(true);
}


void DataConnectDialog::on_httpSettingsPushButton_clicked()
{
    m_ui->httpSettingsPushButton->setEnabled(false);

    QString resp = "0";
    if(m_ui->httpResponseheaderCheckBox->isChecked())resp = "1";
    getAnswer("AT+ZHTTPCFG=responseheader," + resp + "\r\n");

    getAnswer("AT+ZHTTPCFG=customheader,Accept: " + m_ui->httpDataTypeComboBox->currentText() + "\r\n");

    getAnswer("AT+ZHTTPCFG=customheader,Connection: " + m_ui->httpConnectionComboBox->currentText() + "\r\n");

    getAnswer("AT+ZHTTPCFG=tls_securitylevel,: " + m_ui->httpSecurityComboBox->currentText() + "\r\n");

    getAnswer("AT+ZHTTPCFG=timeout," + m_ui->httpTimeoutLineEdit->text() + "\r\n");

    m_ui->httpSettingsPushButton->setEnabled(true);
}


void DataConnectDialog::on_fotaStartPushButton_clicked()
{
    this->setEnabled(false);

    QString infoText;
    textData = "";//erase console output buffer
    if(m_ui->fotaProtocolComboBox->currentText()=="FTP")
    {
        if(m_ui->fotaFtpUsernameLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill username for ftp");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpPasswordLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill password for ftp");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpServerUrlLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill server URL for ftp");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpPathLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill server path for ftp");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpPortLineEdit->text().isEmpty()) emit getData("AT+ZFOTADL=ftp://" + m_ui->fotaFtpUsernameLineEdit->text().toLocal8Bit()+":" + m_ui->fotaFtpPasswordLineEdit->text().toLocal8Bit()+ "@" + m_ui->fotaFtpServerUrlLineEdit->text().toLocal8Bit() + "/" + m_ui->fotaFtpPathLineEdit->text().toLocal8Bit() + "\r\n");
        else emit getData("AT+ZFOTADL=ftp://" + m_ui->fotaFtpUsernameLineEdit->text().toLocal8Bit()+":" + m_ui->fotaFtpPasswordLineEdit->text().toLocal8Bit()+ "@" + m_ui->fotaFtpServerUrlLineEdit->text().toLocal8Bit() +":" + m_ui->fotaFtpPortLineEdit->text().toLocal8Bit() + "/" +m_ui->fotaFtpPathLineEdit->text().toLocal8Bit() + "\r\n");
    }
    else if(m_ui->fotaProtocolComboBox->currentText()=="HTTP")
    {
        if(m_ui->fotaFtpServerUrlLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill server URL for http");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpPathLineEdit->text().isEmpty())
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Fill server path for http");
            this->setEnabled(true);
            return;
        }
        if(m_ui->fotaFtpPortLineEdit->text().isEmpty()) emit getData("AT+ZFOTADL=http://" + m_ui->fotaFtpServerUrlLineEdit->text().toLocal8Bit() + "/" + m_ui->fotaFtpPathLineEdit->text().toLocal8Bit() + "\r\n");
        else emit getData("AT+ZFOTADL=http://" + m_ui->fotaFtpServerUrlLineEdit->text().toLocal8Bit() +":" + m_ui->fotaFtpPortLineEdit->text().toLocal8Bit()+ "/" + m_ui->fotaFtpPathLineEdit->text().toLocal8Bit() + "\r\n");
    }
    for(int i=0; i<50; i++){
        delay(600);
        if(textData.indexOf("OK")>=0)
        {
            infoText = QString::fromStdString(textData.toStdString());
            break;
        }
        if(textData.indexOf("ERROR")>=0)
        {
            m_ui->statusLabel->clear();
            m_ui->statusLabel->setText("Can't update firmware");
            break;
        }
    }//wait for response

    if(textData.indexOf("OK")>=0)
    {
        for(int i=0; i<5000; i++){
            delay(600);
            if(textData.indexOf("ZFOTA: UPDATE BEGINE")>0) m_ui->fotaProgressBar->setEnabled(true);
            else if(textData.indexOf("ZFOTA: UPDATEING 100")>0) m_ui->fotaProgressBar->setValue(100);
            else if(textData.indexOf("ZFOTA: UPDATEING 10")>0) m_ui->fotaProgressBar->setValue(10);
            else if(textData.indexOf("ZFOTA: UPDATEING 20")>0) m_ui->fotaProgressBar->setValue(20);
            else if(textData.indexOf("ZFOTA: UPDATEING 30")>0) m_ui->fotaProgressBar->setValue(30);
            else if(textData.indexOf("ZFOTA: UPDATEING 40")>0) m_ui->fotaProgressBar->setValue(40);
            else if(textData.indexOf("ZFOTA: UPDATEING 50")>0) m_ui->fotaProgressBar->setValue(50);
            else if(textData.indexOf("ZFOTA: UPDATEING 60")>0) m_ui->fotaProgressBar->setValue(60);
            else if(textData.indexOf("ZFOTA: UPDATEING 70")>0) m_ui->fotaProgressBar->setValue(70);
            else if(textData.indexOf("ZFOTA: UPDATEING 80")>0) m_ui->fotaProgressBar->setValue(80);
            else if(textData.indexOf("ZFOTA: UPDATEING 90")>0) m_ui->fotaProgressBar->setValue(90);
            else if(textData.indexOf("ZFOTA:UPDATE SUCCESS")>0) break;

            if(textData.indexOf("ERROR")>=0)
            {
                m_ui->statusLabel->clear();
                m_ui->statusLabel->setText("Can't update firmware");
                break;
            }
        }//wait for response
    }

    this->setEnabled(true);
}


void DataConnectDialog::on_fotaProtocolComboBox_currentIndexChanged(int index)
{
    if(index == 1)
    {
        m_ui->fotaFtpPasswordLineEdit->setEnabled(false);
        m_ui->fotaFtpUsernameLineEdit->setEnabled(false);
    }
    if(index == 0)
    {
        m_ui->fotaFtpPasswordLineEdit->setEnabled(true);
        m_ui->fotaFtpUsernameLineEdit->setEnabled(true);
    }
}

