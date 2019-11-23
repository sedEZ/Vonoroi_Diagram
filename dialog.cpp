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
    this->tx_finish = false;
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
            qDebug()<<"Got zero!";
            this->tx->flush();
            this->file->close();
            this->from_tx = false;
            this->tx_finish = true;
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

    }while(!this->tx->atEnd());

    qDebug()<<"Read 1 voronoi";
}

//Run
void Dialog::on_pushButton_clicked()
{
    //Run_vonoroi_to_the_end();

    if(this->from_tx){
        this->scene->restart();
        this->set_a_voronoi_from_tx();
    }

    if(this->tx_finish){
        this->file->close();
        this->scene->restart();
        this->tx_finish = false;
        return;
    }

    if (!this->scene->initialized()){
       if(!this->scene->initializeVonoroi()){
           this->scene->restart();
           return;
       }
    }
    while(!this->scene->voronoiEmpty()){
        this->scene->runOneStep();
    }
    this->scene->writeOutputTxt("./result.txt");
    qDebug()<<"Finish running all steps";
}

//Step by step
void Dialog::on_pushButton_2_clicked()
{
    if(this->from_tx){
        this->scene->restart();
        this->set_a_voronoi_from_tx();
    }

    if(this->tx_finish){
        this->file->close();
        this->scene->restart();
        this->tx_finish = false;
        return;
    }

    if (!this->scene->initialized()){
       if(!this->scene->initializeVonoroi()){
           this->scene->restart();
           return;
       }
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

void Dialog::on_pushButton_4_clicked()
{
    /***************/
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/",
                                                    tr("Txt (*.txt)"));
    if(fileName.isEmpty() || fileName.isNull()){
        return;
    }
    if (!fileName.endsWith(".txt"))
        fileName = fileName.append(".txt");
    this->file = new QFile(fileName);
    this->file->open(QIODevice::ReadOnly);
    tx = new QTextStream(file);
    from_tx = true;
    qDebug()<<"Read from "<<fileName<<" success!";
}

void Dialog::on_pushButton_5_clicked()
{
    this->scene->restart();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/",
                                                    tr("Txt (*.txt)"));
    if(fileName.isEmpty() || fileName.isNull()){
        return;
    }
    if (!fileName.endsWith(".txt"))
        fileName = fileName.append(".txt");
    this->file = new QFile(fileName);
    this->file->open(QIODevice::ReadOnly);
    this->tx = new QTextStream(file);

    QString str;
    do{
        str = this->tx->readLine();
        if(str[0] == 'P'){
            QStringList list1 = str.split(' ');
            double x = list1[1].toDouble();
            double y = list1[2].toDouble();

            QPointF *p = new QPointF(x,y);
            this->scene->addAGeneratingPoints(p,10,10);
            delete p;
        }
        else if(str[0] == 'E'){
            QStringList list2 = str.split(' ');
            double x1 = list2[1].toDouble();
            double y1 = list2[2].toDouble();
            double x2 = list2[3].toDouble();
            double y2 = list2[4].toDouble();
            QGraphicsLineItem* l = new QGraphicsLineItem(x1, y1, x2, y2);
            l->setPen(QPen(Qt::red));
            this->scene->addItem(l);
        }

    }while(!this->tx->atEnd());


    this->file->close();
    this->tx_finish = false;
    qDebug()<<"Read from "<<fileName<<" success!";
}
