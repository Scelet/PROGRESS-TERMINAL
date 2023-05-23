#include "globals.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "mobiledialog.h"
#include "gnssdialog.h"
#include "networkdialog.h"
#include "bluetoothdialog.h"
#include "dataconnectdialog.h"
#include "editcommanddialog.h"
#include "notebookdialog.h"
#include "bandsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>

#define COMM_S "commands"
#define COMM "command"
#define DESCR "descriprtion"

QString jsonATCommandText;
QFile jsonATCommandFile;
QFile consoleLog;
QJsonArray jsonArrayAT;
QJsonArray jsonArrayATCommand;
QJsonArray jsonArrayATdescriprtion;
QJsonParseError parseError;
QStack<QString> lineEditMem;
QMenu* userButton;


//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),

    m_settings(new SettingsDialog(this)),
    m_mobile(new MobileDialog(this)),
    m_gnss (new GnssDialog(this)),
    m_network (new NetworkDialog(this)),
    m_blt (new BluetoothDialog(this)),
    m_data (new DataConnectDialog(this)),
    m_edit (new EditCommandDialog(this)),
    m_note (new NotebookDialog(this)),
    m_band (new BandsDialog(this)),
//! [1]
    m_serial(new QSerialPort(this))
//! [1]
{
//! [0]
    m_ui->setupUi(this);

    this->setWindowTitle(APPLICATION_NAME);

    m_console->setEnabled(false);
    m_console->setReadOnly(true);

    //m_ui->mdiArea->addSubWindow(m_console)->showMaximized();
    //setCentralWidget(m_console);
    m_ui->verticalLayout->addWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->menuLoadS->setEnabled(false);
    m_ui->menuUI->setEnabled(false);    
    m_ui->checkBoxRN->setChecked(true);

    m_ui->actionStopScript->setEnabled(false);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    consoleLog.setFileName("../log.txt");
    consoleLog.open(QIODevice::WriteOnly | QIODevice::Text);

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

//! [2]
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2] console input
    connect(m_console, &Console::getData, this, &MainWindow::writeData);
    connect(m_mobile, &MobileDialog::getData, this, &MainWindow::writeData);
    connect(m_gnss, &GnssDialog::getData, this, &MainWindow::writeData);
    connect(m_network, &NetworkDialog::getData, this, &MainWindow::writeData);
    connect(m_blt, &BluetoothDialog::getData, this, &MainWindow::writeData);
    connect(m_data, &DataConnectDialog::getData, this, &MainWindow::writeData);
    connect(m_note, &NotebookDialog::getData, this, &MainWindow::writeData);
    connect(m_band, &BandsDialog::getData, this, &MainWindow::writeData);

    connect(m_ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_lineEdit_returnPressed);
    connect(m_ui->actionNotebook, &QAction::triggered,[this]()
            {
                if(m_note->isHidden()){
                    //int mainPosY = this->frameGeometry().width();
                    QPoint mainPos = this->mapToGlobal(QPoint(this->frameGeometry().width(),0));
                    m_note->move(mainPos);
                    m_note->show();
                }
                else m_note->hide();
            });
    connect(m_ui->actionBands, &QAction::triggered, m_band, &BandsDialog::show);

    connect(m_console, &Console::textChanged, this, &MainWindow::writeToLog);

//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    //qDebug()<<"destruct main";
    consoleLog.close();
    delete m_settings;
    delete m_ui;
}

void MainWindow::delay(int n)//time in msec
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

//! [4]
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        //m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->menuLoadS->setEnabled(true);
        m_ui->menuUI->setEnabled(true);
        m_network->setEnabled(true);
        m_gnss->setEnabled(true);
        m_mobile->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name, p.stringBaudRate, p.stringDataBits,
                               p.stringParity, p.stringStopBits, p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->menuLoadS->setEnabled(false);
    m_ui->menuUI->setEnabled(false);
    m_network->setEnabled(false);
    m_gnss->setEnabled(false);
    m_mobile->setEnabled(false);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Progress Terminal"),
    tr("The <b>Progress Terminal</b> designed to control modems"
    "via AT commands"));
}

