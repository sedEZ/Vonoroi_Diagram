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
    this->w[0] = 1;
    this->w[1] = 1;
    this->num_vertices = 3;
    this->num_edges=3;

    //Initializing all edges' arrays in wingededge data structure
    this->changeArraysForEdges(this->num_edges);

    if(y_1 == y_2){//Vertical median line
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
    if( (int(this->g_y[0])==int(this->g_y[1])) && (int(this->g_y[1])==int(this->g_y[2])) ){
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
    }
    else if( (int(this->g_x[0])==int(this->g_x[1])) && (int(this->g_x[1])==int(this->g_x[2])) ){
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
        this->y[3] = (this->g_y[0]+this->g_y[1])/2;
        this->w[3] = 0;//points at infinity

        this->num_vertices = 4;

        //Setting edges
        this->num_edges=6;//2 oridinary edges, 4 augumented edges
        this->changeArraysForEdges(this->num_edges);

        /***Todo***/
        //Config edges' arrays
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

        /** Start add vertices **/

        //Judge where the vertices should be.
        if(m>=0){
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
                this->y[0] = 0;
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
        }
        /** Done add vertices **/
    }
    else if( (int(this->g_x[0]) == int(this->g_x[1])) || (int(this->g_x[1])==int(this->g_x[2])) ){
        /*兩點垂直*/
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
            }

            if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
                this->x[2] = (this->g_x[0] + this->g_x[2])/2;
                this->y[2] = 600;
                this->w[2] = 0;
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
            }
            /*Todo*/
            //Construct wingededge ds
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
            /*Todo*/
            //Construct wingededge ds

        }
    }
    else{
        /*一般情況*/
    }

    /*更新WingedEdge DS*/

}

void WingedEdge::divide(WingedEdge &W_l, WingedEdge &W_r)
{

    //Divide part
    vector<double> l_x,l_y,r_x,r_y;
    double m ;

    //Deal with 3 points at the same vertical line
    if(this->threePointsVertical()){
        qDebug()<<"3 points at the same vertical line";
        if(this->g_y[0]>=this->g_y[1]&&this->g_y[0]>=this->g_y[2]){
            l_x.push_back(this->g_x[0]);
            l_y.push_back(this->g_y[0]);

            r_x.push_back(this->g_x[1]);
            r_y.push_back(this->g_y[1]);
            r_x.push_back(this->g_x[2]);
            r_y.push_back(this->g_y[2]);
        }
        else if(this->g_y[1]>=this->g_y[0]&&this->g_y[1]>=this->g_y[2]){
            l_x.push_back(this->g_x[1]);
            l_y.push_back(this->g_y[1]);

            r_x.push_back(this->g_x[0]);
            r_y.push_back(this->g_y[0]);
            r_x.push_back(this->g_x[2]);
            r_y.push_back(this->g_y[2]);
        }
        else if(this->g_y[2]>=this->g_y[1]&&this->g_y[2]>=this->g_y[0]){
            l_x.push_back(this->g_x[2]);
            l_y.push_back(this->g_y[2]);

            r_x.push_back(this->g_x[0]);
            r_y.push_back(this->g_y[0]);
            r_x.push_back(this->g_x[1]);
            r_y.push_back(this->g_y[1]);
        }
    }
    else{
        m = this->findMedianLine();//Three points version
        for(unsigned long i=0;i<this->g_x.size();i++){
            //num_polygons should be same as g_x.size() and g_y.size()
            //Less than m, put in left
            if(this->g_x[i] <m){
                l_x.push_back(g_x[i]);
                l_y.push_back(g_y[i]);
            }
            else if(this->g_x[i] >= m ){
                r_x.push_back(g_x[i]);
                r_y.push_back(g_y[i]);
            }
        }
        if(r_x.size()>2){
            if(r_x[0]==m){
                l_x.push_back(r_x[0]);
                l_y.push_back(r_y[0]);
            }
            else if(r_x[1]==m){y.push_back(r_y[0]);
                r_x.erase(r_x.begin()+0);
                r_y.erase(r_y.begin()+0);
                l_x.push_back(r_x[1]);
                l_y.push_back(r_y[1]);
                r_x.erase(r_x.begin()+1);
                r_y.erase(r_y.begin()+1);
            }
        }
    }
    W_l = WingedEdge(l_x,l_y);
    W_r = WingedEdge(r_x,r_y);
    qDebug()<<"W_l.getNumPolygons()"<<W_l.getNumPolygons();
    qDebug()<<"W_r.getNumPolygons()"<<W_r.getNumPolygons();

}

