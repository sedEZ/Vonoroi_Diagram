#include "wingededge.h"
#include <QDebug>

#define INF 65536

WingedEdge::WingedEdge(){

}

WingedEdge::WingedEdge(vector<double> p_x, vector<double> p_y){
    unsigned long i=0;
    for(;i<p_x.size();i++){
        g_x.push_back(p_x[i]);
        g_y.push_back(p_y[i]);
    }
    num_polygons = i+1;//i+1 is the infinity polygon
    waiting_merge=false;
}

void WingedEdge::constructOnePointVoronoi()
{
    num_vertices = 0;
    num_edges = 0;
    w.push_back(0); //infinity
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
    this->w[0] = 0;
    this->w[1] = 0;
    this->num_vertices = 3;
    this->num_edges=3;

    //Initializing all edges' arrays in wingededge data structure
    this->changeArraysForEdges(this->num_edges);

    if(fabs(y_1 - y_2)< 1e-8){//Vertical median line
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
    else if(fabs(x_1 - x_2)< 1e-8){//Horizontal median line
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
        //y = mx + b
        double m,b;
        findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);

        //x = ny + c
        double n = (y_1-y_2)/(x_2-x_1);
        double c = b*(y_2-y_1)/(x_2-x_1);

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

        //(g_x[0],g_y[0]) should be at left side of edge[0], (g_x[1],g_y[1]) should be at right side of edge[0].
        //Use cross product to judge if (g_x[0],g_y[0]) is on the left of edge[0]
        //a = ((x[1]-x[0]), (y[1]-y[0]))
        //b = ((x[1]-g_x[0]), (y[1]-g_y[0]))
        //a x b > 0 => b is on the left side of a
        if((x[1]-x[0])*(x[1]-g_x[0]) - (y[1]-y[0])*(y[1]-g_y[0]) < 0){
            iter_swap(x.begin()+0,x.begin()+1);
            iter_swap(y.begin()+0,y.begin()+1);
        }
    }

    //Set edge[0], edge[0] is the ordinary edge of 2 points from vertex[0]->vertex[1]
    this->configArraysForEdges(0,1,0,0,1,2,1,1,2);

    //Set edge[1], edge[1] is on the outer side of generating_point[0] from edge[0]
    this->configArraysForEdges(1,0,2,0,1,0,2,2,0);

    //Set edge[2], edge[2] is on the outer side of generating_point[1] from edge[0]
    this->configArraysForEdges(2,2,0,0,1,1,0,0,1);
}