void MainWindow::aboutScript()
{
    QMessageBox::about(this, tr("About script commands"),
    tr("@sleep [int] | set delay where is [int] - wait in msec<br>"
    "@loop [int] | set cycle where is [int] - number of repetitions<br>"
    "@endloop | end of cycle<br>"
    "*support degree of nesting 2 for cycle"));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    QTextCursor cursor = m_console->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_console->setTextCursor(cursor);

    const SettingsDialog::Settings p = m_settings->settings();
    //qDebug() << "writeData to ser" << data;
    m_serial->write(data);
    if(p.localEchoEnabled)
    {
        if(m_mobile->isActiveWindow()) m_console->insertPlainText(data);//add input from m_mobile to console
        if(m_gnss->isActiveWindow()) m_console->insertPlainText(data);//add input from m_gnss to gnss
        if(m_network->isActiveWindow()) m_console->insertPlainText(data);//add input from m_network to gnss
        if(m_blt->isActiveWindow()) m_console->insertPlainText(data);//add input from m_blt to gnss
        if(m_data->isActiveWindow()) m_console->insertPlainText(data);//add input from m_data to gnss
        if(m_note->isActiveWindow()) m_console->insertPlainText(data);//add input from m_note to gnss
        if(m_band->isActiveWindow()) m_console->insertPlainText(data);//add input from m_band to gnss
    }
}
//! [6]

//! [7]
void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    //qDebug() << "readData from ser" << data;
    m_console->putData(data);
    if(m_mobile->isVisible()) m_mobile->putData(data);//add output from mobile to console
    if(m_gnss->isVisible()) m_gnss->putData(data);//add output from m_gnss to console
    if(m_network->isVisible()) m_network->putData(data);//add output from m_network to console ( ||  m_network->m_edit->isActiveWindow())
    if(m_blt->isVisible()) m_blt->putData(data);//add output from m_network to console
    if(m_data->isVisible()) m_data->putData(data);//add output from m_data to console
    if(m_note->isVisible()) m_note->putData(data);//add output from m_note to console
    if(m_band->isVisible()) m_band->putData(data);//add output from m_band to console
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);    
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionClear, &QAction::triggered, m_ui->lineEdit, &QLineEdit::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAbout_Scripts, &QAction::triggered, this, &MainWindow::aboutScript);
    connect(m_ui->actionMobile, &QAction::triggered, m_mobile, &MobileDialog::show);
    connect(m_ui->actionGNSS, &QAction::triggered, m_gnss, &GnssDialog::show);
    connect(m_ui->actionNetwork, &QAction::triggered, m_network, &GnssDialog::show);
    connect(m_ui->actionBT, &QAction::triggered, m_blt, &BluetoothDialog::show);
    connect(m_ui->actionData_connection, &QAction::triggered, m_data, &DataConnectDialog::show);
    connect(m_note, &NotebookDialog::getLEData, this, &MainWindow::setLEData);
    connect( m_ui->actionStopScript, &QAction::triggered, [this]()
    {
        stopScriptlag=true;
        m_ui->actionStopScript->setEnabled(false);
    });

