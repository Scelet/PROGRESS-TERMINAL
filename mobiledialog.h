#ifndef MOBILEDIALOG_H
#define MOBILEDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>

QT_BEGIN_NAMESPACE

namespace Ui {
class MobileDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class MobileDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit MobileDialog(QWidget *parent = nullptr);
    ~MobileDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);

private slots:
    void showModemInfo();

    void on_voiceInputSlider_valueChanged(int value);

    void on_voiceOutputSlider_valueChanged(int value);

    void on_dialPpushButton_clicked();

    void on_hangPushButton_clicked();

    void on_answerPushButton_clicked();

    void on_sendPushButton_clicked();

private:
    void delay(int i);
    Ui::MobileDialog *m_ui = nullptr;
    QByteArray textData;//for console output
};
#endif // MOBILEDIALOG_H