void WingedEdge::constructThreePointsVoronoi()
{
    if(this->getNumPolygons() != 3){
        qDebug()<<"Number of generating points is not 3. To use this function, only 3 points is available.";
        return;
    }

    /*依照(x,y) sorts generating points in increasing order*/
    vector<struct g_point> g_p;
    struct g_point tmp_p;
    tmp_p.x = this->g_x[0];
    tmp_p.y = this->g_y[0];
    g_p.push_back(tmp_p);

    tmp_p.x = this->g_x[1];
    tmp_p.y = this->g_y[1];
    g_p.push_back(tmp_p);

    tmp_p.x = this->g_x[2];
    tmp_p.y = this->g_y[2];
    g_p.push_back(tmp_p);

    sort(g_p.begin(),g_p.end(),compare_g_point);
    this->g_x[0] = g_p[0].x;this->g_y[0] = g_p[0].y;
    this->g_x[1] = g_p[1].x;this->g_y[1] = g_p[1].y;
    this->g_x[2] = g_p[2].x;this->g_y[2] = g_p[2].y;

    //Judge three points' positions to construct the voronoi
    if( (fabs(this->g_y[0]-this->g_y[1])<1e-8) && (fabs(this->g_y[1]-this->g_y[2])<1e-8) ){
        /*三點共線-水平*/

        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);
        //畫兩條垂直edge
        //First edge : (x[0],y[0])~(x[1],y[1])
        this->x[0] = (this->g_x[0]+this->g_x[1])/2;
        this->y[0] = 0;     this->w[0] = 0;//points at infinity

        this->x[1] = (this->g_x[0]+this->g_x[1])/2;
        this->y[1] = 600;   this->w[1] = 0;//points at infinity

        //Second edge : (x[2],y[2])~(x[3],y[3])
        this->x[2] = (this->g_x[1]+this->g_x[2])/2;
        this->y[2] = 0;     this->w[2] = 0;//points at infinity

        this->x[3] = (this->g_x[1]+this->g_x[2])/2;
        this->y[3] = 600;   this->w[3] = 0;//points at infinity

        this->num_vertices = 4;

        //Setting edges
        this->num_edges=6;//2 oridinary edges, 4 augumented edges
        this->changeArraysForEdges(this->num_edges);

        /***Todo***/
        //Config edges' arrays
        //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
        this->configArraysForEdges(0,1,0,0,1,5,2,2,3);
        this->configArraysForEdges(1,2,1,2,3,4,5,3,4);
        this->configArraysForEdges(2,0,3,0,1,0,5,1,0);
        this->configArraysForEdges(3,1,3,1,3,0,2,4,1);
        this->configArraysForEdges(4,2,3,3,2,1,3,5,1);
        this->configArraysForEdges(5,1,3,2,0,1,4,2,0);

    }
    else if( (fabs(this->g_x[0]-this->g_x[1])<1e-8) && (fabs(this->g_x[1]-this->g_x[2])<1e-8) ){
        /*三點共線-垂直*/
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);
        //畫兩條水平edge
        //First edge : (x[0],y[0])~(x[1],y[1])
        this->x[0] = 0;
        this->y[0] = (this->g_y[0]+this->g_y[1])/2;
        this->w[0] = 0;//points at infinity

        this->x[1] = 600;
        this->y[1] = (this->g_y[0]+this->g_y[1])/2;
        this->w[1] = 0;//points at infinity

        //Second edge : (x[2],y[2])~(x[3],y[3])
        this->x[2] = 0;
        this->y[2] = (this->g_y[1]+this->g_y[2])/2;
        this->w[2] = 0;//points at infinity

        this->x[3] = 600;
        this->y[3] = (this->g_y[1]+this->g_y[2])/2;
        this->w[3] = 0;//points at infinity

        this->num_vertices = 4;

        //Setting edges
        this->num_edges=6;//2 oridinary edges, 4 augumented edges
        this->changeArraysForEdges(this->num_edges);

        /***Todo***/
        //Config edges' arrays
        this->configArraysForEdges(0,0,1,0,1,2,5,3,2);
        this->configArraysForEdges(1,1,2,2,3,5,4,4,3);
        this->configArraysForEdges(2,3,0,0,1,5,0,0,1);
        this->configArraysForEdges(3,3,1,1,3,2,0,1,4);
        this->configArraysForEdges(4,3,2,3,2,3,1,1,5);
        this->configArraysForEdges(5,3,1,2,0,4,1,0,2);

    }
    else if( fabs((this->g_y[1]-this->g_y[0])/(this->g_x[1]-this->g_x[0]) - (this->g_y[2]-this->g_y[1])/(this->g_x[2]-this->g_x[1])) < 1e-8 ){
        /*三點共線-其他*/
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        double x_1,y_1,x_2,y_2;
        x_1 = this->g_x[0];
        y_1 = this->g_y[0];
        x_2 = this->g_x[1];
        y_2 = this->g_y[1];

        //y = mx + b , x = ny + c
        double m,b,n,c;
        this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
        n = 1/m;        c = (-1)*b/m;

        /** Start adding vertices **/

        //Judge where the vertices should be.
        if(m>=0){
            //Line of (x[0],y[0]),(x[1],y[1])
            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[0] = 0;
                this->y[0] = b;
                this->w[0] = 0;
            }
            else{
                //first vertix is (c,0)
                this->x[0] = c;
                this->y[0] = 0;
                this->w[0] = 0;
            }

            //x_cross_y_600 is the x-coordinate of the point that intersect with y=600
            double x_cross_y_600 = n*600+c;
            if(x_cross_y_600 >=0 && x_cross_y_600 <=600){
                //second vertex is at the bound of y=600
                this->x[1] = x_cross_y_600;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[1] = 600;
                this->y[1] = m*600+b;
                this->w[1] = 0;
            }

            /******************************************************************/

            //Line of (x[1],y[1]),(x[2],y[2])
            x_1 = this->g_x[1];
            y_1 = this->g_y[1];
            x_2 = this->g_x[2];
            y_2 = this->g_y[2];
            this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
            n = 1/m;        c = (-1)*b/m;

            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[2] = 0;
                this->y[2] = b;
                this->w[2] = 0;
            }
            else{
                //first vertix is (c,0)
                this->x[2] = c;
                this->y[2] = 0;
                this->w[2] = 0;
            }

            //x_cross_y_600 is the x-coordinate of the point that intersect with y=600
            x_cross_y_600 = n*600+c;
            if(x_cross_y_600 >=0 && x_cross_y_600 <=600){
                //second vertex is at the bound of y=600
                this->x[3] = x_cross_y_600;
                this->y[3] = 600;
                this->w[3] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[3] = 600;
                this->y[3] = m*600+b;
                this->w[3] = 0;
            }
            /******************************************************************/

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,1,0,0,1,5,2,2,3);
            this->configArraysForEdges(1,2,1,2,3,4,5,3,4);
            this->configArraysForEdges(2,0,3,0,1,0,5,1,0);
            this->configArraysForEdges(3,1,3,1,3,0,2,4,1);
            this->configArraysForEdges(4,2,3,3,2,1,3,5,1);
            this->configArraysForEdges(5,1,3,2,0,1,4,2,0);

            /******************************************************************/
        }
        else{//m<0
            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[0] = 0;
                this->y[0] = b;
                this->w[0] = 0;
            }
            else{
                //first vertix is (n*600+c,600)
                this->x[0] = n*600+c;
                this->y[0] = 600;
                this->w[0] = 0;
            }

            //x_cross_y_0 is the x-coordinate of the point that intersect with y=0
            double x_cross_y_0 = n*0+c;

            if(x_cross_y_0 >0 && x_cross_y_0 <600){
                //second vertex is at the bound of y=0
                this->x[1] = x_cross_y_0;
                this->y[1] = 0;
                this->w[1] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[1] = 600;
                this->y[1] = m*600+b;
                this->w[1] = 0;
            }

            /******************************************************************/
            //Line of (x[1],y[1]),(x[2],y[2])
            x_1 = this->g_x[1];
            y_1 = this->g_y[1];
            x_2 = this->g_x[2];
            y_2 = this->g_y[2];
            this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
            n = 1/m;        c = (-1)*b/m;

            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[2] = 0;
                this->y[2] = b;
                this->w[2] = 0;
            }
            else{
                //first vertix is (n*600+c,600)
                this->x[2] = n*600+c;
                this->y[2] = 600;
                this->w[2] = 0;
            }

            //x_cross_y_0 is the x-coordinate of the point that intersect with y=0
            x_cross_y_0 = n*0+c;

            if(x_cross_y_0 >0 && x_cross_y_0 <600){
                //second vertex is at the bound of y=0
                this->x[3] = x_cross_y_0;
                this->y[3] = 0;
                this->w[3] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[3] = 600;
                this->y[3] = m*600+b;
                this->w[3] = 0;
            }
            /******************************************************************/

            //Setting edges
            this->num_edges=6;//2 oridinary edges, 4 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,1,0,1,0,5,2,2,3);
            this->configArraysForEdges(1,2,1,3,2,4,5,3,4);
            this->configArraysForEdges(2,0,3,1,0,0,5,1,0);
            this->configArraysForEdges(3,1,3,0,2,0,2,4,1);
            this->configArraysForEdges(4,2,3,2,3,1,3,5,1);
            this->configArraysForEdges(5,1,3,3,1,1,4,2,0);

            /******************************************************************/
        }
        /** Done add vertices **/

    }
    else if( (fabs(this->g_x[0]-this->g_x[1])<1e-8) || (fabs(this->g_x[1]-this->g_x[2])<1e-8) ){
        /*兩點垂直*/
        //1 oridinary vertex(circumcenter), 3 infinity vertices
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        if(fabs(this->g_x[0] - this->g_x[1]) < 1e-8){
            //2 points on left, 1 point on right
            this->x[0] = 0;
            this->y[0] = (this->g_y[0]+this->g_y[1])/2;
            this->w[0] = 0;

            double m_top,m_bot,b_top,b_bot;
            double n_top,n_bot,c_top,c_bot;

            if(fabs(this->g_y[1] - this->g_y[2]) < 1e-8){
                this->x[1] = (this->g_x[1] + this->g_x[2])/2;
                this->y[1] = 600;
                this->w[1] = 0;

                //Circumcenter
                this->x[3] = (this->g_x[1] + this->g_x[2])/2;
                this->y[3] = (this->g_y[0] + this->g_y[1])/2;
                this->w[3] = 1;
            }
            else{
                this->findPerpendicularBisector(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],m_top,b_top);
                n_top = 1/m_top;    c_top = (-1)*b_top/m_top;
                double x_cross_y_600;
                x_cross_y_600 = n_top*600+c_top;

                //point intersect with upper margin
                if(x_cross_y_600>=0 && x_cross_y_600<=600){
                    this->x[1] = x_cross_y_600;
                    this->y[1] = 600;
                    this->w[1] = 0;
                }
                else if(x_cross_y_600 <0){
                    this->x[1] = 0;
                    this->y[1] = b_top;
                    this->w[1] = 0;
                }
                else{
                    this->x[1] = 600;
                    this->y[1] = m_top*600+b_top;
                    this->w[1] = 0;
                }


                //Circumcenter
                this->x[3] = (b_bot-b_top)/(m_top-m_bot);
                this->y[3] = m_top * (this->y[3]) +b_top;
                this->w[3] = 1;

            }

            if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
                this->x[2] = (this->g_x[0] + this->g_x[2])/2;
                this->y[2] = 0;
                this->w[2] = 0;

                //Circumcenter
                this->x[3] = (this->g_x[0] + this->g_x[2])/2;
                this->y[3] = (this->g_y[0] + this->g_y[1])/2;
                this->w[3] = 1;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_bot,b_bot);
                n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

                double x_cross_y_0;
                x_cross_y_0 = n_bot*600+c_bot;

                //points intersect with lower margin
                if(x_cross_y_0>=0 && x_cross_y_0<=600){
                    this->x[2] = x_cross_y_0;
                    this->y[2] = 0;
                    this->w[2] = 0;
                }
                else if(x_cross_y_0 <0){
                    this->x[2] = 0;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }
                else{
                    this->x[2] = 600;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }


                //Circumcenter
                this->x[3] = (b_bot-b_top)/(m_top-m_bot);
                this->y[3] = m_top * (this->y[3]) +b_top;
                this->w[3] = 1;
            }
            /******************************************************************/

            //Setting edges
            this->num_edges=6;//3 oridinary edges, 3 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,0,1,0,3,5,3,1,2);
            this->configArraysForEdges(1,1,2,1,3,3,4,2,0);
            this->configArraysForEdges(2,2,0,2,3,4,5,0,1);
            this->configArraysForEdges(3,1,3,0,1,0,5,4,1);
            this->configArraysForEdges(4,2,3,1,2,1,3,5,2);
            this->configArraysForEdges(5,0,3,2,0,2,4,3,0);

            /******************************************************************/

        }
        else{
            //2 points on right, 1 point on left
            this->x[0] = 600;
            this->y[0] = (this->g_y[1]+this->g_y[2])/2;
            this->w[0] = 0;

            double m_top,m_bot,b_top,b_bot;
            double n_top,n_bot,c_top,c_bot;

            if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
                this->x[1] = (this->g_x[0] + this->g_x[2])/2;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_top,b_top);
                n_top = 1/m_top;
                c_top = (-1)*b_top/m_top;
                double x_cross_y_600;
                x_cross_y_600 = n_top*600+c_top;

                //point intersect with upper margin
                if(x_cross_y_600>=0 && x_cross_y_600<=600){
                    this->x[1] = x_cross_y_600;
                    this->y[1] = 600;
                    this->w[1] = 0;
                }
                else if(x_cross_y_600 <0){
                    this->x[1] = 0;
                    this->y[1] = b_top;
                    this->w[1] = 0;
                }
                else{
                    this->x[1] = 600;
                    this->y[1] = m_top*600+b_top;
                    this->w[1] = 0;
                }
            }

            if(fabs(this->g_y[0] - this->g_y[1]) < 1e-8){
                this->x[2] = (this->g_x[0] + this->g_x[1])/2;
                this->y[2] = 600;
                this->w[2] = 0;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[1],this->g_y[1],m_bot,b_bot);
                n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

                double x_cross_y_0;
                x_cross_y_0 = n_bot*600+c_bot;

                //points intersect with lower margin
                if(x_cross_y_0>=0 && x_cross_y_0<=600){
                    this->x[2] = x_cross_y_0;
                    this->y[2] = 0;
                    this->w[2] = 0;
                }
                else if(x_cross_y_0 <0){
                    this->x[2] = 0;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }
                else{
                    this->x[2] = 600;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }

            }
            //Circumcenter
            this->x[3] = (b_bot-b_top)/(m_top-m_bot);
            this->y[3] = m_top*this->x[3]+b_top;
            this->w[3] = 1;

            /******************************************************************/

            //Setting edges
            this->num_edges=6;//3 oridinary edges, 3 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,2,1,0,3,3,5,2,1);
            this->configArraysForEdges(1,0,2,1,3,4,3,0,2);
            this->configArraysForEdges(2,1,0,2,3,5,4,1,0);
            this->configArraysForEdges(3,3,2,0,1,5,0,1,4);
            this->configArraysForEdges(4,3,0,1,2,3,1,2,5);
            this->configArraysForEdges(5,3,1,2,0,4,2,0,3);

            /******************************************************************/
        }
    }
    else{
        /*一般情況*/

        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        /******* Divide 3 points as : left -- 1 point , right -- 2 points *******/

        if((this->g_y[2] -this->g_y[0])/(this->g_x[2] -this->g_x[0])< (this->g_y[1] -this->g_y[0])/(this->g_x[1] -this->g_x[0])){
            //Let upper-m be the vertex number 2
            //    lower-m be the vertex number 1
            double tmp = this->g_y[1];
            this->g_y[1] = this->g_y[2];
            this->g_y[2] = tmp;

            tmp = this->g_x[1];
            this->g_x[1] = this->g_x[2];
            this->g_x[2] = tmp;
        }

        double m_top,m_bot,b_top,b_bot;
        double n_top,n_bot,c_top,c_bot;

        //The coordinate of infinity vertex of left & upper-right point 's perpendicular bisector
        if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
            this->x[1] = (this->g_x[0] + this->g_x[2])/2;
            this->y[1] = 600;
            this->w[1] = 0;
        }
        else{
            this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_top,b_top);
            n_top = 1/m_top;
            c_top = (-1)*b_top/m_top;
            double x_cross_y_600;
            x_cross_y_600 = n_top*600+c_top;

            //point intersect with upper margin
            if(x_cross_y_600>=0 && x_cross_y_600<=600){
                this->x[1] = x_cross_y_600;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else if(x_cross_y_600 <0){
                this->x[1] = 0;
                this->y[1] = b_top;
                this->w[1] = 0;
            }
            else{
                this->x[1] = 600;
                this->y[1] = m_top*600+b_top;
                this->w[1] = 0;
            }
        }

        //The coordinate of infinity vertex of left & lower-right point 's perpendicular bisector
        if(fabs(this->g_y[0] - this->g_y[1]) < 1e-8){
            this->x[2] = (this->g_x[0] + this->g_x[1])/2;
            this->y[2] = 0;
            this->w[2] = 0;
        }
        else{
            this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[1],this->g_y[1],m_bot,b_bot);
            n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

            double x_cross_y_0;
            x_cross_y_0 = c_bot;

            //points intersect with lower margin
            if(x_cross_y_0>=0 && x_cross_y_0<=600){
                this->x[2] = x_cross_y_0;
                this->y[2] = 0;
                this->w[2] = 0;
            }
            else if(x_cross_y_0 <0){
                this->x[2] = 0;
                this->y[2] = b_bot;
                this->w[2] = 0;
            }
            else{
                this->x[2] = 600;
                this->y[2] = 600*m_bot+b_bot;
                this->w[2] = 0;
            }

        }


        //Circumcenter, which is an ordinary vertex
        if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
            this->x[3] = this->x[1];
            this->y[3] = m_bot*this->x[3]+b_bot;
            this->w[3] = 1;
        }
        else if(fabs(this->g_y[0] - this->g_y[1]) < 1e-8){
            this->x[3] = this->x[2];
            this->y[3] = m_top*this->x[3]+b_top;
            this->w[3] = 1;
        }
        else{
            this->x[3] = (b_bot-b_top)/(m_top-m_bot);
            this->y[3] = m_top*this->x[3]+b_top;
            this->w[3] = 1;
        }

        //The coordinate of infinity vertex of 2 right-most-generating-points' perpendicular bisector
        double m,b,n,c;
        double candidate1_x,candidate1_y,candidate2_x,candidate2_y;

        if(fabs(this->g_y[1]-this->g_y[2]) < 1e-8){
            candidate1_x = (this->g_x[1]+this->g_x[2])/2;
            candidate1_y = 0;
            candidate2_x = (this->g_x[1]+this->g_x[2])/2;
            candidate2_y = 600;
        }
        else{
            this->findPerpendicularBisector(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],m,b);
            n = 1/m;
            c = (-1)*b/m;

            candidate1_x = n*0+c;
            candidate1_y = 0;

            candidate2_x = n*600+c;
            candidate2_y = 600;

        }

        //Determine if the generating point at left side is at different side with circumcenter
        double cross_product_of_left_point = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],this->g_x[0],this->g_y[0]);
        double cross_product_of_candidate1 = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],candidate1_x,candidate1_y);

        if(cross_product_of_left_point*cross_product_of_candidate1<0){
            //the generating point at left side is at different side with circumcenter
            if(candidate1_x>=0 && candidate1_x<=600){
                this->x[0] = candidate1_x;
                this->y[0] = candidate1_y;
                this->w[0] = 0;
            }
            else if(candidate1_x < 0){
                this->x[0] = 0;
                this->y[0] = m*0+b;
                this->w[0] = 0;
            }
            else{
                this->x[0] = 600;
                this->y[0] = m*600+b;
                this->w[0] = 0;
            }
        }
        else{
            //candidate1 is at different side with circumcenter

            if(candidate2_x>=0 && candidate2_x<=600){
                this->x[0] = candidate2_x;
                this->y[0] = candidate2_y;
                this->w[0] = 0;
            }
            else if(candidate2_x < 0){
                this->x[0] = 0;
                this->y[0] = m*0+b;
                this->w[0] = 0;
            }
            else{
                this->x[0] = 600;
                this->y[0] = m*600+b;
                this->w[0] = 0;
            }
        }

        /******************************************************************/

        //Setting edges
        this->num_edges=6;//3 oridinary edges, 3 augumented edges
        this->changeArraysForEdges(this->num_edges);

        //Config edges' arrays
        //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
        this->configArraysForEdges(0,2,1,0,3,3,5,2,1);
        this->configArraysForEdges(1,0,2,1,3,4,3,0,2);
        this->configArraysForEdges(2,1,0,2,3,5,4,1,0);
        this->configArraysForEdges(3,3,2,0,1,5,0,1,4);
        this->configArraysForEdges(4,3,0,1,2,3,1,2,5);
        this->configArraysForEdges(5,3,1,2,0,4,2,0,3);

        /******************************************************************/
    }

}