//    {//replaced by NotebookDialog
//        userButton = m_ui->menuAT->addMenu(tr("rewriteble"));
//        QAction* userCommandButton = userButton->addAction(tr("+add new+"));
//        connect(userCommandButton, &QAction::triggered, m_edit, &EditCommandDialog::show);
//        connect(m_edit, &EditCommandDialog::sendData, this, &MainWindow::on_commandEdit);
//        //QAction* userCommandButton = userButton->addAction("");
//    }//add user command dictionary

    QFile file("menuAT.par");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString menuLine =  QString::fromStdString(line.toStdString());
        QStringList atMenu = menuLine.split("|");
        QMenu* ATButton = m_ui->menuAT->addMenu(atMenu[0].sliced(1,atMenu[0].size()-2));
        QString tempStr = atMenu[1].sliced(1,atMenu[1].size()-2);
        QStringList atAction = tempStr.split('"', Qt::SkipEmptyParts);
        for(int i=0; i<atAction.size();i++)
        {
            QAction* ATcommandButton = ATButton->addAction(atAction[i]);
            QStringList atComm = atAction[i].split(" ");
            connect(ATcommandButton, &QAction::triggered, [this,atComm]()
            {
                m_ui->lineEdit->clear();
                m_ui->lineEdit->insert(atComm[0]);
            });
        }
    }
//    else
//    {//read pdf manual -> menuAT
//        QPdfDocument AT_Commands_Manual;
//        QString chapterName, miniChapterName, microChapterName;
//        QMap<QString, QStringList> atMenu;
//        QString nameMenu;
//        QMenu* ATButton;
//        int chapterNum = 0;

//        AT_Commands_Manual.load("./GOSUNCN_GM520_Module_AT_Commands_Manual_V1.0_20220609.pdf");
//        qDebug()<<"PDF error"<<AT_Commands_Manual.error();
//        //qDebug()<<"PDF pageLabel"<<AT_Commands_Manual.metaData(QPdfDocument::MetaDataField::Author);
//        //qDebug()<<"PDF pageCount"<<AT_Commands_Manual.pageCount();
//        int startPage = 0;
//        for(int page=0; page<AT_Commands_Manual.pageCount(); page++)//find start page in table of contents by ["ATI Request identification information"]
//        {
//            QPdfSelection pdfView=AT_Commands_Manual.getAllText(page);//all text from page
//            QString pdfText = pdfView.text();

//            if(pdfText.indexOf("ATI Request identification information")>=0)
//            {
//                //qDebug()<<"pdfText"<<pdfText;
//                QStringList pdfString = pdfText.split("\r\n");
//                for(int i=0; i<pdfString.size();i++)
//                {
//                    if(pdfString[i].indexOf("ATI Request identification information")>=0)
//                    {
//                        //qDebug()<<"spec pdfString = "<< pdfString[i];
//                        QStringList pdfWords = pdfString[i].split(" ");
//                        startPage = pdfWords[pdfWords.size()-1].toInt();
//                        break;
//                    }
//                }
//                break;
//            }
//            startPage=page;
//        }
//        //qDebug()<<"PDF start page = "<< startPage;
//        for(int page = startPage; page<AT_Commands_Manual.pageCount(); page++)
//        {
//            QPdfSelection pdfView=AT_Commands_Manual.getAllText(page);//all text from page
//            QString pdfText = pdfView.text();
//            QStringList pdfString = pdfText.split("\r\n");
//            for(int i=0; i<pdfString.size();i++)//all strings splitted by [\r\n] from text from page
//            {
//                if(pdfString[i].indexOf(" "))
//                {
//                    QStringList wordInString = pdfString[i].split(" ");
//                    //qDebug()<<"PDF text"<<wordInString;
//                    for(int j=0; j<wordInString.size();j++)//all words splitted by [ ] fron string from text from page
//                    {
//                        if(wordInString[0].indexOf(".")>=0) //chapters with [Note]  &&  pdfString[i-1].indexOf("Note")<0
//                        {
//                            QStringList numbers = wordInString[j].split(".");

//                            bool ok;
//                            numbers[0].toInt(&ok, 10);//check that first is number - not like [characters.]
//                            if(wordInString[j].last(1) == "." && ok && numbers.size()==2) //chapters like [3.]
//                            {
//                                if(chapterNum==0 || chapterNum==numbers[0].toInt()-1)//only if chapters is serial
//                                {
//                                    chapterName = pdfString[i].sliced(pdfString[i].indexOf(" ")+1);
//                                    //qDebug()<<"PDF chapter"<<chapterName;
//                                    ATButton = m_ui->menuAT->addMenu(chapterName);
//                                    chapterNum = numbers[0].toInt();

