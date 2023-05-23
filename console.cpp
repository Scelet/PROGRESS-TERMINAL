#include "console.h"

#include <QScrollBar>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor::fromRgb(255, 255, 255, 200));
    //p.setBrush(QPalette::Base, QBrush(QPixmap(":/images/bakground.png")));
    p.setColor(QPalette::Text, QColor::fromRgb(0, 45, 114, 255));//progress blue
    setPalette(p);
}

void Console::putData(const QByteArray &data)
{
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());

    QTextCharFormat textCharFormat = this->currentCharFormat();
    textCharFormat.setForeground(QColor::fromRgb(138, 21, 56, 255));//progress red
    this->setCurrentCharFormat(textCharFormat);
    insertPlainText(data);
    textCharFormat.setForeground(QColor::fromRgb(0, 45, 114, 255));//progress blue
    this->setCurrentCharFormat(textCharFormat);
    insertPlainText("\r\n");

//    QString coloredText = "<font color='red'>"+data+"</font><br>";
//    appendHtml(coloredText);//color works but lost /r/n

    bar->setValue(bar->maximum());
}

//void Console::setLocalEchoEnabled(bool set)
//{
//    m_localEchoEnabled = set;
//}

//void Console::keyPressEvent(QKeyEvent *e)
//{

//    switch (e->key()) {
//    case Qt::CTRL + Qt::Key_V:
//    case Qt::Key_Backspace:
//    case Qt::Key_Left:
//    case Qt::Key_Right:
//    case Qt::Key_Up:

//    case Qt::Key_Down:
//        break;
//    default:
//        if (m_localEchoEnabled)
//            QPlainTextEdit::keyPressEvent(e);
//        emit getData(e->text().toLocal8Bit());

//    }
//}

//void Console::mousePressEvent(QMouseEvent *e)
//{
//    Q_UNUSED(e);
//    setFocus();
//}

//void Console::mouseDoubleClickEvent(QMouseEvent *e)
//{
//    Q_UNUSED(e);
//}

//void Console::contextMenuEvent(QContextMenuEvent *e)
//{
//    Q_UNUSED(e);
//}