double WingedEdge::cross_product(double x_0, double y_0, double x_1, double y_1, double x_2, double y_2)
{
    return (x_1-x_0)*(y_2-y_0) - (x_2-x_0)*(y_1-y_0);
}

void WingedEdge::divide(WingedEdge &W_l, WingedEdge &W_r)
{

    //Divide part
    vector<double> l_x,l_y,r_x,r_y;
    double L ;

    //Step 1 : Find a median line L perpendicular to the X-axis
    //which divides W into W_l and W_r, with equal sizes.
    //Use Prune-and-Search
    L = this->find_k_th(this->get_g_x(),this->g_x.size()/2);

    for(unsigned long i=0;i<this->g_x.size();i++){
        //num_polygons should be same as g_x.size() and g_y.size()
        //Less than m, put in left
        if(this->g_x[i] < L){
            l_x.push_back(g_x[i]);
            l_y.push_back(g_y[i]);
        }
        else if(this->g_x[i] >= L ){
            r_x.push_back(g_x[i]);
            r_y.push_back(g_y[i]);
        }
    }

    W_l = WingedEdge(l_x,l_y);
    W_r = WingedEdge(r_x,r_y);
    qDebug()<<"W_l.getNumPolygons()"<<W_l.getNumPolygons();
    qDebug()<<"W_r.getNumPolygons()"<<W_r.getNumPolygons();

}

