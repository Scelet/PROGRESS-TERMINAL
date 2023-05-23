#include "globals.h"
#include "notebookdialog.h"
#include "ui_notebookdialog.h"
#include "editpdpdialog.h"
#include "editcommanddialog.h"

#include <QFileDialog>

NotebookDialog::NotebookDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NotebookDialog),
    m_edit(new EditPdpDialog),
    m_cEdit(new EditPdpDialog)
{
    m_ui->setupUi(this); 

    m_ui->groupCommandsComboBox->setEditable(true);
    m_ui->editCommandsPushButton->setEnabled(false);
    m_ui->addCommansPushButton->setEnabled(false);
    m_ui->deleteCommandsPushButton->setEnabled(false);
    m_ui->saveCommandsPushButton->setEnabled(false);
    m_ui->editGroupsPushButton->setEnabled(false);
    m_ui->deleteGroupsPushButton->setEnabled(false);

    connect(m_edit, &EditPdpDialog::finished, [this](){this->setEnabled(true);});

    connect(m_edit, &EditPdpDialog::sendData, this, &NotebookDialog::on_edit);
    connect(this, &NotebookDialog::toEdit, m_edit, &EditPdpDialog::on_start);

    connect(m_cEdit, &EditPdpDialog::finished, [this](){this->setEnabled(true);});

    connect(m_cEdit, &EditPdpDialog::sendData, this, &NotebookDialog::on_cEdit);
    connect(this, &NotebookDialog::toComEdit, m_cEdit, &EditPdpDialog::on_start);

    QStringList header;
    header<<"Commmand"<<"Comments";
    m_ui->commandsTableWidget->setColumnCount(2);
    m_ui->commandsTableWidget->setHorizontalHeaderLabels(header);
    m_ui->commandsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //slNotebook -> curNotebook | convert [QStringList] to [struct notebook] for restore from settings
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    QStringList slNotebook = settings.value("NotebookDialog/curNotebook").toStringList();
    //qDebug()<<"slNotebook ="<< slNotebook;
    if(!slNotebook.isEmpty())
    {
        for (int i=0;i<slNotebook.size();i++) {
            QStringList data = slNotebook[i].split("|");
            //qDebug() << "Text in data :" << data;
            if(data[0]=="group" && data.size()==2)
            {
                appendNew();//append new element to 'curNotebook' array
                curNotebook[notebookAmount-1].groupName = data[1];
                m_ui->groupCommandsComboBox->addItem(data[1]);
            }
            else if(data[0]=="item" && data.size()==4)
            {
                //qDebug() << " test notebookAmount :" << notebookAmount;
                for(int x=0; x<notebookAmount; x++)
                {
                    if(curNotebook[x].groupName == data[1])
                    {
                            curNotebook[x].comments.append(data[3]);
                            curNotebook[x]._atCommand.append(data[2]);
                    }
                }
            }
        }
        emit m_ui->groupCommandsComboBox->activated(0);
        m_ui->saveCommandsPushButton->setEnabled(true);
        m_ui->editGroupsPushButton->setEnabled(true);
        m_ui->addCommansPushButton->setEnabled(true);
    }

}

NotebookDialog::~NotebookDialog()
{
    //qDebug()<<"destruct NotebookDialog";
    //slNotebook <- curNotebook | convert [struct notebook] to [QStringList] for save in settings
    QStringList slNotebook;
    if(curNotebook!=0 && notebookAmount!=0)
    {
        for(int i=0; i<notebookAmount;i++) slNotebook.append("group|"+ curNotebook[i].groupName.toLocal8Bit());
        for(int i=0; i<notebookAmount;i++) for(int j=0; j<curNotebook[i]._atCommand.size();j++)  slNotebook.append("item|"+ curNotebook[i].groupName.toLocal8Bit()+"|"+ curNotebook[i]._atCommand[j].toLocal8Bit()+"|"+ curNotebook[i].comments[j].toLocal8Bit());
    }    
    //qDebug()<<"QStringList for save in settings ="<< slNotebook;

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.beginGroup("NotebookDialog");
    settings.setValue("curNotebook", slNotebook);
    settings.endGroup();

    delete m_ui;
}


void NotebookDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void NotebookDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "networkdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

QString NotebookDialog::getAnswer(const QString &data)
{
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
    QString infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "networkdialog get data0" << infoText;
    infoText.resize(infoText.size()-8);// cut "OK" from answer
    infoText=infoText.sliced(2);
    //qDebug() << "networkdialog get data1" << infoText;
    return infoText;
}

void NotebookDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

