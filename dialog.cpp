#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    scene = new MyGraphicsScene();
    scene->setSceneRect(0,0,600,600);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

Dialog::~Dialog()
{
    delete ui;
}

//Run
void Dialog::on_pushButton_clicked()
{
    //Run_vonoroi_to_the_end();

    if (!this->scene->initialized()){
       this->scene->initializeVonoroi();
    }

    //this->scene->v->run_to_end();
}

//Step by step
void Dialog::on_pushButton_2_clicked()
{
    if (!this->scene->initialized()){
       this->scene->initializeVonoroi();
    }
    //Run_next_step();
    //this->scene->runOneStep();
}

//Clear
void Dialog::on_pushButton_3_clicked()
{
    this->scene->restart();
}
