#ifndef DATACONNECTDIALOG_H
#define DATACONNECTDIALOG_H

#include <QDialog>
#include <QValidator>
#include <QRegularExpression>
#include <QTimer>
#include <QEventLoop>

namespace Ui {
class DataConnectDialog;
}

class DataConnectDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit DataConnectDialog(QWidget *parent = nullptr);
    ~DataConnectDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);
    QString ftpResultCode(const QString &data);
    QString cmeErrCode(const QString &data);
    QString httpResultCode(const QString &data);

private slots:

    void on_refreshPushButton_clicked();

    void on_tcpIpPushButton_clicked();

    void on_connectPushButton_clicked();

    void on_tcpIpSendPushButton_clicked();

    void on_ftpAccountPushButton_clicked();

    void on_ftpConnectPushButton_clicked();

    void on_sendFilePushButton_clicked();

    void on_getFtpPushButton_clicked();

    void on_tcpTransferPushButton_clicked();

    void on_ipByNamePushButton_clicked();

    void on_tcpServerPushButton_clicked();

    void on_tcpSerSettingsPushButton_clicked();

    void on_httpSetUrlPushButton_clicked();

    void on_httpGetPushButton_clicked();

    void on_httpPostPushButton_clicked();

    void on_httpSettingsPushButton_clicked();

    void on_fotaStartPushButton_clicked();

    void on_fotaProtocolComboBox_currentIndexChanged(int index);

private:
    Ui::DataConnectDialog *m_ui;
    QByteArray textData;//for console output
    void delay(int i);
    int model = 0;//0=1603;1=1803;
};

#endif // DATACONNECTDIALOG_H
