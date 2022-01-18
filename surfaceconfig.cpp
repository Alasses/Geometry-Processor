#include "surfaceconfig.h"
#include "ui_surfaceconfig.h"

SurfaceConfig::SurfaceConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SurfaceConfig)
{
    ui->setupUi(this);
}

SurfaceConfig::~SurfaceConfig()
{
    delete ui;
}
