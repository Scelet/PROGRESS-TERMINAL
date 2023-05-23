#ifndef NOTEBOOKDIALOG_H
#define NOTEBOOKDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>
#include <QSettings>

namespace Ui {
class NotebookDialog;
}

class EditPdpDialog;

class NotebookDialog : public QDialog
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);
    void getLEData(const QString &data);
    void toEdit(const QString &data);
    void toComEdit (const QString &data);

public:
    explicit NotebookDialog(QWidget *parent = nullptr);
    ~NotebookDialog();

    void putData(const QByteArray &data);
    void vaitForOk();
    QString getAnswer(const QString &data);

public slots:
    void on_edit(const QString &data);
    void on_cEdit(const QString &data);

private slots:
    void on_editGroupsPushButton_clicked();

    void on_loadCommandsPushButton_clicked();

    void on_groupCommandsComboBox_currentIndexChanged(int index);

    void on_commandsTableWidget_cellClicked(int row, int column);

    void on_commandsTableWidget_cellDoubleClicked(int row, int column);

    void on_editCommandsPushButton_clicked();

    void on_addCommansPushButton_clicked();

    void on_saveCommandsPushButton_clicked();

    void on_groupCommandsComboBox_activated(int index);

    void on_deleteGroupsPushButton_clicked();

    void on_deleteCommandsPushButton_clicked();

    void on_groupCommandsComboBox_currentTextChanged(const QString &arg1);

private:
    Ui::NotebookDialog *m_ui;
    EditPdpDialog *m_edit;
    EditPdpDialog *m_cEdit;
    struct notebook
    {
        QString groupName;
        QList <QString> _atCommand;
        QList <QString> comments;
    };
    void appendNew();
    void delay(int i);
    QByteArray textData;//for console output
    notebook* curNotebook = 0;
    int notebookAmount = 0;
    int currentRow = 0;
    int currentColumn =0;
};

#endif // NOTEBOOKDIALOG_H
