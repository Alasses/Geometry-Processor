#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector3D>
#include <QFileDialog>
#include <QMessageBox>
#include <QMessageBox>
#include <QColorDialog>
#include <QDebug>

#include <mywidget.h>
#include <surfaceconfig.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //void on_actionOpen_Mesh_triggered();

    void on_actionOpen_obj_file_triggered();

    void on_actionOpen_ply_file_triggered();

    void on_actionGenerate_a_Surface_triggered();

    void on_DeleteButton_clicked();

    void on_LineBox_toggled(bool checked);

    void on_BoundBox_toggled(bool checked);

    void on_MeshBox_currentIndexChanged(int index);

    void on_HideBox_toggled(bool checked);

    void on_ColorButton_clicked();

    void on_ResetButton_clicked();

    void on_IrregularDivide_clicked();

    void on_RegularDivide_clicked();

    void on_UniformSmooth_clicked();

    void on_CordSmooth_clicked();

    void on_MeanCurvSmooth_clicked();

    void on_MeanValSmooth_clicked();

    void on_CheckBoardBox_toggled(bool checked);

    void on_ExplicitDiffusion_clicked();

    void on_ExplicitDiffusion_3_clicked();

    void on_ExplicitDiffusion_2_clicked();

    void on_PrincipalCurvature_clicked();

    void on_PrincipalSmooth_clicked();

    void on_CalcStreamLine_clicked();

    void on_GaussianCurvature_clicked();

    void on_SphericalParameterize_clicked();

private:
    Ui::MainWindow *ui;
    myWidget *glwidget;
    SurfaceConfig surfaceCon;

    int GetType();

    bool CheckPolyType(int type);

    void SubdivideSubFunc(int mode);

    void SmoothSubFunc(int mode);

    void HeatDiffusionSubFunc(int mode);
};
#endif // MAINWINDOW_H
