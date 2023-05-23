#include "editpdpdialog.h"
#include "ui_editpdpdialog.h"
#include <QRegularExpression>


EditPdpDialog::EditPdpDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::EditPdpDialog)
{
    m_ui->setupUi(this);
    m_ui->pdpLineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^а-яА-Я]*")));//forbid Cyrillic symbols
}

EditPdpDialog::~EditPdpDialog()
{
    delete m_ui;
}

void EditPdpDialog::on_setPushButton_clicked()
{
    emit sendData(m_ui->pdpLineEdit->text());
}

void EditPdpDialog::on_start(const QString &data){
    m_ui->pdpLineEdit->setText(data);
}