//                                    nameMenu = chapterName;//for saving in QMap
//                                }
//                            } else if (wordInString[j].last(1) == "." && ok && numbers.size()==3) //chapters like [3.1.]
//                            {
//                                miniChapterName = pdfString[i].sliced(pdfString[i].indexOf(" ")+1);
//                                if(wordInString[1].size()>2 && wordInString[1].indexOf("AT")==0)//only [AT*] command, no any  no [+CMTI] or [MQTT] or [* Example]
//                                {
//                                    QAction* ATcommandButton = ATButton->addAction(miniChapterName);

//                                    atMenu[nameMenu].append(miniChapterName);//saving in QMap
//                                    //ATcommandButton->setToolTip();
//                                    connect(ATcommandButton, &QAction::triggered, [this,wordInString]()
//                                    {
//                                        m_ui->lineEdit->clear();
//                                        m_ui->lineEdit->insert(wordInString[1]);
//                                    });
//                                }
//                                //qDebug()<<"PDF mini chapter"<<miniChapterName;
//                            } else if (wordInString[j].last(1) == "." && ok && numbers.size()==4)//chapters like [19.8.1.]
//                            {
//                                microChapterName = pdfString[i].sliced(pdfString[i].indexOf(" ")+1);
//                                //qDebug()<<"PDF micro chapter"<<microChapterName;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        //qDebug()<<"PDF text"<<pdfString;
//        qDebug()<<"atMenu text"<<atMenu;

//        //write atMenu to file
//        QFile file("menuAT.par");
//        file.open(QIODevice::WriteOnly | QIODevice::Text);
//        foreach (QString key, atMenu.keys())
//        {
//            file.write('"'+key.toLocal8Bit()+'"'+'|');
//            for(int i=0; i<atMenu.value(key).size();i++)
//            {
//                file.write('"'+atMenu.value(key)[i].toLocal8Bit()+'"');
//            }
//            file.write("\n");
//        }
//    }//This block add AT commands as [QMenu* ATButton][QAction* ATcommandButton] from .pdf file


//    {//read json file -> menuAT
//        jsonATCommandFile.setFileName("../at/DataProcessedAT.kt");
//        jsonATCommandFile.open(QIODevice::ReadOnly | QIODevice::Text);
//        jsonATCommandText = jsonATCommandFile.readAll();
//        //qDebug() << "jsonconnectSendDebug text " << jsonATCommandText;
//        jsonATCommandFile.close();
//        QJsonDocument doc = QJsonDocument::fromJson(jsonATCommandText.toUtf8(), &parseError);
//        qDebug() << "JSon file parse error code: " << parseError.error << "err: " << parseError.errorString();
//        //qDebug() << "Text from file:" << doc;

//        QJsonObject json = doc.object();
//        //qDebug() << "jsonconnectSendDebug " << json;
//        jsonArrayAT = json[COMM_S].toArray();
//        //qDebug() << "jsonconnectSendDebug " << jsonArrayAT[1];

//        for (int ID = 0; ID < jsonArrayAT.size(); ID++) {

//            QJsonObject obj = jsonArrayAT[ID].toObject();
//            //qDebug() << "Command " << obj[COMM].toString();

//            QAction* ATButton = m_ui->menuAT->addAction(obj[COMM].toString());
//            //ATButton->setToolTip(obj[DESCR].toString());
//            //qDebug() << "Description " << obj[DESCR].toString();
//            //connect(ATButton, &QAction::triggered, [this,obj](){m_console->insertPlainText(obj[COMM].toString());});
//            connect(ATButton, &QAction::triggered, [this,obj](){m_ui->lineEdit->insert(obj[COMM].toString());});
//            connect(ATButton, &QAction::triggered, [this,obj](){m_status->setText(obj[DESCR].toString());});

