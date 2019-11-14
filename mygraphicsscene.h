#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include "wingededge.h"
#include "voronoi.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPointF>
#include <QVector>
#include <stack>
#include <vector>


class MyGraphicsScene : public QGraphicsScene{
    Q_OBJECT
public:
    MyGraphicsScene();

    void initializeVonoroi();

    //**Todo
    void runOneStep();

    //Restart vonoroi's diagram
    void restart();
    void addAGeneratingPoints(QPointF* pos, qreal width, qreal height );
    bool initialized(){return initialized_indicator;}
    void setInitializedIndicator(int i);
    bool voronoiEmpty();
    void clearLine_p();


signals:

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
private:
    //Class for running vonoroi's diagram step-by-step using divide-and-conquer strategy
    Voronoi* v;

    //generating points of Vonoroi's diagram
    QVector<QPointF> g_points;

    //Storing pointers to QGraphicsLineItem
    QVector<QGraphicsLineItem*> line_p;

    //If initialized_indicator == 0, initilize the vonoroi's diagram,
    //then adjust initialized_indicator to 1;
    int initialized_indicator = 0;
};

#endif // MYGRAPHICSSCENE_H
