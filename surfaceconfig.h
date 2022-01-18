#ifndef SURFACECONFIG_H
#define SURFACECONFIG_H

#include <QDialog>

namespace Ui {
class SurfaceConfig;
}

class SurfaceConfig : public QDialog
{
    Q_OBJECT

public:
    explicit SurfaceConfig(QWidget *parent = nullptr);
    ~SurfaceConfig();

private:
    Ui::SurfaceConfig *ui;
};

#endif // SURFACECONFIG_H
