#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

#include <QDate>
#include <QFile>
#include <QDataStream>
/* for obsolete part in void MainWindow::initActionsConnections() replaced by <QtPdf/QPdfDocument>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonParseError>
*/

//#include <QtPdf/QPdfDocument>

#include <QEvent>
#include <QStack>

#include <QTimer>
#include <QEventLoop>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class MobileDialog;
class GnssDialog;
class NetworkDialog;
class BluetoothDialog;
class DataConnectDialog;
class EditCommandDialog;
class NotebookDialog;
class BandsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void aboutScript();
    void writeData(const QByteArray &data);
    void writeToLog();
    void readData();

    void handleError(QSerialPort::SerialPortError err);    

    void on_lineEdit_returnPressed();
    void on_actionScript_loadFile();
    void on_actionFile_loadFile();
    void on_commandEdit(const QList<QString> &message);
    void setLEData(const QString &data);

private:
    void initActionsConnections();
    void keyPressEvent(QKeyEvent* event);

private:
    void showStatusMessage(const QString &message);
    void delay(int i);

    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    Console *m_console = nullptr;
    SettingsDialog *m_settings = nullptr;
    MobileDialog *m_mobile = nullptr;
    GnssDialog *m_gnss = nullptr;
    NetworkDialog *m_network = nullptr;
    BluetoothDialog *m_blt = nullptr;
    DataConnectDialog *m_data = nullptr;
    EditCommandDialog *m_edit = nullptr;
    NotebookDialog *m_note = nullptr;
    BandsDialog *m_band = nullptr;
    QSerialPort *m_serial = nullptr;

    bool stopScriptlag = false;
    int pos=0;
    QByteArray consoleText="";
};

#endif // MAINWINDOW_H
