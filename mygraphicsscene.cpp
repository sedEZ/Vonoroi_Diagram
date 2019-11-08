#include "mygraphicsscene.h"

using namespace std;

//Constructor
MyGraphicsScene::MyGraphicsScene(){
    QGraphicsScene();
}

void MyGraphicsScene::restart()
{
    //Clear generating points
    this->clear();
    this->setInitializedIndicator(0);
    delete this->v;
}

//Use mouse to add genetating points of vonoroi diagram
void MyGraphicsScene::addAGeneratingPoints(QPointF *pos, qreal width, qreal height){
    QGraphicsEllipseItem* dot = new QGraphicsEllipseItem(pos->x(),pos->y(),width,height);
    dot->setBrush(QBrush(Qt::red));
    this->addItem(dot);
    g_points.append(*pos);
}

void MyGraphicsScene::setInitializedIndicator(int i)
{
    initialized_indicator = i;
}

void MyGraphicsScene::initializeVonoroi()
{
    //Create the vectors of generating points' x,y coordinates
    //in order to create vonoroi
    vector<double> x,y;
    for(int i=0;i<this->g_points.size();i++){
        x.push_back(this->g_points[i].x());
        y.push_back(this->g_points[i].y());
    }
    v = new Vonoroi(x,y);

}

//Todo
void MyGraphicsScene::runOneStep()
{
    WingedEdge current_vonoroi = this->v->runOneStep();
    //Update scene with current_vonoroi
    //Check if reaching result
}

//Click on graphic scene
void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        QPointF* p = new QPointF(e->scenePos());
        addAGeneratingPoints(p,10.0,10.0);
    }
}
