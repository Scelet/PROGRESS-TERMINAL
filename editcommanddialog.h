#ifndef EDITCOMMANDDIALOG_H
#define EDITCOMMANDDIALOG_H

#include <QDialog>

namespace Ui {
class EditCommandDialog;
}

class EditCommandDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendData(const QList<QString> &data);

public:
    explicit EditCommandDialog(QWidget *parent = nullptr);
    ~EditCommandDialog();

private slots:
    void on_sendPushButton_clicked();

private:
    Ui::EditCommandDialog *m_ui;
};

#endif // EDITCOMMANDDIALOG_H
