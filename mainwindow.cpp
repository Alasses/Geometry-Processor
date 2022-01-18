#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Retrive the access to the opengl widget
    glwidget = ui->glWidget;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_obj_file_triggered()
{
    //Get the file name
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    if (!fileName.isEmpty()) {
        //Check if the file is accessible
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);

        //Check the type of target object
        if(fileName.contains(".obj"))
        {
            //Update the opengl scene
            int id = glwidget->UpdateMeshFilePath(fileName, 1);

            //Generate the QVariant
            QVariant ID(id);

            //Insert the new object
            ui->MeshBox->addItem("Object" + QString::number(id), ID);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Please select a .obj file.");
            msgBox.exec();
        }

        file.close();
    }
}

void MainWindow::on_actionOpen_ply_file_triggered()
{
    //Get the file name
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    if (!fileName.isEmpty()) {
        //Check if the file is accessible
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
          QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
          return;
        }
        QTextStream in(&file);

        //Check the type of target object
        if(fileName.contains(".ply"))
        {
            //Update the opengl scene
            int id = glwidget->UpdateMeshFilePath(fileName, 2);

            //Generate the QVariant
            QVariant ID(id);

            //Insert the new object
            ui->MeshBox->addItem("Poly" + QString::number(id), ID);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Please select a .ply file.");
            msgBox.exec();
        }

        file.close();
    }
}

void MainWindow::on_actionGenerate_a_Surface_triggered()
{
    //Show up the surface configuration window, if there is a better way to do this?
    surfaceCon.show();
}

int MainWindow::GetType()
{
    QString name = ui->MeshBox->currentText();

    //Classify the target object
    if(name.contains("Surface"))
        return 1;
    else if(name.contains("Object"))
        return 2;
    else if(name.contains("Poly"))
        return 3;
    else
        return -1;
}

bool MainWindow::CheckPolyType(int type)
{
    if(type != 3)
    {
        QMessageBox msgBox;
        msgBox.setText("Current object is not a ply object!");
        msgBox.exec();

        return false;
    }

    return true;
}

void MainWindow::on_DeleteButton_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Classify the target object
    bool result = false;
        result = glwidget->DeleteObject(GetType(), id);

    if(result)
        ui->MeshBox->removeItem(ui->MeshBox->currentIndex());
    else
        qInfo("Error when deleting the object with id: %d", id);
}

void MainWindow::on_LineBox_toggled(bool checked)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    glwidget->DisplayMode(GetType(), id, checked);
}

void MainWindow::on_BoundBox_toggled(bool checked)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    glwidget->BoundBoxMode(GetType(), id, checked);
}

void MainWindow::on_MeshBox_currentIndexChanged(int index)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Classify the target object
    vec3 currentColor = glwidget->GetColor(GetType(), id);

    //Update the check boxs
    ui->HideBox->setChecked(glwidget->GetRenderMode(GetType(), id));
    ui->LineBox->setChecked(glwidget->GetDisplayMode(GetType(), id));
    ui->BoundBox->setChecked(glwidget->GetBoundMode(GetType(), id));

    //Update the current object
    glwidget->SetCurrentObject(GetType(), id);

    //Update the check board, only for ply
    if(GetType() == 3)
        ui->CheckBoardBox->setChecked(glwidget->GetCheckBoardMode(id));
}

void MainWindow::on_HideBox_toggled(bool checked)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    glwidget->RenderMode(GetType(), id, checked);
}

void MainWindow::on_ColorButton_clicked()
{
    QColor rcolor = QColorDialog::getColor();

    vec3 color = vec3(rcolor.rgba64().red8() / 255.0f,
                      rcolor.rgba64().green8() / 255.0f,
                      rcolor.rgba64().blue8() / 255.0f);

    //Change color
    glwidget->ChangeColor(GetType(), ui->MeshBox->currentData().toInt(), color);
}

void MainWindow::on_ResetButton_clicked()
{
    //Reset rotation and position
    glwidget->ResetPosition(GetType(), ui->MeshBox->currentData().toInt());
}

void MainWindow::SubdivideSubFunc(int mode)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Only apply for ply object
    if(CheckPolyType(GetType()))
        glwidget->SubdividePly(id, mode);
}

void MainWindow::on_IrregularDivide_clicked()
{
    //Regular 1
    SubdivideSubFunc(2);
}

void MainWindow::on_RegularDivide_clicked()
{
    //Irregular 2
    SubdivideSubFunc(1);
}

void MainWindow::SmoothSubFunc(int mode)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Only apply for ply object
    if(CheckPolyType(GetType()))
        glwidget->SmoothPly(id, mode);
}

void MainWindow::on_UniformSmooth_clicked()
{
    //Uniform 1
    SmoothSubFunc(1);
}

void MainWindow::on_CordSmooth_clicked()
{
    //Cord 2
    SmoothSubFunc(2);
}

void MainWindow::on_MeanCurvSmooth_clicked()
{
    //Mean Curv 3
    SmoothSubFunc(3);
}

void MainWindow::on_MeanValSmooth_clicked()
{
    //Mean Val 4
    SmoothSubFunc(4);
}

void MainWindow::on_CheckBoardBox_toggled(bool checked)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Get the target type
    int type = GetType();

    //Only apply for ply object
    if(CheckPolyType(GetType()))
        glwidget->CheckBoardMode(id, checked);
}

void MainWindow::HeatDiffusionSubFunc(int mode)
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();

    //Get the target type
    int type = GetType();

    //Only apply for ply object
    if(CheckPolyType(GetType()))
        glwidget->HeatDiffusionPly(id, mode);
}

void MainWindow::on_ExplicitDiffusion_clicked()
{
    //The explicit update method
    HeatDiffusionSubFunc(1);
}

void MainWindow::on_ExplicitDiffusion_3_clicked()
{
    //The gaussian update method
    HeatDiffusionSubFunc(3);
}

void MainWindow::on_ExplicitDiffusion_2_clicked()
{
    //The implicit method
    HeatDiffusionSubFunc(2);
}

void MainWindow::on_PrincipalCurvature_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();
    glwidget->CurvaturePly(id, 3);
}

void MainWindow::on_PrincipalSmooth_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();
    glwidget->CurvaturePly(id, 4);
}

void MainWindow::on_CalcStreamLine_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();
    glwidget->CurvaturePly(id, 5);
}

void MainWindow::on_GaussianCurvature_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();
    glwidget->CurvaturePly(id, 2);
}

void MainWindow::on_SphericalParameterize_clicked()
{
    //Get the target object
    int id = ui->MeshBox->currentData().toInt();
    glwidget->SphericalParaPly(id);
}