//        }
//    }//This block add AT commands as [QAction* ATButton] from JSon file

    connect(m_ui->actionScript, &QAction::triggered, this, &MainWindow::on_actionScript_loadFile);
    connect(m_ui->actionFile, &QAction::triggered, this, &MainWindow::on_actionFile_loadFile);
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::on_lineEdit_returnPressed()
{
    QTextCursor cursor = m_console->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_console->setTextCursor(cursor);

    const SettingsDialog::Settings p = m_settings->settings();
    QString test = m_ui->lineEdit->text();
    QByteArray data = test.toLocal8Bit();
    if(!lineEditMem.contains(test))lineEditMem.push(test);
    if(m_ui->checkBoxHEX->isChecked())data = data.toHex();
    if(m_ui->checkBoxRN->isChecked()) {        
        if(p.localEchoEnabled) m_console->insertPlainText(data+"\r\n");
        m_serial->write(data+"\r\n");
    }   else {
        if(p.localEchoEnabled) m_console->insertPlainText(data);
        m_serial->write(data);
    }
}

void MainWindow::on_actionFile_loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script File"), "", tr("Script File (*.jps):: All Files (*)"));
    if (fileName.isEmpty()) return;
    else {
        m_ui->actionStopScript->setEnabled(true);
        QFile file(fileName);
        //QDataStream in(&file);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString textFromFile;
        while (!file.atEnd()) {
            textFromFile = file.readLine();
            QByteArray data = textFromFile.toLocal8Bit();
            if(stopScriptlag)
            {
                stopScriptlag=false;
                return;
            }
            m_serial->write(data);
            //qDebug() << "Text in [QByteArray] from file :" << data;
            m_console->insertPlainText(data);
        }
        m_ui->actionStopScript->setEnabled(false);
    }
}

