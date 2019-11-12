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

void WingedEdge::constructTOnePointsVoronoi()
{
    num_vertices = 0;
    num_edges = 0;
    w[0] = 0; //infinity
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

    if(y_1 == y_2){
        //x = b
        double b = (x_1+x_2)/2;

        //(x_1,y_1) = (b,0)
        x.push_back(b);
        y.push_back(0.0);

        //(x_1,y_1) = (b,600)

        x.push_back(b);
        y.push_back(600.0);
    }
    else if(x_1 == x_2){
        //y = c
        double c = (y_1+y_2)/2;

        //(x_1,y_1) = (0,c)
        x.push_back(0.0);
        y.push_back(c);

        //(x_1,y_1) = (600,c)

        x.push_back(600.0);
        y.push_back(c);
    }
    else{
        //y = mx + b
        double m = (-1)*(x_1-x_2)/(y_1-y_2);
        double b = ((pow(x_1,2)+pow(y_1,2)) - (pow(x_2,2)+pow(y_2,2))) / (y_1-y_2);

        //x = ny + c
        double n = (-1)*(y_1-y_2)/(x_1-x_2);
        double c = ((pow(x_1,2)+pow(y_1,2)) - (pow(x_2,2)+pow(y_2,2))) / (x_1-x_2);

        //Find the 2 vertices cross with the edge of scene
        //Origin (0,0) is on left-top side
        double y_0,y_600,x_0,x_600;

        //x=0
        y_0 = m*0+b;
        //x=600.0
        y_600 = m*600.0+b;
        //y=0
        x_0 =n*0 + c;
        //y=600.0
        x_600 = n*600.0 + c;

        //Put 2 vertices in
        if(y_0 >=0 && y_0 <=600){
            x.push_back(0);
            y.push_back(y_0);
        }
        if(y_600 >=0 && y_600 <=600){
            x.push_back(600.0);
            y.push_back(y_600);
        }
        if(x_0 >=0 && x_0 <=600){
            x.push_back(x_0);
            y.push_back(0);
        }if(x_600 >=0 && x_600 <=600){
            x.push_back(x_600);
            y.push_back(600.0);
        }
    }

    //Todo : 已新增兩點，再來要更改WingedEdge的內容

}

void WingedEdge::divide(WingedEdge *W_l, WingedEdge *W_r)
{
    //Divide part
    vector<double> l_x,l_y,r_x,r_y;
    double m = this->findMedianLine();//Three points version
    for(unsigned long i=0;i<this->g_x.size();i++){
        //num_polygons should be same as g_x.size() and g_y.size()

        //Less than m, put in left
        if(this->g_x[i] < m){
            l_x.push_back(g_x[i]);
            l_y.push_back(g_y[i]);
        }
        else if(this->g_x[i] >= m){
            r_x.push_back(g_x[i]);
            r_y.push_back(g_y[i]);
        }
    }
    W_l = new WingedEdge(l_x,l_y);
    W_r = new WingedEdge(r_x,r_y);
}

void WingedEdge::merge(WingedEdge S_l, WingedEdge S_r)
{

}

double WingedEdge::findMedianLine()
{
    //Three points version
    if((this->g_x[2] >= this->g_x[1] && this->g_x[1] >= this->g_x[0]) || (this->g_x[0] >= this->g_x[1] && this->g_x[1] >= this->g_x[2]))
        //g_x[1] is median
        return g_x[1];
    else if((this->g_x[2] >= this->g_x[0] && this->g_x[0] >= this->g_x[1]) || (this->g_x[1] >= this->g_x[0] && this->g_x[0] >= this->g_x[2]))
        //g_x[0] is median
        return g_x[0];
    else if((this->g_x[0] >= this->g_x[2] && this->g_x[2] >= this->g_x[1]) || (this->g_x[1] >= this->g_x[2] && this->g_x[2] >= this->g_x[0]))
        //g_x[2] is median
        return g_x[2];
}

int WingedEdge::getNumPolygons()
{
    return num_polygons;
}

bool WingedEdge::threePointsVertical()
{
    return num_polygons==3 && g_x[0]==g_x[1] && g_x[1]==g_x[2];
}

void WingedEdge::setWaitingMerge(bool i)
{
    this->waiting_merge = i;
}

bool WingedEdge::IsWaitingMerge()
{
    return this->waiting_merge;
}