void NotebookDialog::appendNew()
{
    m_ui->saveCommandsPushButton->setEnabled(true);
    m_ui->editGroupsPushButton->setEnabled(true);
    m_ui->addCommansPushButton->setEnabled(true);
    m_ui->deleteGroupsPushButton->setEnabled(true);

    if (notebookAmount == 0)
    {
        delete [] curNotebook;
        curNotebook = new notebook[notebookAmount + 1]; // first struct memory
    }
    else
    {
        notebook* tempObj = new notebook[notebookAmount + 1];

        for (int i = 0; i < notebookAmount; i++)
        {
            tempObj[i] = curNotebook[i]; // copy to temp obj
        }
        delete [] curNotebook;

        curNotebook = tempObj;
    }
    notebookAmount++;
}

void NotebookDialog::on_editGroupsPushButton_clicked()
{
    qDebug()<<m_ui->groupCommandsComboBox->currentIndex();
    if(m_ui->groupCommandsComboBox->currentIndex()>=0)
    {
        this->setEnabled(false);
        m_edit->show();
        emit toEdit(m_ui->groupCommandsComboBox->currentText());

    }
    else return;// can edit only existed group ???add worning???
}

void NotebookDialog::on_edit(const QString &data)//edit curNotebook[curIndex].groupName
{
    int curIndex = m_ui->groupCommandsComboBox->currentIndex();
    m_ui->groupCommandsComboBox->setItemText(curIndex,data);
    curNotebook[curIndex].groupName = data;
    //if(curNotebook==0){}else if(curIndex==0){}else{}
    m_edit->close();
    this->setEnabled(true);
}

void NotebookDialog::on_loadCommandsPushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ATCommand File"), "", tr("ATCommand File (*.par):: All Files (*)"));
    if (fileName.isEmpty()) return;
    else {
        QFile file(fileName);
        //QDataStream in(&file);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString textFromFile;
        while (!file.atEnd()) {
            textFromFile = file.readLine();
            textFromFile = textFromFile.trimmed();
            //textFromFile = textFromFile.append("\r\n");
            //qDebug() << "Text from file :" << textFromFile;
            QStringList data = textFromFile.split("|");
            //qDebug() << "Text in data :" << data;
            if(data[0]=="group" && data.size()==2)
            {
                appendNew();//append new element to 'curNotebook' array
                curNotebook[notebookAmount-1].groupName = data[1];
                m_ui->groupCommandsComboBox->addItem(data[1]);
            }
            else if(data[0]=="item" && data.size()==4)
            {
                //qDebug() << " test notebookAmount :" << notebookAmount;
                for(int x=0; x<notebookAmount; x++)
                {
                    if(curNotebook[x].groupName == data[1])
                    {
                            curNotebook[x].comments.append(data[3]);
                            curNotebook[x]._atCommand.append(data[2]);
                    }
                }
            }
        }
        qDebug() << "notebookAmount :" << notebookAmount;
    }    
    m_ui->groupCommandsComboBox->setCurrentIndex(0);
    emit m_ui->groupCommandsComboBox->activated(0);
}



void NotebookDialog::on_commandsTableWidget_cellClicked(int row, int column)//send to editStrng
{
    //qDebug() << "NotebookDialog table cli row="<<row<<" column="<<column;
    m_ui->editCommandsPushButton->setEnabled(true);
    m_ui->deleteCommandsPushButton->setEnabled(true);
    currentRow = row;
    currentColumn = column;
    //qDebug() << "text row="<<m_ui->commandsTableWidget->item(row,0);
    if(m_ui->commandsTableWidget->item(row,0)!=0)
    emit getLEData(m_ui->commandsTableWidget->item(row,0)->text());//send data to lineEdit on main window
}


void NotebookDialog::on_commandsTableWidget_cellDoubleClicked(int row, int column)//send to console
{
    m_ui->editCommandsPushButton->setEnabled(true);
    m_ui->deleteCommandsPushButton->setEnabled(true);
    currentRow = row;
    currentColumn = column;
    //qDebug() << "NotebookDialog table cli row="<<row<<" column="<<column;
    QString comm = m_ui->commandsTableWidget->item(row,0)->text() +"\r\n";
    emit getData( comm.toLocal8Bit());//send data to port
}

void NotebookDialog::on_editCommandsPushButton_clicked()
{
    this->setEnabled(false);
    m_cEdit->show();
    if(m_ui->commandsTableWidget->item(currentRow,currentColumn)!=0) emit toComEdit(m_ui->commandsTableWidget->item(currentRow,currentColumn)->text());
}

void NotebookDialog::on_cEdit(const QString &data)//edit curNotebook[curIndex]._atCommand[] or curNotebook[curIndex].comments[]
{
    m_ui->commandsTableWidget->item(currentRow,currentColumn)->setText(data);
    int curIndex = m_ui->groupCommandsComboBox->currentIndex();
    if(currentColumn==0)
    {
        curNotebook[curIndex]._atCommand[currentRow] = data;
    }
    else if(currentColumn==1)
    {
        curNotebook[curIndex].comments[currentRow] = data;
    }
    m_cEdit->close();
    this->setEnabled(true);
}

