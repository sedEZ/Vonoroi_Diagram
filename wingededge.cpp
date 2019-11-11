#include "wingededge.h"
#include <QDebug>

WingedEdge::WingedEdge(){

}

WingedEdge::WingedEdge(vector<double> p_x, vector<double> p_y){
    int i=0;
    for(;i<p_x.size();i++){
        g_x.push_back(p_x[i]);
        g_y.push_back(p_y[i]);
    }
    num_polygons = i;
}

void WingedEdge::constructTwoPointsVoronoi()
{
    //Assert if # of generating points is 2
    if(num_polygons != 2){
        qDebug()<<"Number of generating points is not 2. To use this function, only 2 points is available.";
        return;
    }

    double x_1 = g_x[0],x_2 = g_x[1];
    double y_1 = g_y[0],y_2 = g_y[1];

    //y = mx + b
    double m = (-1)*(x_1-x_2)/(y_1-y_2);
    double b = ((pow(x_1,2)+pow(y_1,2)) - (pow(x_2,2)+pow(y_2,2))) / (y_1-y_2);

    //Find the 2 vertices cross with the edge of scene

}

int WingedEdge::getNumPolygons()
{
    return num_polygons;
}

void WingedEdge::setWaitingMerge(bool i)
{
    this->waiting_merge = i;
}

bool WingedEdge::IsWaitingMerge()
{
    return this->waiting_merge;
}