void WingedEdge::merge(WingedEdge S_l, WingedEdge S_r)
{
    /******************/

    /* Construct a dividing piece-wise linear
     * hyperplane HP which is the locus of points
     * simultaneously closest to a point in S_l
     * and a point in S_r.
     */

    /* Step 1: Find the convex hulls of SL
     * and SR,denoted as Hull(SL) and Hull(SR),
     * respectively.
     *
     *******Construct a convex hull from a Voronoi diagram********
     *      Step 1: Find an infinite ray by examining all
     *              Voronoi edges.
     *      Step 2: Let Pi be the point to the left of the
     *              infinite ray. Pi is a convex hull vertex.
     *              Examine the Voronoi polygon of Pi to find
     *              the next infinite ray.
     *      Step 3: Repeat Step 2 until we return to the starting ray.
     *************************************************************
     */

    vector<int> Hull_Sl,inf_rays_Sl;
    S_l.constructConvexHull(&Hull_Sl,&inf_rays_Sl);
    vector<int> Hull_Sr,inf_rays_Sr;
    S_r.constructConvexHull(&Hull_Sr,&inf_rays_Sr);

    /* Step 2: Find segments PaPb and PcPd which join
     * HULL(SL) and HULL(SR) into a convex hull
     * (Pa and Pc belong to SL and Pb and
     *  Pd belong to SR).
     * Assume that PaPb lies above PcPd.
     * Let x = a, y = b, SG= PxPy and HP = empty
     */

    //Here, Pa.Pb.Pc.Pd are the INDEX of convex hull points
    //To get the number of each convex hull point,
    //use Hull_Sl[Pa].Hull_Sl[Pb].Hull_Sr[Pc].Hull_Sr[Pd]
    int Pa, Pb, Pc, Pd;
    find_outter_tangent_top(Pa,Pb,S_l,Hull_Sl,S_r,Hull_Sr);
    find_outter_tangent_bot(Pc,Pd,S_l,Hull_Sl,S_r,Hull_Sr);

    vector<bisector> HP;
    int Px = Pa, Py = Pb;

    /* Step 3: Find the perpendicular bisector of SG.
     * Denote it by BS. Let HP = HP∪{BS}.
     * If SG = PcPd, go to Step 5; otherwise, go to Step 4
     */
    vector<double> g_x_l = S_l.get_g_x(), g_y_l = S_l.get_g_y();
    vector<double> g_x_r = S_r.get_g_x(), g_y_r = S_r.get_g_y();

    vector<int> start_vertex_l =  S_l.get_start_vertex(), end_vertex_l =  S_l.get_end_vertex();
    vector<int> start_vertex_r =  S_r.get_start_vertex(), end_vertex_r =  S_r.get_end_vertex();
    vector<double> vertex_x_l = S_l.get_x(), vertex_y_l = S_l.get_y();
    vector<double> vertex_x_r = S_r.get_x(), vertex_y_r = S_r.get_y();


    bisector* BS;
    bool top = true;

    //(x1,y1) and (x2,y2) are the coordinates of Hull_Sl[Px] and Hull_Sl[Py]
    double x1,y1,x2,y2;
    //Previous bisector's intersection
    double prev_x,prev_y;

    do{
        BS = new bisector();
        //Find the line coefficients : ax+by+c=0 for BS
        x1 = g_x_l[Hull_Sl[Px]];     y1 = g_y_l[Hull_Sl[Px]];
        x2 = g_x_l[Hull_Sr[Py]];     y2 = g_y_l[Hull_Sr[Py]];
        BS->line = WingedEdge::findPerpendicularBisector(x1,y1,x2,y2);

        /* Step 4: The ray from VD(SL) and VD(SR) which
         * BS first intersects with must be a perpendicular
         * bisector of either PxPz or PyPz for some z.
         * If this ray is the perpendicular bisector of PyPz, then let SG = PxPz ;
         * otherwise, let SG = PzPy. Go to Step 3.
         */

        //PxPz, PxPz will be the infinite ray correspond to Px
        Line left_ray(vertex_x_l[start_vertex_l[inf_rays_Sl[Px]]]  ,  vertex_y_l[start_vertex_l[inf_rays_Sl[Px]]] ,
                      vertex_x_l[end_vertex_l[inf_rays_Sl[Px]]]    ,  vertex_y_l[end_vertex_l[inf_rays_Sl[Px]]]    );

        //PyPz, PyPz will be the infinite ray correspond to Py+1
        Line right_ray(vertex_x_r[start_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]]  ,  vertex_y_r[start_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]] ,
                       vertex_x_r[end_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]]    ,  vertex_y_r[end_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]]    );

        //BS and left_ray 's intersection point
        double inter_l_x, inter_l_y;

        //BS and right_ray 's intersection point
        double inter_r_x, inter_r_y;

        bool have_inter_l, have_inter_r;
        have_inter_l = Line::find_intersect(BS->line, left_ray, inter_l_x, inter_l_y);
        have_inter_r = Line::find_intersect(BS->line, right_ray, inter_r_x, inter_r_y);

        //The next point BS really intersect with
        double inter_x,inter_y;
        if(!have_inter_r || inter_l_y >= inter_r_y){
            //No intersection with right_ray, Must intersect with left_ray
            //Or left_ray got the upper intersection with BS

            inter_x = inter_l_x;
            inter_y = inter_l_y;

            //Px changes, clockwise
            Px = (Px +Hull_Sl.size()-1) % Hull_Sl.size();
        }
        else{
            //No intersection with left_ray, must intersect with right_ray
            //Or right_ray got the upper intersection with BS

            inter_x = inter_r_x;
            inter_y = inter_r_y;

            //Py changes, counter clockwise
            Py = (Py+1) % Hull_Sr.size();
        }

        if(top){
            //First BS i.e. BS of PxPy
            top = false;

            BS->x1 = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
            BS->y1 = inter_y+600;
        }
        else{
            BS->x1 = prev_x;
            BS->y1 = prev_y;
        }
        BS->x2 = inter_x;
        BS->y2 = inter_y;

        prev_x = inter_x;
        prev_y = inter_y;

        //HP = HP ∪ BS
        HP.push_back(*BS);

    }while(Px != Pc || Py != Pd);

    //Add last BS
    BS = new bisector();
    //Find the line coefficients : ax+by+c=0 for BS
    x1 = g_x_l[Hull_Sl[Px]];     y1 = g_y_l[Hull_Sl[Px]];
    x2 = g_x_l[Hull_Sr[Py]];     y2 = g_y_l[Hull_Sr[Py]];
    BS->line = WingedEdge::findPerpendicularBisector(x1,y1,x2,y2);


    if(top){
        //deal with the situation that Pa==Pc, Pb==Pd
        prev_y = 600;
        prev_x = (-1)*(BS->line.b*(prev_y) + BS->line.c)/BS->line.a;
    }

    BS->x1 = prev_x;
    BS->y1 = prev_y;
    BS->x2 = (-1)*(BS->line.b*(prev_y-600) + BS->line.c)/BS->line.a;
    BS->y2 = prev_y-600;

    //HP = HP ∪ BS
    HP.push_back(*BS);

    /* Step 5: Discard the edges of VD(SL) which extend to
     * the right of HP and discard the edges of VD(SR) which
     * extend to the left of HP.
     * The resulting graph is the Voronoi diagram of S = SL ∪ SR
     */

}

