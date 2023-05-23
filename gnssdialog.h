#ifndef GNSSDIALOG_H
#define GNSSDIALOG_H

#include <QDialog>

#include <QTimer>
#include <QEventLoop>
#include <QtCharts>

QT_BEGIN_NAMESPACE

namespace Ui {
class GnssDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class GnssDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit GnssDialog(QWidget *parent = nullptr);
    ~GnssDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);
    QList<QStringList> parseGSV(const QString &data);

private slots:
    void on_refreshPushButton_clicked();

    void on_StopRadioButton_clicked(bool checked);

    void on_SingleRadioButton_clicked(bool checked);

    void on_trackRadioButton_clicked(bool checked);

    void on_gpsRadioButton_clicked(bool checked);

    void on_bduRadioButton_clicked(bool checked);

    void on_gpsBduRadioButton_clicked(bool checked);

    void on_gpsGloRadioButton_clicked(bool checked);

    void on_gpsGalRadioButton_clicked(bool checked);

    void on_gpsGloGalRadioButton_clicked(bool checked);

    void on_gpsBduGalRadioButton_clicked(bool checked);

    void on_gnssResetPushButton_clicked();

private:
    void delay(int i);
    Ui::GnssDialog *m_ui = nullptr;
    QChartView *m_sigStrGraf = nullptr;
    QByteArray textData;//for console output

};

#endif // GNSSDIALOG_H
