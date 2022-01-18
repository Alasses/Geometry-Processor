#include "colordia.h"
#include "ui_colordia.h"

ColorDia::ColorDia(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorDia)
{
    ui->setupUi(this);

    QColor color = QColorDialog::getColor();
}

ColorDia::~ColorDia()
{
    delete ui;
}
