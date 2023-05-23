#ifndef BANDSDIALOG_H
#define BANDSDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>

namespace Ui {
class BandsDialog;
}

class BandsDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit BandsDialog(QWidget *parent = nullptr);
    ~BandsDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);
    QString hexToName(const QString &group, const QString &data);
    QString nameToHex(const QString &group, const QString &data);
    QList<QStringList> getAllBands();
    QList<QStringList> getCurBands();

private slots:
    void on_bandRefreshPushButton_clicked();

    void on_BandsActivePushButton_clicked();

    void on_bandsTableWidget_cellClicked(int row, int column);

private:
    Ui::BandsDialog *m_ui;
    QByteArray textData;//for console output
    void delay(int i);
    int selectedRow=0;
};

#endif // BANDSDIALOG_H