void MainWindow::on_actionScript_loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script File"), "", tr("Script File (*.jps):: All Files (*)"));
    if (fileName.isEmpty()) return;
    else {
        m_ui->actionStopScript->setEnabled(true);
        bool loopFlag=false;
        int loopNum=0;
        bool loopFlag2=false;
        int loopNum2=0;
        QStringList loopCommands;
        QStringList loopCommands2;
        QFile file(fileName);
        //QDataStream in(&file);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString textFromFile;
        while (!file.atEnd()) {
            //Add interrupt by [Load->StopScript] button
            textFromFile = file.readLine();
            textFromFile = textFromFile.trimmed();

            //add script commands with @

            if(textFromFile.indexOf("@sleep")==0 && loopNum<2)//sleep out of loop
            {
                delay(textFromFile.sliced(7).toInt());
                continue;
            }
            else if(textFromFile.indexOf("@loop")==0 && loopNum<2)//first degree of nesting loop
            {
                loopNum = textFromFile.sliced(6).toInt();
                continue;
            }
            else if(textFromFile.indexOf("@loop")==0 && loopNum>1)//2 degree of nesting loop
            {
                loopNum2 = textFromFile.sliced(6).toInt();
                continue;
            }
            else if(textFromFile.indexOf("@endloop")==0)
            {
                if(loopNum2>1)loopFlag2=true;//end 2 degree of nesting loop
                else if(loopNum>1)loopFlag = true;//end first degree of nesting loop
            }
            //its all script commands

            //qDebug() << "loopNum :" << loopNum <<  " loopFlag :" << loopFlag;


            if(loopNum>1 && !loopFlag && loopNum2<1)      //collect all command for the first degree of nesting loop
            {
                loopCommands.append(textFromFile.append("\r\n"));
                continue;
            }
            else if (loopNum2>1 && !loopFlag2) //collect all command for the 2 degree of nesting loop
            {
                loopCommands2.append(textFromFile.append("\r\n"));
                continue;
            }
            else if (loopFlag2) //ends 2 degree of nesting loop
            {

                for(int i=0;i<loopNum2;i++) loopCommands.append(loopCommands2);
                loopNum2=0;
                loopFlag2=false;
                continue;
            }
            else if (loopFlag) //ends first degree of nesting loop
            {
                for(int i=0;i<loopNum;i++)for(int j=0;j<loopCommands.size();j++)
                {
                    if(loopCommands[j].indexOf("@sleep")==0)
                    {
                        delay(loopCommands[j].sliced(7).toInt());
                        continue;
                    }
                    if(stopScriptlag)
                    {
                        stopScriptlag=false;
                        return;
                    }
                    //qDebug() << "Text in loop from file :" << loopCommands[j];
                    QByteArray data = loopCommands[j].toLocal8Bit();
                    m_serial->write(data);
                    //qDebug() << "Text in [QByteArray] from file :" << data;
                    m_console->insertPlainText(loopCommands[j]);
                }
                loopNum=0;
                loopFlag=false;
                continue;
            }

            if(stopScriptlag)
            {
                stopScriptlag=false;
                return;
            }
            textFromFile = textFromFile.append("\r\n");

            //Add script words processing

            //qDebug() << "Text from file :" << textFromFile;
            QByteArray data = textFromFile.toLocal8Bit();
            m_serial->write(data);
            //qDebug() << "Text in [QByteArray] from file :" << data;
            m_console->insertPlainText(textFromFile);
        }
        m_ui->actionStopScript->setEnabled(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    //int pos = 0;
    qDebug() << "Event from LineEdit :" << event->type();
    if(event->key() == Qt::Key_Up && !lineEditMem.empty()) {
        qDebug() << "Key up";
        m_ui->lineEdit->clear();
        m_ui->lineEdit->insert(lineEditMem.at(pos));
        if(pos>0)pos--;
    }
    if(event->key() == Qt::Key_Down && !lineEditMem.empty()) {
        qDebug() << "Key down";
        m_ui->lineEdit->clear();
        m_ui->lineEdit->insert(lineEditMem.at(pos));
        if(pos<(lineEditMem.size()-1))pos++;
    }
}

void MainWindow::writeToLog()
{
    QByteArray newConsoleText = m_console->toPlainText().toLocal8Bit();//get new text
    QByteArray changes;
    if(consoleText>"" && newConsoleText.contains(consoleText))
    {
        changes= newConsoleText.sliced(consoleText.size());
        //qDebug()<<"text"<<changes;
    }
    else changes= newConsoleText;
    //qDebug()<<"text"<<changes;
    consoleLog.write(changes);
    consoleText = newConsoleText;//save text for next time
}

void MainWindow::on_commandEdit(const QList<QString> &message)
{
    //qDebug()<<"menu size"<<m_ui->menuAT->actions().size();
    bool coincidenceFlag = false;
    QList <QAction*> allMenuAT = m_ui->menuAT->actions();

    for(int i=0; i<allMenuAT.size(); i++)
    {
        //qDebug()<<"menu names"<<allMenuAT[i]->text();
        if(allMenuAT[i]->text()==message[0])
        {
            QMenu* newMenu = allMenuAT[i]->menu();
            QAction* userNewButton = newMenu->addAction(message[1]+message[2]);
            connect(userNewButton, &QAction::triggered, [this,message](){m_ui->lineEdit->insert(message[1]);});
            coincidenceFlag = true;
        }
    }
    if(!coincidenceFlag)
    {
    QMenu* newMenu = m_ui->menuAT->addMenu(message[0]);
    QAction* userNewButton = newMenu->addAction(message[1]+message[2]);
    connect(userNewButton, &QAction::triggered, [this,message](){m_ui->lineEdit->insert(message[1]);});
    }
}

void MainWindow::setLEData(const QString &data)
{
    m_ui->lineEdit->clear();
    m_ui->lineEdit->insert(data);
}