void NotebookDialog::on_addCommansPushButton_clicked()
{
    int curIndex = m_ui->groupCommandsComboBox->currentIndex();
    int rows = curNotebook[curIndex]._atCommand.size();
    //qDebug()<<"rows :"<< rows<< " index:" << curIndex << " notebookAmount:" << notebookAmount;
    m_ui->commandsTableWidget->setRowCount(rows+1);

    QTableWidgetItem* cell=0;
    cell = new QTableWidgetItem("?");
    m_ui->commandsTableWidget->setItem(rows,0,cell);
    cell = new QTableWidgetItem("?");
    m_ui->commandsTableWidget->setItem(rows,1,cell);

    curNotebook[curIndex]._atCommand.append("?");
    curNotebook[curIndex].comments.append("?");

    m_ui->commandsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}


void NotebookDialog::on_saveCommandsPushButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save ATCommand File"), "", tr("ATCommand File (*.par *.txt)"));
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write("version|0.0.0|alpha\n");
    for(int i=0; i<notebookAmount;i++) file.write("group|"+ curNotebook[i].groupName.toLocal8Bit() +"\n");
    for(int i=0; i<notebookAmount;i++) for(int j=0; j<curNotebook[i]._atCommand.size();j++) file.write("item|"+ curNotebook[i].groupName.toLocal8Bit()+"|"+ curNotebook[i]._atCommand[j].toLocal8Bit()+"|"+ curNotebook[i].comments[j].toLocal8Bit() +"\n");
}


void NotebookDialog::on_groupCommandsComboBox_activated(int index)
{
    m_ui->editCommandsPushButton->setEnabled(false);
    m_ui->deleteCommandsPushButton->setEnabled(false);
    currentRow = 0;
    currentColumn = 0;

    if(curNotebook==0 || index>=notebookAmount)//add new group on start or append it to existed list by pressing ENTER on ComboBox
    {
        appendNew();//append new element to 'curNotebook' array
        curNotebook[notebookAmount-1].groupName = m_ui->groupCommandsComboBox->currentText();
    }
    //show curNotebook .comments and ._atCommand in table
    {
        QTableWidgetItem* cell=0;
        //show curNotebook[notebookAmount].comments & curNotebook[notebookAmount]._atCommand
        //qDebug() << "groupName :" << curNotebook[index].groupName << "comments :" << curNotebook[index].comments.size() << "_atCommand :" << curNotebook[index]._atCommand.size();

        int rows=curNotebook[index].comments.size();
        m_ui->commandsTableWidget->setRowCount(rows);

        for(int x=0; x<rows; x++)
        {
            cell = new QTableWidgetItem(curNotebook[index]._atCommand[x]);
            m_ui->commandsTableWidget->setItem(x,0,cell);
            cell = new QTableWidgetItem(curNotebook[index].comments[x]);
            m_ui->commandsTableWidget->setItem(x,1,cell);
        }
        m_ui->commandsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}

void NotebookDialog::on_groupCommandsComboBox_currentIndexChanged(int index){}
void NotebookDialog::on_groupCommandsComboBox_currentTextChanged(const QString &arg1){}

void NotebookDialog::on_deleteGroupsPushButton_clicked()
{
    int j=0;
    notebook* tempObj = new notebook[notebookAmount -1];
    for(int i=0; i<notebookAmount;i++)
    {
        if(i!=m_ui->groupCommandsComboBox->currentIndex())
        {
        tempObj[j] = curNotebook[i];
        j++;
        }
    }
    delete [] curNotebook;
    curNotebook = tempObj;
    //for(int i=0; i<notebookAmount-1;i++)qDebug()<<"groups"<<curNotebook[i].groupName;
    notebookAmount--;
    m_ui->groupCommandsComboBox->removeItem(m_ui->groupCommandsComboBox->currentIndex());
    if(notebookAmount==0)
    {
        m_ui->deleteGroupsPushButton->setEnabled(false);
        m_ui->addCommansPushButton->setEnabled(false);
        m_ui->editCommandsPushButton->setEnabled(false);
        m_ui->editGroupsPushButton->setEnabled(false);
        m_ui->saveCommandsPushButton->setEnabled(false);
        m_ui->commandsTableWidget->clear();
    }
    else emit m_ui->groupCommandsComboBox->activated(0);
}


void NotebookDialog::on_deleteCommandsPushButton_clicked()
{    
    int curIndex = m_ui->groupCommandsComboBox->currentIndex();
    qDebug()<<"rows :"<< currentRow<< " index:" << curIndex;
    m_ui->commandsTableWidget->removeRow(currentRow);
    curNotebook[curIndex]._atCommand.removeAt(currentRow);
    curNotebook[curIndex].comments.removeAt(currentRow);
    m_ui->commandsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    if(curNotebook[curIndex]._atCommand.isEmpty()) m_ui->deleteCommandsPushButton->setEnabled(false);
}


