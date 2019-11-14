#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>

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

    while(!this->scene->voronoiEmpty()){
        qDebug()<<"Run next step";
        this->scene->runOneStep();
        qDebug()<<"Finish running the step";
    }
}

//Step by step
void Dialog::on_pushButton_2_clicked()
{
    if (!this->scene->initialized()){
       this->scene->initializeVonoroi();
    }
    //Run_next_step();
    qDebug()<<"Run next step";
    if(this->scene->voronoiEmpty()){
        return;
    }

    this->scene->runOneStep();
}

//Clear
void Dialog::on_pushButton_3_clicked()
{
    this->scene->restart();
}


void Dialog::on_lineEdit_editingFinished()
{
    QString savedText = ui->lineEdit->text();
    QString fileName(savedText);
    if (!fileName.endsWith(".txt"))
        fileName = fileName.append(".txt");
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream tx(&file);
    QString str;
    str = tx.readLine();
    qDebug()<<str;
    file.close();

    while(!tx.atEnd()){
        str = tx.readLine();
        if(str[0] == '#' || str[0] == ' ' || str.size()==0)
            continue;
        else if(str[0] == '0'){
            tx.flush();
            break;
        }
        qDebug()<<str;
    }
}