double WingedEdge::find_k_th(vector<double> S,unsigned long k)
{
    if(S.size() <= 20){
        //If the size is less than 20, sort it directly
        //O(1)

        sort(S.begin(),S.end());

        if(S.size()%2 == 0){
            return (S[S.size()/2]+S[S.size()/2 - 1])/2;
        }
        else{
            return S[S.size()/2];
        }
    }

    //Step 1: Divide S into n/5 subsets
    //        Add some dummy INF points to the last subset
    //        if n is not a net multiple of 5
    if(S.size() % 5 != 0){
        //Add some dummy INF points
        //O(1)
        for(unsigned long i=0;i<5-(S.size() % 5);i++){
            S.push_back(INF);
        }
    }

    //Step 2: Sort each subset of elements and add the median into "medians"
    //O(n)
    vector<double> medians;
    for(unsigned long i=0;i<S.size();i+=5){
        vector<double> tmp(S.begin()+i,S.begin()+i+4);
        sort(tmp.begin(),tmp.end());
        medians.push_back(tmp[2]);
    }

    //Step 3: Recursively sort "medians" to find the element p whih is the median of medians
    //T(n/5)
    double p = this->find_k_th(medians, S.size()/2);

    /** Not necessary to do Step 4 and 5 in this project, because p is the answer
     *  But for the convenience of future porting, I finished them. **/
    //Step 4: Partition S into S1, S2 and S3,
    //        which contain the elements less than, equal to, and greater than p, respectively.
    //O(n)
    vector<double> S1,S2,S3;

    for(unsigned long i=0;i<S.size();i++){
        if(S[i] < p)
            S1.push_back(S[i]);
        else if(fabs(S[i]-p)<1e-8)
            S2.push_back(S[i]);
        else
            S3.push_back(S[i]);
    }

    //Step 5: Determine where the median located in S1, S2 or S3.
    //        Prune the other 2 and recursively search
    //T(4n/5)

    if(S1.size() >= k){
        return find_k_th(S1,k);
    }
    else if(S1.size()+S2.size() >= k){
        /** In this project, it should return here **/
        return p;
    }
    else{
        return find_k_th(S3,k-S1.size()-S2.size());
    }
}

