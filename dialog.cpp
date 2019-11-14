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
    this->from_tx = false;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::set_a_voronoi_from_tx()
{
    QString str;
    do{
        str = this->tx->readLine();

        if(str[0] == '#' || str[0] == ' ' || str.size()==0)
            continue;
        else if(str[0] == '0'){
            this->tx->flush();
            this->file->close();
            break;
        }

        else{
            bool ok;
            int num_p = str.toInt(&ok,10);
            if(!ok){
                qDebug()<<"set_a_voronoi_from_tx():Str to Int error";
                exit(-1);
            }
            for(int i=0;i<num_p;i++){
                double x, y;
                *(this->tx) >> x >> y;
                QPointF* p = new QPointF(x,y);
                this->scene->addAGeneratingPoints(p,10.0,10.0);
            }
            break;
        }
        qDebug()<<str;
    }while(!this->tx->atEnd());
    qDebug()<<"Read 1 voronoi";
}

//Run
void Dialog::on_pushButton_clicked()
{
    //Run_vonoroi_to_the_end();

    if(from_tx){
        this->scene->restart();
        this->set_a_voronoi_from_tx();
    }

    if (!this->scene->initialized()){
       this->scene->initializeVonoroi();
    }
    while(!this->scene->voronoiEmpty()){

        this->scene->runOneStep();
    }

    qDebug()<<"Finish running all steps";
}

//Step by step
void Dialog::on_pushButton_2_clicked()
{
    if (!this->scene->initialized()){
       if(from_tx){
           this->scene->restart();
           this->set_a_voronoi_from_tx();
       }
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
    //read from file
    QString savedText = ui->lineEdit->text();
    QString fileName(savedText);
    if (!fileName.endsWith(".txt"))
        fileName = fileName.append(".txt");
    this->file = new QFile(fileName);
    file->open(QIODevice::ReadOnly);
    tx = new QTextStream(file);
    from_tx = true;
    qDebug()<<"Read from "<<fileName<<" success!";
}
