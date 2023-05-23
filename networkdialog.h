#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>

namespace Ui {
class NetworkDialog;
}

class EditPdpDialog;

class NetworkDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);
    void toEdit(const QString &data);    

public:
    explicit NetworkDialog(QWidget *parent = nullptr);
    ~NetworkDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);

public slots:

    void on_edit(const QString &data);

private slots:
    void on_refreshInfoPushButton_clicked();

    void on_deregPushButton_clicked();

    void on_automodePushButton_clicked();

    void on_searchPushButton_clicked();

    void on_netOpListableWidget_cellClicked(int row, int column);

    void on_selectOpPushButton_clicked();

    void on_refreshPsdPushButton_clicked();

    void on_psdTableWidget_cellClicked(int row, int column);

    void on_adPushButton_clicked();

    void on_editPushButton_clicked();

private:
    void delay(int i);
    Ui::NetworkDialog *m_ui;
    EditPdpDialog *m_edit;
    QByteArray textData;//for console output
    int opSelectedRow=0;
    int pdpSelectedRow=0;
};

#endif // NETWORKDIALOG_H