void WingedEdge::merge(WingedEdge S_l, WingedEdge S_r)
{
        qDebug()<<"merge";
        bool swapped = false;

        qDebug()<<"S_l.getNumPolygons()"<<S_l.getNumPolygons();
        if(S_l.getNumPolygons()>1){
            qDebug()<<"1";
            WingedEdge tmp = S_l;
            S_l = S_r;
            S_r = tmp;
            swapped = true;
        }
    //Temporary version
        vector<double> S_l_g_x = S_l.get_g_x();
        vector<double> S_l_g_y = S_l.get_g_y();
        vector<double> S_r_g_x = S_r.get_g_x();
        vector<double> S_r_g_y = S_r.get_g_y();

        double S_r_upper_x ,S_r_upper_y , S_r_lower_x ,S_r_lower_y;
        double m1 = (S_r_g_y[0]-S_l_g_y[0])/(S_r_g_x[0]-S_l_g_x[0]);
        double m2 = (S_r_g_y[1]-S_l_g_y[0])/(S_r_g_x[1]-S_l_g_x[0]);

        qDebug()<<"m1 = "<<m1<<", m2 = "<<m2;
        //Slope smaller is upper
        if(m1 < m2){
            S_r_upper_x = S_r_g_x[1];
            S_r_upper_y = S_r_g_y[1];
            S_r_lower_x = S_r_g_x[0];
            S_r_lower_y = S_r_g_y[0];
        }
        else if(m1 > m2){
            S_r_upper_x = S_r_g_x[0];
            S_r_upper_y = S_r_g_y[0];
            S_r_lower_x = S_r_g_x[1];
            S_r_lower_y = S_r_g_y[1];
        }
        else if(m1 == m2){
            /*double dis_0 =(S_r_g_x[0]-S_l_g_x[0])*(S_r_g_x[0]-S_l_g_x[0])
                         +(S_r_g_y[0]-S_l_g_y[0])*(S_r_g_y[0]-S_l_g_y[0]);
            double dis_1 =(S_r_g_x[1]-S_l_g_x[0])*(S_r_g_x[1]-S_l_g_x[0])
                    +(S_r_g_y[1]-S_l_g_y[0])*(S_r_g_y[1]-S_l_g_y[0]);*/
            S_r_upper_x = S_r_g_x[0];
            S_r_upper_y = S_r_g_y[0];
            S_r_lower_x = S_r_g_x[1];
            S_r_lower_y = S_r_g_y[1];
        }


        //upper point
        double x_1 = S_l_g_x[0] , y_1 = S_l_g_y[0];
        double x_2 = S_r_upper_x , y_2 = S_r_upper_y;
        //y = mx + b
        double m,b;
        findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
        //x = ny + c
        double n = (y_1-y_2)/(x_2-x_1);
        double c = b*(y_2-y_1)/(x_2-x_1);

        double x_bot = n*600+c, y_bot = 600;

        //lower point
        x_1 = S_l_g_x[0] , y_1 = S_l_g_y[0];
        x_2 = S_r_lower_x , y_2 = S_r_lower_y;
        //y = mx + b
        findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
        //x = ny + c
        n = (y_1-y_2)/(x_2-x_1);
        c = b*(y_2-y_1)/(x_2-x_1);
        double x_top = n*0+c, y_top = 0;
    /***************************************************************/
        vector<int> S_r_w = S_r.get_w();
        vector<double> S_r_x = S_r.get_x();
        vector<double> S_r_y = S_r.get_y();
        double m_r = (S_r_y[1]-S_r_y[0])/(S_r_x[1]-S_r_x[0]);
        double b_r = S_r_y[0]-m_r*S_r_x[0];

        //Cross point
        double x_mid = (b_r-b)/(m-m_r) ,y_mid = m*x_mid+b;

        this->num_edges=3;
        this->changeArraysForEdges(this->num_edges);

        if((S_r_x[1] > S_r_x[0] && !swapped) || (S_r_x[1] < S_r_x[0] && swapped)){
            this->x.push_back(S_r_x[1]);//right-most vertex of S_r
            this->x.push_back(x_mid);
            this->x.push_back(x_top);
            this->x.push_back(x_bot);

            this->y.push_back(S_r_y[1]);//right-most vertex of S_r
            this->y.push_back(y_mid);
            this->y.push_back(y_top);
            this->y.push_back(y_bot);
        }
        else{
            this->x.push_back(S_r_x[0]);//left-most vertex of S_r
            this->x.push_back(x_mid);
            this->x.push_back(x_top);
            this->x.push_back(x_bot);

            this->y.push_back(S_r_y[0]);//left-most vertex of S_r
            this->y.push_back(y_mid);
            this->y.push_back(y_top);
            this->y.push_back(y_bot);

        }
        //Todo : adjust WingedEdge ds

        //Man-made judgement, not general
        this->start_vertex[0] = 0;
        this->end_vertex[0] = 1;
        this->start_vertex[1] = 2;
        this->end_vertex[1] = 1;
        this->start_vertex[2] = 1;
        this->end_vertex[2] = 3;

        //Wrong assighment, but these can help drawing
        this->right_polygon[0]=0;
        this->left_polygon[0]=0;
        this->right_polygon[1]=0;
        this->left_polygon[1]=0;
        this->right_polygon[2]=0;
        this->left_polygon[2]=0;

   // }
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
    return num_polygons==3 && (int)g_x[0]==(int)g_x[1] && (int)g_x[1]==(int)g_x[2];
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

void WingedEdge::findPerpendicularBisector(double x_1, double y_1, double x_2, double y_2, double &m, double &b)
{
    if(fabs(y_1 - y_2)<1e-8){
        qDebug()<<"findPerpendicularBisector :divide by zero error! y_1 == y_2.";
        exit(-1);
    }
    m = (x_2-x_1)/(y_1-y_2);
    b = (x_1*x_1+y_1*y_1-x_2*x_2-y_2*y_2)/(2*(y_1-y_2));
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


bool compare_g_point(const g_point a, const g_point b)
{
    if(a.x == b.x){
        return a.y < b.y;
    }
    else{
        return a.x < b.x;
    }
}