void WingedEdge::constructConvexHull(vector<int> *Hull, vector<int> *infinite_rays)
{
    //O(n)
    vector<int> HULL,inf_rays;
    //HULL is the verctor of counter clockwise order of conver hull points.
    //inf_rays is the vector of counter clockwise order of infinite rays used when finding convex hull.


    if(this->getNumPolygons() == 1 ){
        HULL.push_back(0);
    }
    else if(this->getNumPolygons() == 2){
        HULL.push_back(0);
        HULL.push_back(1);
        inf_rays.push_back(0);
    }
    else if(this->getNumPolygons() == 3 && threePointsSameLine()){
        //three points on the same line's convex hull is a line which conbined by the point on 2 extreame sides.
        HULL.push_back(0);
        HULL.push_back(2);

        //May need to be dealt seperately when merging
        inf_rays.push_back(0);
        inf_rays.push_back(1);
    }
    else{

        /*******Construct a convex hull from a Voronoi diagram********/

        /* Step 1: Find an infinite ray by examining all Voronoi edges.*/
        int infinite_ray = -1;
        for(unsigned long i=0 ; i < this->getNum_edges(); i++){
            //Examine if the edge is ordinary edge
            if(this->right_polygon[i] != this->getNumPolygons()+1 && this->left_polygon[i] != this->getNumPolygons()+1){
                //Examine if the edge is an infinite ray
                if(this->w[this->start_vertex[i]] == 0 || this->w[this->end_vertex[i]] == 0 ){
                    infinite_ray = i;
                    break;
                }
            }
        }


        if(infinite_ray == -1){
            qDebug()<<"constructConvexHull: Fail to find an infinite ray!";
            exit(-1);
        }


        /* Step 2: Let Pi be the point to the left of the infinite ray.
         *         Pi is a convex hull vertex.
         *         Examine the Voronoi polygon of Pi to find the next infinite ray.
         */

        int next_infinite_ray = infinite_ray;
        int tmp_line;
        int current_vertex;

        do{
            inf_rays.push_back(next_infinite_ray);

            if(this->w[this->start_vertex[next_infinite_ray]] == 0 ){
                //The ray is from infinite->oridinary
                //right generating point is a convex hull vertex
                HULL.push_back(this->right_polygon[next_infinite_ray]);

                current_vertex = this->end_vertex[next_infinite_ray];
                tmp_line = this->ccw_successor[next_infinite_ray];
            }
            else if(this->w[this->end_vertex[next_infinite_ray]] == 0 ){
                //The ray is from oridinary->infinite
                //left generating point is a convex hull vertex
                HULL.push_back(this->left_polygon[next_infinite_ray]);

                current_vertex = this->start_vertex[next_infinite_ray];
                tmp_line = this->ccw_predecessor[next_infinite_ray];
            }
            else{
                qDebug()<<"constructConvexHull: Current infinite ray is not an infinite ray";
                exit(-1);
            }

            //Find next infinite ray
            while(this->w[this->start_vertex[tmp_line]] == 1 && this->w[this->end_vertex[tmp_line]] == 1 ){

                if(current_vertex == this->start_vertex[tmp_line]){
                    tmp_line = this->ccw_successor[tmp_line];
                }
                else if(current_vertex == this->end_vertex[tmp_line]){
                    tmp_line = this->ccw_predecessor[tmp_line];
                }
                else{
                    qDebug()<<"constructConvexHull: Current tmp_line is not adjacent to previous one";
                    exit(-1);
                }
            }
            next_infinite_ray = tmp_line;

        /* Step 3: Repeat Step 2 until we return to the starting ray. */
        }while(next_infinite_ray != infinite_ray);
    }

    Hull = new vector<int>(HULL);
    infinite_rays = new vector<int>(inf_rays);
}

