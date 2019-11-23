#include "mygraphicsscene.h"
#include <QDebug>
#include <QPen>

using namespace std;

//Constructor
MyGraphicsScene::MyGraphicsScene(){
    QGraphicsScene();
}

void MyGraphicsScene::restart()
{
    //Clear all generating points
    this->clearLine_p();
    this->clear();
    this->setInitializedIndicator(0);
    this->g_points.clear();

}

//Use mouse to add genetating points of vonoroi diagram
void MyGraphicsScene::addAGeneratingPoints(QPointF *pos, qreal width, qreal height){
    QGraphicsEllipseItem* dot = new QGraphicsEllipseItem(pos->x(),pos->y(),width,height);
    dot->setBrush(QBrush(Qt::red));
    this->addItem(dot);
    this->g_points.append(*pos);
}

void MyGraphicsScene::setInitializedIndicator(int i)
{
    initialized_indicator = i;
}

bool MyGraphicsScene::voronoiEmpty()
{
    return v->empty();
}

void MyGraphicsScene::clearLine_p()
{
    if(!line_p.empty()){
        qDebug()<<"line_p is not empty";
        for(unsigned long i=0;i<line_p.size();i++)
            this->removeItem(line_p[i]);
        line_p.clear();
    }
}

bool MyGraphicsScene::initializeVonoroi()
{
    //Create the vectors of generating points' x,y coordinates
    //in order to create vonoroi
    //Success: return 1 ; Fail: return 0
    vector<double> x,y;
    for(int i=0;i<this->g_points.size();i++){
        x.push_back(this->g_points[i].x());
        y.push_back(this->g_points[i].y());
    }
    v = new Voronoi(x,y);
    this->setInitializedIndicator(1);

    return !x.empty();
}

/*
//Todo
void MyGraphicsScene::runOneStep()
{
    WingedEdge current_vonoroi = this->v->runOneStep();
    //Update scene with current_vonoroi
    //Check if reaching result
}
*/

//Temporary version: for first time demo
void MyGraphicsScene::runOneStep()
{
    if(this->v->empty())
        return;
    else if(this->v->stack_leak()){
        qDebug()<<"Stack size out of range.Exit";
        exit(-1);
    }


    WingedEdge current_vonoroi = this->v->runOneStep();
    this->clearLine_p();

    //Update scene with current_vonoroi
    //Check if reaching result

    vector<int> e;
    current_vonoroi.getOrdinaryEdges(e);

    for(int i=0;i<e.size();i++){
        double x_1,x_2,y_1,y_2;
        current_vonoroi.getOridinaryEdgesCoordinates(e[i], x_1, x_2, y_1, y_2);
        QGraphicsLineItem* l = new QGraphicsLineItem(x_1, y_1, x_2, y_2);
        l->setPen(QPen(Qt::red));
        this->addItem(l);
        this->line_p.append(l);
    }

}

//Click on graphic scene
void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        QPointF* p = new QPointF(e->scenePos());
        addAGeneratingPoints(p,10.0,10.0);

    }
}
