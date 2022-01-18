#ifndef COLORDIA_H
#define COLORDIA_H

#include <QWidget>
#include <QtDebug>
#include <QColorDialog>

namespace Ui {
class ColorDia;
}

class ColorDia : public QWidget
{
    Q_OBJECT

public:
    explicit ColorDia(QWidget *parent = nullptr);
    ~ColorDia();

private:
    Ui::ColorDia *ui;
};

#endif // COLORDIA_H