void WingedEdge::find_outter_tangent_top(int &Pa, int &Pb, WingedEdge Sl, vector<int> Hull_Sl, WingedEdge Sr, vector<int> Hull_Sr)
{
    //Hull_Sl is Sl's convex hull points' numbers sort in counter clockwise. Use it as a circular list.
    //Hull_Sr is Sr's convex hull points' numbers sort in counter clockwise. Use it as a circular list.

    vector<double> g_x_l = Sl.get_g_x(), g_y_l = Sl.get_g_y();
    vector<double> g_x_r = Sr.get_g_x(), g_y_r = Sr.get_g_y();

    //Find the # of right-most point in Hull_Sl and # of left-most point in Hull_Sr
    int right_most_Sl = 0, left_most_Sr = 0;
    for(unsigned long i = 1; i < Hull_Sl.size(); i++){
        if(g_x_l[Hull_Sl[i]] > g_x_l[Hull_Sl[right_most_Sl]])
            right_most_Sl = i;
    }
    for(unsigned long i = 1; i < Hull_Sr.size(); i++){
        if(g_x_r[Hull_Sr[i]] < g_x_l[Hull_Sr[left_most_Sr]])
            left_most_Sr = i;
    }

    //Take these 2 points as a segment. Assume that these 2 points will not be vertical
    //Determine the slope, if slope > 0, left should be moved first;
    //                        slope < 0, right should be moved first.
    double m = (g_y_r[Hull_Sr[left_most_Sr]] - g_y_l[Hull_Sl[right_most_Sl]]) / (g_x_r[Hull_Sr[left_most_Sr]] - g_x_l[Hull_Sl[right_most_Sl]]);

    int current_left = right_most_Sl, current_right = left_most_Sr;
    int to_be_moved;//indicate next point to be moved. 0 indicates current_left
                    //                                 1 indicates current_right
    if(m > 0)
        to_be_moved = 0;
    else
        to_be_moved = 1;

    //Move each point in turn, record if the point of tangent is reached
    bool fixed_left = false, fixed_right = false;

    while(!fixed_left && !fixed_right){
        if(to_be_moved == 0 ){
            if(fixed_left){
                //left is fixed, deal with right
                to_be_moved = 1;
                continue;
            }
            //Moved current_left counter clockwise
            int next_point = Hull_Sl[(current_left+1) % Hull_Sl.size()];

            //Calculate cross product of current_right=>current_left and current_right=>next_point
            //to determine if the outter tangent point is reached.
            double x_0 = g_x_r[Hull_Sr[current_right]];
            double y_0 = g_y_r[Hull_Sr[current_right]];

            double x_1 = g_x_l[Hull_Sl[current_left]];
            double y_1 = g_y_l[Hull_Sl[current_left]];

            double x_2 = g_x_l[Hull_Sl[next_point]];
            double y_2 = g_y_l[Hull_Sl[next_point]];

            if(this->cross_product(x_0,y_0,x_1,y_1,x_2,y_2) >= 0){
                //Next point is over the left outter tangent point,
                //so left outter tangent point should be current_left & fixed_left should be true
                to_be_moved = 1;
                fixed_left = true;
            }
            else{
                current_left = next_point;
                m =  (g_y_r[Hull_Sr[current_right]] - g_y_l[Hull_Sl[current_left]]) / (g_x_r[Hull_Sr[current_right]] - g_x_l[Hull_Sl[current_left]]);
                if(m > 0)
                    to_be_moved = 0;
                else
                    to_be_moved = 1;
            }
        }
        else if(to_be_moved == 1 ){
            if(fixed_right){
                //right is fixed, deal with left
                to_be_moved = 0;
                continue;
            }

            //Moved current_right clockwise
            int next_point = Hull_Sl[(current_right + Hull_Sl.size() -1) % Hull_Sl.size()];

            //Calculate cross product of current_left=>current_right and current_left=>next_point
            //to determine if the outter tangent point is reached.
            double x_0 = g_x_l[Hull_Sl[current_left]];
            double y_0 = g_y_l[Hull_Sl[current_left]];

            double x_1 = g_x_r[Hull_Sr[current_right]];
            double y_1 = g_y_r[Hull_Sr[current_right]];


            double x_2 = g_x_l[Hull_Sl[next_point]];
            double y_2 = g_y_l[Hull_Sl[next_point]];

            if(this->cross_product(x_0,y_0,x_1,y_1,x_2,y_2) <= 0){
                //Next point is over the right outter tangent point,
                //so right outter tangent point should be current_right & fixed_right should be true
                to_be_moved = 0;
                fixed_right = true;
            }
            else{
                current_right = next_point;
                m =  (g_y_r[Hull_Sr[current_right]] - g_y_l[Hull_Sl[current_left]]) / (g_x_r[Hull_Sr[current_right]] - g_x_l[Hull_Sl[current_left]]);
                if(m > 0)
                    to_be_moved = 0;
                else
                    to_be_moved = 1;
            }
        }

    }

    //left top outter tangent point (Pa) will be current_left
    Pa = current_left;
    //right top outter tangent point (Pb) will be current_right
    Pb = current_right;
}

void WingedEdge::find_outter_tangent_bot(int &Pc, int &Pd, WingedEdge Sl, vector<int> Hull_Sl, WingedEdge Sr, vector<int> Hull_Sr)
{
    vector<double> g_x_l = Sl.get_g_x(), g_y_l = Sl.get_g_y();
    vector<double> g_x_r = Sr.get_g_x(), g_y_r = Sr.get_g_y();

    //Find the # of right-most point in Hull_Sl and # of left-most point in Hull_Sr
    int right_most_Sl = 0, left_most_Sr = 0;
    for(unsigned long i = 1; i < Hull_Sl.size(); i++){
        if(g_x_l[Hull_Sl[i]] > g_x_l[Hull_Sl[right_most_Sl]])
            right_most_Sl = i;
    }
    for(unsigned long i = 1; i < Hull_Sr.size(); i++){
        if(g_x_r[Hull_Sr[i]] < g_x_l[Hull_Sr[left_most_Sr]])
            left_most_Sr = i;
    }

    //Take these 2 points as a segment. Assume that these 2 points will not be vertical
    //Determine the slope, if slope < 0, left should be moved first;
    //                        slope > 0, right should be moved first.
    double m = (g_y_r[Hull_Sr[left_most_Sr]] - g_y_l[Hull_Sl[right_most_Sl]]) / (g_x_r[Hull_Sr[left_most_Sr]] - g_x_l[Hull_Sl[right_most_Sl]]);

    int current_left = right_most_Sl, current_right = left_most_Sr;
    int to_be_moved;//indicate next point to be moved. 0 indicates current_left
                    //                                 1 indicates current_right
    if(m < 0)
        to_be_moved = 0;
    else
        to_be_moved = 1;

    //Move each point in turn, record if the point of tangent is reached
    bool fixed_left = false, fixed_right = false;

    while(!fixed_left && !fixed_right){
        if(to_be_moved == 0 ){
            if(fixed_left){
                //left is fixed, deal with right
                to_be_moved = 1;
                continue;
            }
            //Moved current_left clockwise
            int next_point = Hull_Sl[(current_left+ Hull_Sl.size() -1) % Hull_Sl.size()];

            //Calculate cross product of current_right=>current_left and current_right=>next_point
            //to determine if the outter tangent point is reached.
            double x_0 = g_x_r[Hull_Sr[current_right]];
            double y_0 = g_y_r[Hull_Sr[current_right]];

            double x_1 = g_x_l[Hull_Sl[current_left]];
            double y_1 = g_y_l[Hull_Sl[current_left]];

            double x_2 = g_x_l[Hull_Sl[next_point]];
            double y_2 = g_y_l[Hull_Sl[next_point]];

            if(this->cross_product(x_0,y_0,x_1,y_1,x_2,y_2) <= 0){
                //Next point is over the left outter tangent point,
                //so left outter tangent point should be current_left & fixed_left should be true
                to_be_moved = 1;
                fixed_left = true;
            }
            else{
                current_left = next_point;
                m =  (g_y_r[Hull_Sr[current_right]] - g_y_l[Hull_Sl[current_left]]) / (g_x_r[Hull_Sr[current_right]] - g_x_l[Hull_Sl[current_left]]);
                if(m < 0)
                    to_be_moved = 0;
                else
                    to_be_moved = 1;
            }
        }
        else if(to_be_moved == 1 ){
            if(fixed_right){
                //right is fixed, deal with left
                to_be_moved = 0;
                continue;
            }

            //Moved current_right counter clockwise
            int next_point = Hull_Sl[(current_right +1) % Hull_Sl.size()];

            //Calculate cross product of current_left=>current_right and current_left=>next_point
            //to determine if the outter tangent point is reached.
            double x_0 = g_x_l[Hull_Sl[current_left]];
            double y_0 = g_y_l[Hull_Sl[current_left]];

            double x_1 = g_x_r[Hull_Sr[current_right]];
            double y_1 = g_y_r[Hull_Sr[current_right]];


            double x_2 = g_x_l[Hull_Sl[next_point]];
            double y_2 = g_y_l[Hull_Sl[next_point]];

            if(this->cross_product(x_0,y_0,x_1,y_1,x_2,y_2) >= 0){
                //Next point is over the right outter tangent point,
                //so right outter tangent point should be current_right & fixed_right should be true
                to_be_moved = 0;
                fixed_right = true;
            }
            else{
                current_right = next_point;
                m =  (g_y_r[Hull_Sr[current_right]] - g_y_l[Hull_Sl[current_left]]) / (g_x_r[Hull_Sr[current_right]] - g_x_l[Hull_Sl[current_left]]);
                if(m < 0)
                    to_be_moved = 0;
                else
                    to_be_moved = 1;
            }
        }
    }

    //left bot outter tangent point (Pc) will be current_left
    Pc = current_left;
    //right bot outter tangent point (Pb) will be current_right
    Pd = current_right;

}

