#ifndef INPUT_H
#define INPUT_H


//#include <QObject>
//#include <QWidget>
#include <QLineEdit>


class Input : public QLineEdit
{
    Q_OBJECT
public:
    explicit Input(QWidget *parent = nullptr);
    void putData(const QString &data);

//signals:
//    void getData(const QString &data);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
};

#endif // INPUT_H
