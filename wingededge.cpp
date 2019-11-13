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
    num_polygons = i+1;//i+1 is the infinity polygon
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
    if(this->getNumPolygons() != 2){
        qDebug()<<"Number of generating points is not 2. To use this function, only 2 points is available.";
        return;
    }

    double x_1 = this->g_x[0],x_2 = this->g_x[1];
    double y_1 = this->g_y[0],y_2 = this->g_y[1];

    this->w.resize(2);
    this->w[0] = 1;
    this->w[1] = 1;
    this->num_vertices = 3;
    this->num_edges=3;


    this->changeArraysForEdges(this->num_edges);

    if(y_1 == y_2){ //Vertical median line
        //x = b
        double b = (x_1+x_2)/2;

        //(x[0],y[0]) = (b,0)
        x.push_back(b);
        y.push_back(0.0);

        //(x[1],y[1]) = (b,600)
        x.push_back(b);
        y.push_back(600.0);

        if(x_2 < x_1){ //(g_x[0],g_y[0]) on left, (g_x[1],g_y[1])  on right
            iter_swap(g_x.begin()+0,g_x.begin()+1);
            iter_swap(g_y.begin()+0,g_y.begin()+1);
        }
    }
    else if(x_1 == x_2){//Horizontal median line
        //y = c
        double c = (y_1+y_2)/2;

        //(x_1,y_1) = (0,c)
        x.push_back(0.0);
        y.push_back(c);

        //(x_1,y_1) = (600,c)

        x.push_back(600.0);
        y.push_back(c);

        if(y_2 > y_1){ //(g_x[0],g_y[0]) at the top, (g_x[1],g_y[1]) at the bot
            iter_swap(g_x.begin()+0,g_x.begin()+1);
            iter_swap(g_y.begin()+0,g_y.begin()+1);
        }
    }
    else{

        qDebug()<<"x_1 = "<<x_1;
        qDebug()<<"y_1 = "<<y_1;
        qDebug()<<"x_2 = "<<x_2;
        qDebug()<<"y_2 = "<<y_2;

        //y = mx + b
        double m = (x_2-x_1)/(y_1-y_2);
        double b = (x_1*x_1+y_1*y_1-x_2*x_2-y_2*y_2)/(2*(y_1-y_2));

        //x = ny + c
        double n = (y_1-y_2)/(x_2-x_1);
        double c = b*(y_2-y_1)/(x_2-x_1);

        qDebug()<<"m = "<<m;
        qDebug()<<"b = "<<b;
        qDebug()<<"n = "<<n;
        qDebug()<<"c = "<<c;

        //Find the 2 vertices cross with the edge of scene
        double y_0,y_600,x_0,x_600;

        //x=0
        y_0 = m*0+b;
        //x=600.0
        y_600 = m*600.0+b;
        //y=0
        x_0 =n*0 + c;
        //y=600.0
        x_600 = n*600.0 + c;

        //Put 2 vertices into vector
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
        }

        if(x_600 >=0 && x_600 <=600){
            x.push_back(x_600);
            y.push_back(600.0);
        }

        qDebug()<<"x[0] = "<<x[0];
        qDebug()<<"y[0] = "<<y[0];
        qDebug()<<"x[1] = "<<x[1];
        qDebug()<<"y[1] = "<<y[1];


        //(g_x[0],g_y[0]) should be at left side of edge[0], (g_x[1],g_y[1]) should be at right side of edge[0].
        //Use cross product to judge if (g_x[0],g_y[0]) is on the left of edge[0]
        //a = ((x[1]-x[0]), (y[1]-y[0]))
        //b = ((x[1]-g_x[0]), (y[1]-g_y[0]))
        //a x b > 0 => b is on the left side of a
        if((x[1]-x[0])*(x[1]-g_x[0]) - (y[1]-y[0])*(y[1]-g_y[0]) < 0){
            iter_swap(g_x.begin()+0,g_x.begin()+1);
            iter_swap(g_y.begin()+0,g_y.begin()+1);
        }
    }

    //Set edge[0], edge[0] is the ordinary edge of 2 points from vertex[0]->vertex[1]
    this->right_polygon[0]=1;
    this->left_polygon[0]=0;
    this->start_vertex[0]=0;
    this->end_vertex[0]=1;
    this->cw_predecessor[0]=2;
    this->ccw_predecessor[0]=1;
    this->cw_successor[0]=1;
    this->ccw_successor[0]=2;

    //Set edge[1], edge[1] is on the outer side of generating_point[0] from edge[0]
    this->right_polygon[1]=0;
    this->left_polygon[1]=2; //polygon at inf
    this->start_vertex[1]=0;
    this->end_vertex[1]=1;
    this->cw_predecessor[1]=0;
    this->ccw_predecessor[1]=2;
    this->cw_successor[1]=2;
    this->ccw_successor[1]=0;

    //Set edge[2], edge[2] is on the outer side of generating_point[1] from edge[0]
    this->right_polygon[2]=2;//polygon at inf
    this->left_polygon[2]=0;
    this->start_vertex[2]=0;
    this->end_vertex[2]=1;
    this->cw_predecessor[2]=1;
    this->ccw_predecessor[2]=0;
    this->cw_successor[2]=0;
    this->ccw_successor[2]=1;
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
    //Brute-force for three points version
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
    return num_polygons-1;//Except for p_infinity
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

void WingedEdge::changeArraysForEdges(int resize_size)
{
    this->right_polygon.resize(resize_size);
    this->left_polygon.resize(resize_size);
    this->start_vertex.resize(resize_size);
    this->end_vertex.resize(resize_size);
    this->cw_predecessor.resize(resize_size);
    this->ccw_predecessor.resize(resize_size);
    this->cw_successor.resize(resize_size);
    this->ccw_successor.resize(resize_size);
}

void WingedEdge::getOrdinaryEdges(vector<int>& e)
{
    //Get oridinary edges. The augumented edges are adjacent to p_infinity
    for(unsigned long i=0;i<num_edges;i++){
        if(right_polygon[i] == this->getNumPolygons()+1 || left_polygon[i] == this->getNumPolygons()+1)
            continue;
        //put oridinary edge number into e
        e.push_back(i);
    }

}

void WingedEdge::getOridinaryEdgesCoordinates(int i, double &x_1, double &x_2, double &y_1, double &y_2)
{
    x_1 = x[start_vertex[i]];
    x_2 = x[end_vertex[i]];
    y_1 = y[start_vertex[i]];
    y_2 = y[end_vertex[i]];

}
