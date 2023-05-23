#ifndef BLUETOOTHDIALOG_H
#define BLUETOOTHDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>

namespace Ui {
class BluetoothDialog;
}

class BluetoothDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit BluetoothDialog(QWidget *parent = nullptr);
    ~BluetoothDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);

private slots:
    void on_bltPowerPushButton_clicked();

    void on_visPushButton_clicked();

    void on_searchPushButton_clicked();

private:
    Ui::BluetoothDialog *m_ui;
    QByteArray textData;//for console output
    void delay(int i);
};

#endif // BLUETOOTHDIALOG_H
