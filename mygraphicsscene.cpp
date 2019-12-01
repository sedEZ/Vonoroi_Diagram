#include "mygraphicsscene.h"
#include <cmath>
#include <QDebug>
#include <QPen>

using namespace std;


struct point{
    QString s;
};
bool compare_point_lexical(const struct point a, const struct point b){
    return lexicographical_compare(a.s.begin(),a.s.end(),b.s.begin(),b.s.end());
}

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
        //qDebug()<<"line_p is not empty";
        for(unsigned long i=0;i<line_p.size();i++)
            this->removeItem(line_p[i]);
        line_p.clear();
    }
    if(!hp.empty()){
        //qDebug()<<"line_p is not empty";
        for(unsigned long i=0;i<hp.size();i++)
            this->removeItem(hp[i]);
        hp.clear();
    }
    if(!Hull_Sl.empty()){
        //qDebug()<<"line_p is not empty";
        for(unsigned long i=0;i<Hull_Sl.size();i++)
            this->removeItem(Hull_Sl[i]);
        Hull_Sl.clear();
    }
    if(!Hull_Sr.empty()){
        //qDebug()<<"line_p is not empty";
        for(unsigned long i=0;i<Hull_Sr.size();i++)
            this->removeItem(Hull_Sr[i]);
        Hull_Sr.clear();
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

    vector<int> e = current_vonoroi.getOrdinaryEdges();
    vector<struct bisector> HP = current_vonoroi.getHP();
    vector<int> hull_Sl = current_vonoroi.getHULL_Sl();
    vector<int> hull_Sr = current_vonoroi.getHULL_Sr();
    vector<double> g_x = current_vonoroi.get_g_x();
    vector<double> g_y = current_vonoroi.get_g_y();

    for(int i=0;i<e.size();i++){
        double x_1,x_2,y_1,y_2;
        current_vonoroi.getOridinaryEdgesCoordinates(e[i], x_1, x_2, y_1, y_2);
        QGraphicsLineItem* l = new QGraphicsLineItem(x_1, y_1, x_2, y_2);
        l->setPen(QPen(Qt::red));
        this->addItem(l);
        this->line_p.append(l);
    }

    for(unsigned long i=0;i<HP.size();i++){
        QGraphicsLineItem* l = new QGraphicsLineItem(HP[i].x1, HP[i].y1, HP[i].x2, HP[i].y2);
        qDebug()<<"HP[i].x1 = "<<HP[i].x1<<"; HP[i].y1 = "<<HP[i].y1;
        qDebug()<<"HP[i].x2 = "<<HP[i].x2<<"; HP[i].y2 = "<<HP[i].y2;
        l->setPen(QPen(Qt::black));
        this->addItem(l);
        this->hp.append(l);
    }

    if(hull_Sl.size() >1)
        for(unsigned long i=0;i<hull_Sl.size();i++){
            QGraphicsLineItem* l = new QGraphicsLineItem(g_x[hull_Sl[i]], g_y[hull_Sl[i]], g_x[hull_Sl[(i+1)%hull_Sl.size()]], g_y[hull_Sl[(i+1)%hull_Sl.size()]]);
            l->setPen(QPen(Qt::blue));
            this->addItem(l);
            this->Hull_Sl.append(l);
        }
    if(hull_Sr.size() >1)
        for(unsigned long i=0;i<hull_Sr.size();i++){
            QGraphicsLineItem* l = new QGraphicsLineItem(g_x[hull_Sr[i]], g_y[hull_Sr[i]], g_x[hull_Sr[(i+1)%hull_Sr.size()]], g_y[hull_Sr[(i+1)%hull_Sr.size()]]);
            l->setPen(QPen(Qt::blue));
            this->addItem(l);
            this->Hull_Sr.append(l);
        }
}

void MyGraphicsScene::writeOutputTxt(QString dir)
{
    WingedEdge result = this->v->getResult();

    vector<double> g_x = result.get_g_x();
    vector<double> g_y = result.get_g_y();

    vector<struct point> P, E;
    for(unsigned long i=0;i<g_x.size();i++){
        struct point p;
        p.s.append(QString::number(int(round(g_x[i]))));
        p.s.append(" ");
        p.s.append(QString::number(int(round(g_y[i]))));
        P.push_back(p);
    }
    sort(P.begin(),P.end(),compare_point_lexical);

    vector<int> e = result.getOrdinaryEdges();
    for(unsigned long i=0;i<e.size();i++){
        double x_1,x_2,y_1,y_2;
        result.getOridinaryEdgesCoordinates(e[i], x_1, x_2, y_1, y_2);
        int x1 = int(round(x_1));
        int y1 = int(round(y_1));
        int x2 = int(round(x_2));
        int y2 = int(round(y_2));

        if(!(x1<x2 || (x1==x2 and y1<y2))){
            int tmp_x = x1,tmp_y = y1;
            x1 = x2;       y1 = y2;
            x2 = tmp_x;    y2 = tmp_y;
        }

        struct point e;
        e.s.append(QString::number(x1));
        e.s.append(" ");
        e.s.append(QString::number(y1));
        e.s.append(" ");
        e.s.append(QString::number(x2));
        e.s.append(" ");
        e.s.append(QString::number(y2));

        E.push_back(e);
    }
    sort(E.begin(),E.end(),compare_point_lexical);

    QFile output_file(dir);

    if(!output_file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"output_file open fail!";
        return;
    }
    QTextStream out_stream(&output_file);

    for(unsigned long i=0;i<P.size();i++){
        out_stream << "P " << P[i].s <<endl;
    }
    for(unsigned long i=0;i<E.size();i++){
        out_stream << "E " << E[i].s <<endl;
    }

    qDebug()<<"Write result success!";

    output_file.close();
}

//Click on graphic scene
void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        QPointF* p = new QPointF(e->scenePos());
        addAGeneratingPoints(p,5.0,5.0);

    }
}
