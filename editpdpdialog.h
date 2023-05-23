#ifndef EDITPDPDIALOG_H
#define EDITPDPDIALOG_H

#include <QDialog>
#include <QValidator>

namespace Ui {
class EditPdpDialog;
}

class EditPdpDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendData(const QString &data);

public:
    explicit EditPdpDialog(QWidget *parent = nullptr);
    ~EditPdpDialog();

public slots:
    void on_start(const QString &data);

private slots:
    void on_setPushButton_clicked();

private:
    Ui::EditPdpDialog *m_ui;
};

#endif // EDITPDPDIALOG_H