int WingedEdge::getNumPolygons()
{
    return num_polygons-1;//Except for p_infinity
}

bool WingedEdge::threePointsSameLine()
{
    return this->getNumPolygons()==3 &&
         (  ((fabs(this->g_y[0]-this->g_y[1])<1e-8) && (fabs(this->g_y[1]-this->g_y[2])<1e-8)) ||
            ((fabs(this->g_x[0]-this->g_x[1])<1e-8) && (fabs(this->g_x[1]-this->g_x[2])<1e-8)) ||
            ((fabs((this->g_y[1]-this->g_y[0])/(this->g_x[1]-this->g_x[0]) - (this->g_y[2]-this->g_y[1])/(this->g_x[2]-this->g_x[1])) < 1e-8))
         );
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

void WingedEdge::configArraysForEdges(int edge_num, int rp, int lp, int sv, int ev, int cw_p, int ccw_p, int cw_s, int ccw_s)
{
    //edge_num is the edge number to be config
    //rp stands for right_polygon
    //lp stands for left_polygon
    //sv stands for start_vertex
    //ev stands for end_vertex
    //cw_p stands for cw_predecessor
    //ccw_p stands for ccw_predecessor
    //cw_s stands for cw_successor
    //ccw_s stands for ccw_successor

    this->right_polygon[edge_num]=rp;
    this->left_polygon[edge_num]=lp;
    this->start_vertex[edge_num]=sv;
    this->end_vertex[edge_num]=ev;
    this->cw_predecessor[edge_num]=cw_p;
    this->ccw_predecessor[edge_num]=ccw_p;
    this->cw_successor[edge_num]=cw_s;
    this->ccw_successor[edge_num]=ccw_s;

}

vector<int> WingedEdge::getOrdinaryEdges()
{
    vector<int> e;
    //Don't return augumented edges but oridinary edges.
    //The augumented edges are adjacent to p_infinity
    for(unsigned long i=0;i<num_edges;i++){
        if(right_polygon[i] == this->getNumPolygons() || left_polygon[i] == this->getNumPolygons())
            continue;
        //put oridinary edge number into e
        e.push_back(i);
    }
    return e;
}

void WingedEdge::getOridinaryEdgesCoordinates(int i, double &x_1, double &x_2, double &y_1, double &y_2)
{
    x_1 = x[start_vertex[i]];
    x_2 = x[end_vertex[i]];
    y_1 = y[start_vertex[i]];
    y_2 = y[end_vertex[i]];

}

void WingedEdge::findPerpendicularBisector(double x_1, double y_1, double x_2, double y_2, double &m, double &b)
{
    if(fabs(y_1 - y_2)<1e-8){
        qDebug()<<"findPerpendicularBisector :divide by zero error! y_1 == y_2.";
        exit(-1);
    }
    m = (x_2-x_1)/(y_1-y_2);
    b = (x_1*x_1+y_1*y_1-x_2*x_2-y_2*y_2)/(2*(y_1-y_2));
}

Line WingedEdge::findPerpendicularBisector(double x_1, double y_1, double x_2, double y_2)
{
    Line result;

    //ax + by + c = 0

    if(fabs(y_1-y_2) < 1e-8){
        //x = (x_1+x_2)/2
        result.a = 1;
        result.b = 0;
        result.c = (-1)*(x_1+x_2)/2;
    }
    else{
        result.a = (x_1-x_2);
        result.b = (y_1-y_2);
        result.c = (x_2*x_2+y_2*y_2-x_1*x_1-y_1*y_1)/2;
    }

    return result;

}

vector<double> WingedEdge::get_g_x()
{
    return g_x;
}
vector<double> WingedEdge::get_g_y()
{
    return g_y;
}

vector<int> WingedEdge::get_w()
{
    return w;
}
vector<double> WingedEdge::get_x()
{
    return x;
}
vector<double> WingedEdge::get_y()
{
    return y;
}

vector<int> WingedEdge::get_right_polygon()
{
    return right_polygon;
}

vector<int> WingedEdge::get_left_polygon()
{
    return left_polygon;
}

vector<int> WingedEdge::get_start_vertex()
{
    return start_vertex;
}

vector<int> WingedEdge::get_end_vertex()
{
    return end_vertex;
}

vector<int> WingedEdge::getCw_predecessor() const
{
    return cw_predecessor;
}

void WingedEdge::setCw_predecessor(const vector<int> &value)
{
    cw_predecessor = value;
}

vector<int> WingedEdge::getCcw_predecessor() const
{
    return ccw_predecessor;
}

void WingedEdge::setCcw_predecessor(const vector<int> &value)
{
    ccw_predecessor = value;
}

vector<int> WingedEdge::getCw_successor() const
{
    return cw_successor;
}

void WingedEdge::setCw_successor(const vector<int> &value)
{
    cw_successor = value;
}

vector<int> WingedEdge::getCcw_successor() const
{
    return ccw_successor;
}

void WingedEdge::setCcw_successor(const vector<int> &value)
{
    ccw_successor = value;
}

int WingedEdge::getNum_edges() const
{
    return num_edges;
}

void WingedEdge::setNum_edges(int value)
{
    num_edges = value;
}

bool compare_g_point(const g_point a, const g_point b)
{
    return a.x < b.x || (( fabs(a.x-b.x) < 1e-8 )&&(a.y < b.y));
}
