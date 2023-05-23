#include "input.h"

Input::Input(QWidget *parent)
    : QLineEdit{parent}
{

    //document()->setMaximumBlockCount(100);
}

void Input::putData(const QString &data)
{
    insert(data);
}

void Input::keyPressEvent(QKeyEvent *e)
{
    switch (e->) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    default:
        if (m_localEchoEnabled)
            QPlainTextEdit::keyPressEvent(e);
        emit getData(e->text().toLocal8Bit());
    }
}

void Input::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    setFocus();
}

void Input::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

void Input::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e);
}
