#include "editcommanddialog.h"
#include "ui_editcommanddialog.h"

EditCommandDialog::EditCommandDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::EditCommandDialog)
{
    m_ui->setupUi(this);
}

EditCommandDialog::~EditCommandDialog()
{
    delete m_ui;
}

void EditCommandDialog::on_sendPushButton_clicked()
{
    QList<QString> commandData;

    commandData.append(m_ui->commandTitleLineEdit->text());
    commandData.append(m_ui->commandLineEdit->text());
    commandData.append(m_ui->commandInfoPlainTextEdit->toPlainText());
    emit sendData(commandData);
}

