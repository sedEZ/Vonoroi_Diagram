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
    this->num_vertices = 1;
    this->num_edges = 1;
    this->w.push_back(0);
    this->x.push_back(this->g_x[0]);
    this->y.push_back(this->g_y[0]);
    this->changeArraysForEdges(this->num_edges);

    this->configArraysForEdges(0,0,1,0,0,0,0,0,0);

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

    this->w.resize(3);
    this->w[0] = 0;
    this->w[1] = 1;
    this->w[2] = 0;
    this->num_vertices = 3;
    this->num_edges=4;

    //Initializing all edges' arrays in wingededge data structure
    this->changeArraysForEdges(this->num_edges);

    if(fabs(y_1 - y_2)< 1e-8){//Vertical median line
        //x = b
        double b = (x_1+x_2)/2;

        //(x[0],y[0]) = (b,0)
        x.push_back(b);
        y.push_back(0.0);

        //(x[1],y[1]) = (b,y_1)
        x.push_back(b);
        y.push_back(y_1);

        //(x[2],y[2]) = (b,600)
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
        //(x_0,y_0) = (0,c)
        x.push_back(0.0);
        y.push_back(c);

        //(x_1,y_1) = (x_1,c)
        x.push_back(x_1);
        y.push_back(c);

        //(x_2,y_2) = (600,c)
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

        //Add middle vertex
        x.push_back((x_1+x_2)/2);
        y.push_back((y_1+y_2)/2);

        iter_swap(x.begin()+1,x.begin()+2);
        iter_swap(y.begin()+1,y.begin()+2);

    }

    //Set edge[0], edge[0] is the ordinary edge of 2 points from vertex[0]->vertex[1]
    this->configArraysForEdges(0,1,0,0,1,3,2,1,1);


    //Set edge[1], edge[1] is the ordinary edge of 2 points from vertex[1]->vertex[2]
    this->configArraysForEdges(1,1,0,1,2,0,0,2,3);


    //Set edge[2], edge[2] is on the outer side of generating_point[0] from edge[0]
    this->configArraysForEdges(2,0,2,0,2,0,3,3,1);

    //Set edge[3], edge[3] is on the outer side of generating_point[1] from edge[0]
    this->configArraysForEdges(3,2,0,0,2,2,0,1,2);
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

            this->num_vertices = 4;
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

                double candidate1_x,candidate1_y,candidate2_x,candidate2_y;

                candidate1_x = n_top*0+c_top;
                candidate1_y = 0;

                candidate2_x = n_top*600+c_top;
                candidate2_y = 600;

                //Determine if the generating point at left side is at different side with circumcenter
                double cross_product_of_left_point = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],this->g_x[0],this->g_y[0]);
                double cross_product_of_candidate1 = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],candidate1_x,candidate1_y);

                if(cross_product_of_left_point*cross_product_of_candidate1<0){
                    //the generating point at left side is at different side with circumcenter
                    if(candidate1_x>=0 && candidate1_x<=600){
                        this->x[1] = candidate1_x;
                        this->y[1] = candidate1_y;
                        this->w[1] = 0;
                    }
                    else if(candidate1_x < 0){
                        this->x[1] = 0;
                        this->y[1] = m_top*0+b_top;
                        this->w[1] = 0;
                    }
                    else{
                        this->x[1] = 600;
                        this->y[1] = m_top*600+b_top;
                        this->w[1] = 0;
                    }
                }
                else{
                    //candidate1 is at different side with circumcenter

                    if(candidate2_x>=0 && candidate2_x<=600){
                        this->x[1] = candidate2_x;
                        this->y[1] = candidate2_y;
                        this->w[1] = 0;
                    }
                    else if(candidate2_x < 0){
                        this->x[1] = 0;
                        this->y[1] = m_top*0+b_top;
                        this->w[1] = 0;
                    }
                    else{
                        this->x[1] = 600;
                        this->y[1] = m_top*600+b_top;
                        this->w[1] = 0;
                    }
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

                double candidate1_x,candidate1_y,candidate2_x,candidate2_y;

                candidate1_x = n_bot*0+c_bot;
                candidate1_y = 0;

                candidate2_x = n_bot*600+c_bot;
                candidate2_y = 600;


                double cross_product_of_left_point = cross_product(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],this->g_x[1],this->g_y[1]);
                double cross_product_of_candidate1 = cross_product(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],candidate1_x,candidate1_y);

                if(cross_product_of_left_point*cross_product_of_candidate1<0){
                    if(candidate1_x>=0 && candidate1_x<=600){
                        this->x[2] = candidate1_x;
                        this->y[2] = candidate1_y;
                        this->w[2] = 0;
                    }
                    else if(candidate1_x < 0){
                        this->x[2] = 0;
                        this->y[2] = m_bot*0+b_bot;
                        this->w[2] = 0;
                    }
                    else{
                        this->x[2] = 600;
                        this->y[2] = m_bot*600+b_bot;
                        this->w[2] = 0;
                    }
                }
                else{
                    //candidate1 is at different side with circumcenter

                    if(candidate2_x>=0 && candidate2_x<=600){
                        this->x[2] = candidate2_x;
                        this->y[2] = candidate2_y;
                        this->w[2] = 0;
                    }
                    else if(candidate2_x < 0){
                        this->x[2] = 0;
                        this->y[2] = m_bot*0+b_bot;
                        this->w[2] = 0;
                    }
                    else{
                        this->x[2] = 600;
                        this->y[2] = m_bot*600+b_bot;
                        this->w[2] = 0;
                    }
                }


                //Circumcenter
                this->x[3] = (b_bot-b_top)/(m_top-m_bot);
                this->y[3] = m_top * (this->y[3]) +b_top;
                this->w[3] = 1;
            }
            /******************************************************************/

            this->num_vertices = 4;
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

            this->num_vertices = 4;
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

        this->num_vertices = 4;
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

    /* Step 0: Combine the 2 WingedEdge data structure into 1
     * which S_l be the first half and S_r be the last half
     */
    this->combineWingedEdges(S_l,S_r);

    //qDebug()<<"S_r.getNum_vertices() = "<<S_r.getNum_vertices();
    //this->output_all_data_structures();
    //S_l.output_all_data_structures();
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
    S_l.constructConvexHull(Hull_Sl,inf_rays_Sl);
    vector<int> Hull_Sr,inf_rays_Sr;
    S_r.constructConvexHull(Hull_Sr,inf_rays_Sr);

    //qDebug()<<"Hull_Sr[0] = "<<Hull_Sr[0]<<"; Hull_Sr[1] = "<<Hull_Sr[1]<<"; Hull_Sr[2] = "<<Hull_Sr[2];

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

    //qDebug()<<"Pa = "<<Pa<<"; Pb = "<<Pb<<"; Pc = "<<Pc<<"; Pd = "<<Pd;

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

    int num_p_l = S_l.getNumPolygons();
    int num_e_l = S_l.getNum_edges();
    int num_v_l = S_l.getNum_vertices();

    //BS and left_ray 's intersection point
    double inter_l_x, inter_l_y;

    //BS and right_ray 's intersection point
    double inter_r_x, inter_r_y;

    bool have_inter_l, have_inter_r;
    do{
        BS = new bisector();
        //Find the line coefficients : ax+by+c=0 for BS
        x1 = g_x_l[Hull_Sl[Px]];     y1 = g_y_l[Hull_Sl[Px]];
        x2 = g_x_r[Hull_Sr[Py]];     y2 = g_y_r[Hull_Sr[Py]];

        //qDebug()<<"x1 = "<<x1<<" ; y1 = "<<y1<<" ; x2 = "<<x2<<" ; y2 = "<<y2;

        BS->line = WingedEdge::findPerpendicularBisector(x1,y1,x2,y2);

        //qDebug()<<"BS->line.a = "<<BS->line.a<<" ; BS->line.b = "<<BS->line.b<<" ; BS->line.b = "<<BS->line.c;


        /* Step 4: The ray from VD(SL) and VD(SR) which
         * BS first intersects with must be a perpendicular
         * bisector of either PxPz or PyPz for some z.
         * If this ray is the perpendicular bisector of PyPz, then let SG = PxPz ;
         * otherwise, let SG = PzPy. Go to Step 3.
         */
        //left_edge records the numbering of final left_ray; so as right_edge
        int left_edge,right_edge;

        Line left_ray,right_ray;
        //PxPz, PxPz will be the infinite ray correspond to Px or Px+1
        Line left_ray_1(vertex_x_l[start_vertex_l[inf_rays_Sl[Px]]]  ,  vertex_y_l[start_vertex_l[inf_rays_Sl[Px]]] ,
                vertex_x_l[end_vertex_l[inf_rays_Sl[Px]]]    ,  vertex_y_l[end_vertex_l[inf_rays_Sl[Px]]] );
        Line left_ray_2(vertex_x_l[start_vertex_l[inf_rays_Sl[(Px+1)%inf_rays_Sl.size()]]]  ,  vertex_y_l[start_vertex_l[inf_rays_Sl[(Px+1)%inf_rays_Sl.size()]]] ,
                vertex_x_l[end_vertex_l[inf_rays_Sl[(Px+1)%inf_rays_Sl.size()]]]    ,  vertex_y_l[end_vertex_l[inf_rays_Sl[(Px+1)%inf_rays_Sl.size()]]] );
        if(Px == (Px+1)%inf_rays_Sl.size()){
            //only 1 candidate
            left_ray = left_ray_1;

            left_edge = inf_rays_Sl[Px];
        }
        else{
            double inter_l_x_1, inter_l_y_1;
            double inter_l_x_2, inter_l_y_2;
            bool have_inter_l_1 = Line::find_intersect(BS->line, left_ray_1, inter_l_x_1, inter_l_y_1);
            bool have_inter_l_2 = Line::find_intersect(BS->line, left_ray_2, inter_l_x_2, inter_l_y_2);
            if(!have_inter_l_2 || inter_l_y_1 > inter_l_y_2){
                inter_l_x = inter_l_x_1;
                inter_l_y = inter_l_y_1;
                have_inter_l = have_inter_l_1;

                left_edge = inf_rays_Sl[Px];
            }
            else if(!have_inter_l_1 || inter_l_y_2 > inter_l_y_1){
                inter_l_x = inter_l_x_2;
                inter_l_y = inter_l_y_2;
                have_inter_l = have_inter_l_2;

                left_edge = inf_rays_Sl[(Px+1)%inf_rays_Sl.size()];
            }
            else{
                qDebug()<<"Failed when finding left_ray";
                //exit(-1);
            }
        }
        //qDebug()<<"left_ray.a = "<<left_ray.a<<" ; left_ray.b = "<<left_ray.b<<" ; left_ray.c = "<<left_ray.c;

        //PyPz, PyPz will be the infinite ray correspond to Py or Py+1
        Line right_ray_1(vertex_x_r[start_vertex_r[inf_rays_Sr[Py]]]  ,  vertex_y_r[start_vertex_r[inf_rays_Sr[Py]]] ,
                       vertex_x_r[end_vertex_r[inf_rays_Sr[Py]]]    ,  vertex_y_r[end_vertex_r[inf_rays_Sr[Py]]] );
        Line right_ray_2(vertex_x_r[start_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]]  ,  vertex_y_r[start_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]] ,
                       vertex_x_r[end_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]]    ,  vertex_y_r[end_vertex_r[inf_rays_Sr[(Py+1)%inf_rays_Sr.size()]]] );
        if(Py == (Py+1)%inf_rays_Sr.size()){
            //only 1 candidate
            right_ray = right_ray_1;

            right_edge = inf_rays_Sr[Py];
        }
        else{
            double inter_r_x_1, inter_r_y_1;
            double inter_r_x_2, inter_r_y_2;
            bool have_inter_r_1 = Line::find_intersect(BS->line, left_ray_1, inter_r_x_1, inter_r_y_1);
            bool have_inter_r_2 = Line::find_intersect(BS->line, left_ray_2, inter_r_x_2, inter_r_y_2);
            if(!have_inter_r_2 || inter_r_y_1 > inter_r_y_2){
                inter_r_x = inter_r_x_1;
                inter_r_y = inter_r_y_1;
                have_inter_r = have_inter_r_1;

                right_edge = inf_rays_Sr[Py];
            }
            else if(!have_inter_r_1 || inter_r_y_2 > inter_r_y_1){
                inter_r_x = inter_r_x_2;
                inter_r_y = inter_r_y_2;
                have_inter_r = have_inter_r_2;

                right_edge = inf_rays_Sr[(Py+1)%inf_rays_Sr.size()];
            }
            else{
                qDebug()<<"Failed when finding left_ray";
                //exit(-1);
            }
        }
        //qDebug()<<"right_ray.a = "<<right_ray.a<<" ; right_ray.b = "<<right_ray.b<<" ; right_ray.c = "<<right_ray.c;
        //qDebug()<<"have_inter_l = "<<have_inter_l<<"; have_inter_r = "<<have_inter_r;

        //The next point BS really intersect with
        double inter_x,inter_y;

        if(have_inter_l && have_inter_r &&fabs(inter_l_x-inter_r_x) < 1e-8 && fabs(inter_l_y-inter_r_y) < 1e-8){
            inter_x = inter_l_x;
            inter_y = inter_l_y;
            int vertex_to_move;

            if(top){
                prev_x = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
                prev_y = inter_y+600;
            }

            /********* Move 2 infinite vertex to (inter_x,inter_y) **********/
            if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_l[start_vertex_l[left_edge]],vertex_y_l[start_vertex_l[left_edge]]) > 0){
                //start_vertex of left_edge is the infinite vertex
                this->x[start_vertex_l[left_edge]] = inter_x;
                this->y[start_vertex_l[left_edge]] = inter_y;
                this->w[start_vertex_l[left_edge]] = 1;//Become inner vertex

                vertex_to_move = start_vertex_l[left_edge];
                /*
                qDebug()<<"start_vertex_l[left_edge] = "<<start_vertex_l[left_edge];
                qDebug()<<"vertex_x_l[start_vertex_l[left_edge]] = "<<vertex_x_l[start_vertex_l[left_edge]];
                qDebug()<<"vertex_y_l[start_vertex_l[left_edge]] = "<<vertex_y_l[start_vertex_l[left_edge]];
                qDebug()<<"prev_x = "<<prev_x<<"; prev_y = "<<prev_y;
                qDebug()<<"inter_x = "<<inter_x<<"; inter_y = "<<inter_y;
                */
            }
            else if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_l[end_vertex_l[left_edge]],vertex_y_l[end_vertex_l[left_edge]]) > 0){
                this->x[end_vertex_l[left_edge]] = inter_x;
                this->y[end_vertex_l[left_edge]] = inter_y;
                this->w[end_vertex_l[left_edge]] = 1;//Become inner vertex

                vertex_to_move = end_vertex_l[left_edge];
                //qDebug()<<"end_vertex_l[left_edge] = "<<end_vertex_l[left_edge];
            }
            else{
                qDebug()<<"merge: Both start_vertex and end_vertex of left_ray are not on the left side of BS";
            }

            //Now move right_ray
            if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_r[start_vertex_r[right_edge]],vertex_y_r[start_vertex_r[right_edge]]) < 0){
                //start_vertex of right_edge is the infinite vertex
                this->start_vertex[ right_edge + num_e_l] = vertex_to_move;
                qDebug()<<"right_edge + num_e_l = "<<right_edge + num_e_l;
            }
            else if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_r[end_vertex_r[right_edge]],vertex_y_r[end_vertex_r[right_edge]]) < 0){
                qDebug()<<"right_edge + num_e_l = "<<right_edge + num_e_l;
                this->end_vertex[ right_edge + num_e_l] = vertex_to_move;

            }
            else{
                qDebug()<<"merge: Both start_vertex and end_vertex of right_ray are not on the left side of BS";
            }
            /******************************************************************/


            if(!top){
                this->num_edges++;

                *this->ccw_successor.end() = this->num_edges - 1;

                this->right_polygon.push_back(Hull_Sl[Px]);
                this->left_polygon.push_back(Hull_Sr[Py] + num_p_l);

                this->start_vertex.push_back(*this->end_vertex.end());
                this->end_vertex.push_back(this->num_vertices-1);
                this->cw_predecessor.push_back(this->num_edges-2);
                this->ccw_predecessor.push_back(right_edge + num_e_l);

                this->cw_successor.push_back(right_edge + num_e_l);
                this->ccw_successor.push_back(left_edge);

            }

            //Px changes, clockwise
            Px = (Px + Hull_Sl.size() - 1) % Hull_Sl.size();
            //Py changes, counter clockwise
            Py = (Py + 1) % Hull_Sr.size();
        }
        else if(!have_inter_r || inter_l_y >= inter_r_y){
            //No intersection with right_ray, Must intersect with left_ray
            //Or left_ray got the upper intersection with BS
            inter_x = inter_l_x;
            inter_y = inter_l_y;

            if(top){
                prev_x = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
                prev_y = inter_y+600;
            }

            /********* Move the infinite vertex to (inter_x,inter_y) **********/
            if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_l[start_vertex_l[left_edge]],vertex_y_l[start_vertex_l[left_edge]]) > 0){
                //start_vertex of left_edge is the infinite vertex
                this->x[start_vertex_l[left_edge]] = inter_x;
                this->y[start_vertex_l[left_edge]] = inter_y;
                this->w[start_vertex_l[left_edge]] = 1;//Become inner vertex
            }
            else if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_l[end_vertex_l[left_edge]],vertex_y_l[end_vertex_l[left_edge]]) > 0){
                this->x[end_vertex_l[left_edge]] = inter_x;
                this->y[end_vertex_l[left_edge]] = inter_y;
                this->w[end_vertex_l[left_edge]] = 1;//Become inner vertex
            }
            else{
                qDebug()<<"merge: Both start_vertex and end_vertex of left_ray are not on the left side of BS";
                exit(-1);
            }
            /******************************************************************/

            if(!top){
                this->num_edges++;

                *this->ccw_successor.end() = this->num_edges - 1;

                this->right_polygon.push_back(Hull_Sl[Px]);
                this->left_polygon.push_back(Hull_Sr[Py] + num_p_l);

                this->start_vertex.push_back(*this->end_vertex.end());
                this->end_vertex.push_back(this->num_vertices-1);
                this->cw_predecessor.push_back(this->num_edges-2);
                this->ccw_predecessor.push_back(inf_rays_Sr[Py] + num_e_l);

                this->cw_successor.push_back(this->num_edges);
                this->ccw_successor.push_back(left_edge);

            }
            else{
                prev_x = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
                prev_y = inter_y+600;
            }

            //Px changes, clockwise
            Px = (Px + Hull_Sl.size() - 1) % Hull_Sl.size();

        }
        else{
            //No intersection with left_ray, must intersect with right_ray
            //Or right_ray got the upper intersection with BS

            inter_x = inter_r_x;
            inter_y = inter_r_y;

            if(top){
                prev_x = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
                prev_y = inter_y+600;
            }

            /*
            qDebug()<<"Hull_Sr[Py] = "<<Hull_Sr[Py];
            qDebug()<<"inf_rays_Sr[Py] = "<<inf_rays_Sr[Py];
            qDebug()<<"vertex_x_r[start_vertex_r[inf_rays_Sr[Py]]] = "<<vertex_x_r[start_vertex_r[inf_rays_Sr[Py]]];
            qDebug()<<"vertex_y_r[start_vertex_r[inf_rays_Sr[Py]]] = "<<vertex_y_r[start_vertex_r[inf_rays_Sr[Py]]];
            qDebug()<<"prev_x = "<<prev_x<<"; prev_y = "<<prev_y;
            qDebug()<<"inter_x = "<<inter_x<<"; inter_y = "<<inter_y;
            */

            /********* Move the infinite vertex to (inter_x,inter_y) **********/
            if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_r[start_vertex_r[right_edge]],vertex_x_r[start_vertex_r[right_edge]]) < 0){
                //start_vertex of inf_rays_S[Py] is the infinite vertex
                qDebug()<<"start";
                this->x[start_vertex_r[right_edge] + num_v_l] = inter_x;
                this->y[start_vertex_r[right_edge] + num_v_l] = inter_y;
                this->w[start_vertex_r[right_edge] + num_v_l] = 1;//Become inner vertex
            }
            else if(cross_product(prev_x,prev_y,inter_x,inter_y,vertex_x_r[end_vertex_r[right_edge]],vertex_x_r[end_vertex_r[right_edge]]) < 0){
                qDebug()<<"end";
                this->x[end_vertex_r[right_edge] + num_v_l] = inter_x;
                this->y[end_vertex_r[right_edge] + num_v_l] = inter_y;
                this->w[end_vertex_r[right_edge] + num_v_l] = 1;//Become inner vertex
            }
            else{
                qDebug()<<"merge: Both start_vertex and end_vertex of right_ray are not on the right side of BS";

            }
            /******************************************************************/

            if(!top){
                this->num_edges++;

                *this->cw_successor.end()  = this->num_edges-1;

                this->right_polygon.push_back(Hull_Sl[Px]);
                this->left_polygon.push_back(Hull_Sr[Py] + num_p_l);

                this->start_vertex.push_back(*this->end_vertex.end());
                this->end_vertex.push_back(this->num_vertices-1);
                this->cw_predecessor.push_back(inf_rays_Sl[(Px+inf_rays_Sl.size()-1)%inf_rays_Sl.size()]);
                this->ccw_predecessor.push_back(this->num_edges-2);

                this->cw_successor.push_back(right_edge);
                this->ccw_successor.push_back(this->num_edges);

            }
            else{
                prev_x = (-1)*(BS->line.b*(inter_y+600) + BS->line.c)/BS->line.a;
                prev_y = inter_y+600;
            }

            //Py changes, counter clockwise
            Py = (Py+1) % Hull_Sr.size();

        }

        BS->x1 = prev_x;
        BS->y1 = prev_y;

        BS->x2 = inter_x;
        BS->y2 = inter_y;

        prev_x = inter_x;
        prev_y = inter_y;

        //qDebug()<<"inter_x = "<<inter_x<<" ; inter_y = "<<inter_y;
        if(top){
            //First BS i.e. BS of PaPb
            top = false;


            int edge_to_be_config_l, edge_to_be_config_r;
            int vertex_to_be_config;

            //Add a new vertex
            this->x.push_back(BS->x1);
            this->y.push_back(BS->y1);
            this->w.push_back(0);
            this->num_vertices++;

            vertex_to_be_config = this->num_vertices-1;

            /****** First time: Config the edges that are going to be with (BS->x1,BS->y1) ******/
            //left_ray first, find the point located on left side of BS
            //To find out the edge, we sholud check whether left_ray's direction to determine cw or ccw finding order
            if(this->w[start_vertex_l[inf_rays_Sl[(Pa + 1)%inf_rays_Sl.size()]]] == 0 ){
                //left_ray is pointing in to voronoi
                edge_to_be_config_l = this->ccw_predecessor[inf_rays_Sl[(Pa + 1)%inf_rays_Sl.size()]];
            }
            else{
                //left_ray is pointing out from voronoi
                edge_to_be_config_l = this->ccw_successor[inf_rays_Sl[(Pa + 1)%inf_rays_Sl.size()]];
            }
            //qDebug()<<"Pa = "<<Pa;
            //qDebug()<<"inf_rays_Sl[(Pa + 1)%inf_rays_Sl.size()] = "<<inf_rays_Sl[(Pa + 1)%inf_rays_Sl.size()];
            //qDebug()<<"edge_to_be_config_l = "<<edge_to_be_config_l;


            if(cross_product(BS->x1,BS->y1,BS->x2,BS->y2,this->x[this->start_vertex[edge_to_be_config_l]],this->y[start_vertex[edge_to_be_config_l]]) >= 0){
                //start_vertex of edge_to_be_config is the one to be config
                this->start_vertex[edge_to_be_config_l] = vertex_to_be_config;

            }
            else if(cross_product(BS->x1,BS->y1,BS->x2,BS->y2,this->x[end_vertex[edge_to_be_config_l]],this->y[end_vertex[edge_to_be_config_l]]) >= 0){
                //end_vertex of edge_to_be_config is the one to be config
                this->end_vertex[edge_to_be_config_l] = vertex_to_be_config;
            }
            else if(fabs(this->x[this->start_vertex[edge_to_be_config_l]] - this->x[this->end_vertex[edge_to_be_config_l]] < 1e-8) &&
                    fabs(this->y[this->start_vertex[edge_to_be_config_l]] - this->y[this->end_vertex[edge_to_be_config_l]] < 1e-8)){
                //When left voronoi has only 1 point
                this->start_vertex[edge_to_be_config_l] = vertex_to_be_config;
            }
            else{
                //For debug
                qDebug()<<"merge:Neither start_vertex nor end_vertex of edge_to_be_config_l located on left side of BS";
                qDebug()<<"BS->x1 = "<<BS->x1<<"; BS->y1 = "<<BS->y1<<"; BS->x2 = "<<BS->x2<<"; BS->y2 = "<<BS->y2;
                qDebug()<<"this->x[this->start_vertex[edge_to_be_config_l]] = "<<this->x[this->start_vertex[edge_to_be_config_l]];
                qDebug()<<"this->y[this->start_vertex[edge_to_be_config_l]] = "<<this->y[this->start_vertex[edge_to_be_config_l]];
                qDebug()<<"this->x[this->end_vertex[edge_to_be_config_l]] = "<<this->x[this->end_vertex[edge_to_be_config_l]];
                qDebug()<<"this->y[this->end_vertex[edge_to_be_config_l]] = "<<this->y[this->end_vertex[edge_to_be_config_l]];
            }
            //qDebug()<<"vertex_to_be_config = "<<vertex_to_be_config;

            //Then right_ray.
            if(this->w[start_vertex_r[inf_rays_Sr[Pb]] + num_v_l ] == 0 ){
                //right_ray is pointing in to voronoi
                edge_to_be_config_r = this->cw_predecessor[inf_rays_Sr[Pb] + num_e_l];
            }
            else{
                //right_ray is pointing out from voronoi
                edge_to_be_config_r = this->cw_successor[inf_rays_Sr[Pb]+ num_e_l];
            }

            //qDebug()<<"Pb = "<<Pb;
            //qDebug()<<"start_vertex_r[inf_rays_Sr[Pb]] + num_v_l = "<<start_vertex_r[inf_rays_Sr[Pb]] + num_v_l;
            //qDebug()<<"edge_to_be_config_r = "<<edge_to_be_config_r;
            //qDebug()<<"this->end_vertex[edge_to_be_config_r] = "<<this->end_vertex[edge_to_be_config_r];

            if(cross_product(BS->x1,BS->y1,BS->x2,BS->y2,this->x[this->end_vertex[edge_to_be_config_r]],this->y[this->end_vertex[edge_to_be_config_r]]) <= 0){
                            //end_vertex of right_ray is the one to be config
                this->end_vertex[edge_to_be_config_r] = vertex_to_be_config;
            }
            else if(cross_product(BS->x1,BS->y1,BS->x2,BS->y2,this->x[this->start_vertex[edge_to_be_config_r]],this->y[this->start_vertex[edge_to_be_config_r] ]) <= 0){
                //start_vertex of right_ray is the one to be config
              //qDebug()<<"Here";
                this->start_vertex[edge_to_be_config_r] = vertex_to_be_config;
            }
            else if(fabs(this->x[this->start_vertex[edge_to_be_config_r]] - this->x[this->end_vertex[edge_to_be_config_r]] < 1e-8) &&
                    fabs(this->y[this->start_vertex[edge_to_be_config_r]] - this->y[this->end_vertex[edge_to_be_config_r]] < 1e-8)){
                //When left voronoi has only 1 point
                this->end_vertex[edge_to_be_config_r] = vertex_to_be_config;
            }
            else{
                //For debug
                qDebug()<<"merge:Neither start_vertex nor end_vertex of right_ray located on right side of BS";
                qDebug()<<"BS->x1 = "<<BS->x1<<"; BS->y1 = "<<BS->y1<<"; BS->x2 = "<<BS->x2<<"; BS->y2 = "<<BS->y2;
                qDebug()<<"this->x[this->start_vertex[edge_to_be_config_r]] = "<<this->x[this->start_vertex[edge_to_be_config_r]];
                qDebug()<<"this->y[this->start_vertex[edge_to_be_config_r]] = "<<this->y[this->start_vertex[edge_to_be_config_r]];
                qDebug()<<"this->x[this->end_vertex[edge_to_be_config_r]] = "<<this->x[this->end_vertex[edge_to_be_config_r]];
                qDebug()<<"this->y[this->end_vertex[edge_to_be_config_r]] = "<<this->y[this->end_vertex[edge_to_be_config_r]];
            }
            /**************************************************************************/


            //config of first BS

            this->num_edges++;

            this->right_polygon.push_back(Hull_Sl[Pa]);
            this->left_polygon.push_back(Hull_Sr[Pb] + num_p_l);

            this->start_vertex.push_back(vertex_to_be_config);
            this->end_vertex.push_back(this->num_vertices);

            this->cw_predecessor.push_back(edge_to_be_config_l);
            this->ccw_predecessor.push_back(edge_to_be_config_r);

            if(have_inter_l && have_inter_r && fabs(inter_l_x-inter_r_x) < 1e-8 && fabs(inter_l_y-inter_r_y) < 1e-8){
                this->cw_successor.push_back(inf_rays_Sr[(Pb+1)%inf_rays_Sr.size()] + num_e_l);
                this->ccw_successor.push_back(inf_rays_Sl[Pa]);
            }
            else if(!have_inter_r || inter_l_y >= inter_r_y){
                //Left_ray is (inter_x,inter_y)
                this->cw_successor.push_back(this->num_edges);
                this->ccw_successor.push_back(inf_rays_Sl[Pa]);
            }
            else{
                //Right_ray is (inter_x,inter_y)
                this->cw_successor.push_back(inf_rays_Sr[(Pb+1)%inf_rays_Sr.size()] + num_e_l);
                this->ccw_successor.push_back(this->num_edges);
            }
            /*
            qDebug()<<"*(this->right_polygon.end()-1) = "<<*(this->right_polygon.end()-1);
            qDebug()<<"*(this->left_polygon.end()-1) = "<<*(this->left_polygon.end()-1);
            qDebug()<<"*(this->start_vertex.end()-1) = "<<*(this->start_vertex.end()-1);
            qDebug()<<"*(this->end_vertex.end()-1) = "<<*(this->end_vertex.end()-1);

            qDebug()<<"*(this->cw_predecessor.end()-1) = "<<*(this->cw_predecessor.end()-1);
            qDebug()<<"*(this->ccw_predecessor.end()-1) = "<<*(this->ccw_predecessor.end()-1);
            qDebug()<<"*(this->cw_successor.end()-1) = "<<*(this->cw_successor.end()-1);
            qDebug()<<"*(this->ccw_successor.end()-1) = "<<*(this->ccw_successor.end()-1);
            */
        }

        //HP = HP ∪ BS
        HP.push_back(*BS);

    }while(Px != Pc || Py != Pd);
/*******************************       End of while      ************************************/

    //Config last BS
    BS = new bisector();
    //Find the line coefficients : ax+by+c=0 for BS
    x1 = g_x_l[Hull_Sl[Pc]];     y1 = g_y_l[Hull_Sl[Pc]];
    x2 = g_x_r[Hull_Sr[Pd]];     y2 = g_y_r[Hull_Sr[Pd]];
    BS->line = WingedEdge::findPerpendicularBisector(x1,y1,x2,y2);

    if(top){/*?????????*/
        //deal with the situation that Pa==Pc, Pb==Pd
        prev_y = 600;
        prev_x = (-1)*(BS->line.b*(prev_y) + BS->line.c)/BS->line.a;
    }

    BS->x1 = prev_x;
    BS->y1 = prev_y;
    BS->x2 = (-1)*(BS->line.b*(prev_y-600)+ BS->line.c)/BS->line.a;
    BS->y2 = prev_y-600;
    //qDebug()<<"BS->line.a = "<<BS->line.a<<"; BS->line.b = "<<BS->line.b<<"; BS->line.c = "<<BS->line.c;
    //qDebug()<<"BS->x1 = "<<BS->x1<<"; BS->y1 = "<<BS->y1<<"; BS->x2 = "<<BS->x2<<"; BS->y2 = "<<BS->y2;

    //HP = HP ∪ BS
    HP.push_back(*BS);

    /* Step 5: Discard the edges of VD(SL) which extend to
     * the right of HP and discard the edges of VD(SR) which
     * extend to the left of HP.
     * The resulting graph is the Voronoi diagram of S = SL ∪ SR
     */

    /****** Last time: Config the edges that are going to be with (BS->x2,BS->y2) ******/
    //inf_rays_Sr[Pd] first, find the point located on left side of BS

    int edge_to_be_config_l, edge_to_be_config_r;
    int vertex_to_be_config;


    //Add a new vertex
    this->x.push_back(BS->x2);
    this->y.push_back(BS->y2);
    this->w.push_back(0);
    this->num_vertices++;

    vertex_to_be_config = this->num_vertices-1;

    if(this->w[start_vertex_r[inf_rays_Sr[(Pd+1)%inf_rays_Sr.size()]] + num_v_l ] == 0 ){
        //inf_rays_Sr[Pd] is pointing in to voronoi
        edge_to_be_config_r = this->ccw_predecessor[inf_rays_Sr[(Pd+1)%inf_rays_Sr.size()]+ num_e_l];
    }
    else{
        //inf_rays_Sl[Pd] is pointing out from voronoi
        edge_to_be_config_r = this->ccw_successor[inf_rays_Sr[(Pd+1)%inf_rays_Sr.size()]+ num_e_l];
    }

    if(cross_product(BS->x2,BS->y2,BS->x1,BS->y1,this->x[this->start_vertex[edge_to_be_config_r]],this->y[this->start_vertex[edge_to_be_config_r] ]) >= 0){
        //start_vertex of inf_rays_Sr[Pd] is the one to be config

        this->start_vertex[edge_to_be_config_r] = vertex_to_be_config;
    }
    else if(cross_product(BS->x2,BS->y2,BS->x1,BS->y1,this->x[this->end_vertex[edge_to_be_config_r]],this->y[this->end_vertex[edge_to_be_config_r]]) >= 0){
        //end_vertex of inf_rays_Sr[Pd] is the one to be config
        this->end_vertex[edge_to_be_config_r] = vertex_to_be_config;
    }
    else if(fabs(this->x[this->start_vertex[edge_to_be_config_r]] - this->x[this->end_vertex[edge_to_be_config_r]] < 1e-8) &&
            fabs(this->y[this->start_vertex[edge_to_be_config_r]] - this->y[this->end_vertex[edge_to_be_config_r]] < 1e-8)){
        //When left voronoi has only 1 point
        this->start_vertex[edge_to_be_config_r] = vertex_to_be_config;
    }
    else{
        //For debug
        qDebug()<<"merge:Neither start_vertex nor end_vertex of inf_rays_Sl[Pd] located on left side of BS";
        qDebug()<<"BS->x1 = "<<BS->x1<<"; BS->y1 = "<<BS->y1<<"; BS->x2 = "<<BS->x2<<"; BS->y2 = "<<BS->y2;
        qDebug()<<"this->x[this->start_vertex[edge_to_be_config_r]] = "<<this->x[this->start_vertex[edge_to_be_config_r]];
        qDebug()<<"this->y[this->start_vertex[edge_to_be_config_r]] = "<<this->y[this->start_vertex[edge_to_be_config_r]];
        qDebug()<<"this->x[this->end_vertex[edge_to_be_config_r]] = "<<this->x[this->end_vertex[edge_to_be_config_r]];
        qDebug()<<"this->y[this->end_vertex[edge_to_be_config_r]] = "<<this->y[this->end_vertex[edge_to_be_config_r]];
    }
/*
    qDebug()<<"Pd = "<<Pd;
    qDebug()<<"start_vertex_r[inf_rays_Sr[(Pd+1)%inf_rays_Sr.size()]] + num_v_l = "<<start_vertex_r[inf_rays_Sr[(Pd+1)%inf_rays_Sr.size()]] + num_v_l  ;
    qDebug()<<"edge_to_be_config_r = "<<edge_to_be_config_r;
    qDebug()<<"this->start_vertex[edge_to_be_config_r] = "<<this->start_vertex[edge_to_be_config_r];
    qDebug()<<"this->end_vertex[edge_to_be_config_r] = "<<this->end_vertex[edge_to_be_config_r];
*/

    if(this->w[start_vertex_l[inf_rays_Sl[Pc]]] == 0 ){
        edge_to_be_config_l = this->cw_predecessor[inf_rays_Sl[Pc]%inf_rays_Sl.size()];
    }
    else{
        //inf_rays_Sl[Pc] is pointing out from voronoi
        edge_to_be_config_l = this->cw_successor[inf_rays_Sl[Pc]%inf_rays_Sl.size()];
    }

  //qDebug()<<"edge_to_be_config_l = "<<edge_to_be_config_l;

    if(cross_product(BS->x2,BS->y2,BS->x1,BS->y1,this->x[this->start_vertex[edge_to_be_config_l]],this->y[start_vertex[edge_to_be_config_l]]) <= 0){
        //start_vertex of inf_rays_Sl[Pc] is the one to be config

        this->start_vertex[edge_to_be_config_l] = vertex_to_be_config;
    }
    else if(cross_product(BS->x2,BS->y2,BS->x1,BS->y1,this->x[end_vertex[edge_to_be_config_l]],this->y[end_vertex[edge_to_be_config_l]]) <= 0){
        //end_vertex of inf_rays_Sl[Pc] is the one to be config

        this->end_vertex[edge_to_be_config_l] = vertex_to_be_config;
    }    
    else if(fabs(this->x[this->start_vertex[edge_to_be_config_l]] - this->x[this->end_vertex[edge_to_be_config_l]] < 1e-8) &&
            fabs(this->y[this->start_vertex[edge_to_be_config_l]] - this->y[this->end_vertex[edge_to_be_config_l]] < 1e-8)){
        //When left voronoi has only 1 point
        this->end_vertex[edge_to_be_config_l] = vertex_to_be_config;
    }
    else{
        //For debug
        qDebug()<<"merge:Neither start_vertex nor end_vertex of inf_rays_Sl[Pc] located on right side of BS";
        qDebug()<<"BS->x1 = "<<BS->x1<<"; BS->y1 = "<<BS->y1<<"; BS->x2 = "<<BS->x2<<"; BS->y2 = "<<BS->y2;
        qDebug()<<"this->x[this->start_vertex[edge_to_be_config_l]] = "<<this->x[this->start_vertex[edge_to_be_config_l]];
        qDebug()<<"this->y[this->start_vertex[edge_to_be_config_l]] = "<<this->y[this->start_vertex[edge_to_be_config_l]];
        qDebug()<<"this->x[this->end_vertex[edge_to_be_config_l]] = "<<this->x[this->end_vertex[edge_to_be_config_l]];
        qDebug()<<"this->y[this->end_vertex[edge_to_be_config_l]] = "<<this->y[this->end_vertex[edge_to_be_config_l]];
    }

    //Then inf_rays_Sr[Pd].

    /**************************************************************************/

    //config of last BS
    this->num_edges++;

    this->right_polygon.push_back(Hull_Sl[Pc]);
    this->left_polygon.push_back(Hull_Sr[Pd] + num_p_l);

    this->start_vertex.push_back(*(this->end_vertex.end()-1));
    this->end_vertex.push_back(vertex_to_be_config);

    if(!have_inter_r || inter_l_y >= inter_r_y){
        this->cw_predecessor.push_back(inf_rays_Sl[(Pc+1)%inf_rays_Sl.size()]);
        this->ccw_predecessor.push_back(this->num_edges-2);
    }
    else{
        this->cw_predecessor.push_back(this->num_edges-2);
        this->ccw_predecessor.push_back(inf_rays_Sr[Pd] + num_e_l);
    }

    this->cw_successor.push_back(edge_to_be_config_r);
    this->ccw_successor.push_back(edge_to_be_config_l);


    this->output_all_data_structures();
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

void WingedEdge::constructConvexHull(vector<int> &Hull, vector<int> &infinite_rays)
{
    //O(n)
    vector<int> HULL,inf_rays;
    //HULL is the verctor of counter clockwise order of conver hull points.
    //inf_rays is the vector of counter clockwise order of infinite rays used when finding convex hull.


    if(this->getNumPolygons() == 1 ){
        HULL.push_back(0);
        inf_rays.push_back(0);
    }
    else if(this->getNumPolygons() == 2){
        HULL.push_back(0);
        HULL.push_back(1);
        inf_rays.push_back(1);
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

    Hull = HULL;
    infinite_rays = inf_rays;
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
    //qDebug()<<"to_be_moved = "<<to_be_moved;

    while(!fixed_left && !fixed_right){
        if(to_be_moved == 0 ){
            //Moved current_left counter clockwise
            int next_point = (current_left+1) % Hull_Sl.size();

            if(fixed_left || next_point == current_left){
                //left is fixed, deal with right
                fixed_left = true;
                to_be_moved = 1;
                continue;
            }

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

            //Moved current_right clockwise
            int next_point = (current_right + Hull_Sr.size() -1) % Hull_Sr.size();
            if(fixed_right || next_point==current_right){
                //right is fixed, deal with left
                fixed_right = true;
                to_be_moved = 0;
                continue;
            }
            //qDebug()<<"current_right = "<<current_right<<"; next_point = "<<next_point;
            //Calculate cross product of current_left=>current_right and current_left=>next_point
            //to determine if the outter tangent point is reached.
            double x_0 = g_x_l[Hull_Sl[current_left]];
            double y_0 = g_y_l[Hull_Sl[current_left]];

            double x_1 = g_x_r[Hull_Sr[current_right]];
            double y_1 = g_y_r[Hull_Sr[current_right]];


            double x_2 = g_x_r[Hull_Sr[next_point]];
            double y_2 = g_y_r[Hull_Sr[next_point]];

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
            //Moved current_left clockwise
            int next_point = (current_left+ Hull_Sl.size() -1) % Hull_Sl.size();
            if(fixed_left || next_point==current_left){
                //left is fixed, deal with right
                fixed_left = true;
                to_be_moved = 1;
                continue;
            }

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

            //Moved current_right counter clockwise
            int next_point = (current_right +1) % Hull_Sl.size();

            if(fixed_right || next_point == current_right){
                //right is fixed, deal with left
                fixed_right = true;
                to_be_moved = 0;
                continue;
            }

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

void WingedEdge::output_all_data_structures()
{
    qDebug()<<"num_polygons = "<<num_polygons;
    qDebug()<<"num_vertices = "<<num_vertices;
    qDebug()<<"num_edges = "<<num_edges;

    QString str;
    qDebug("g_x:\t");
    //generating points' coordinates
    for(unsigned long i=0;i<g_x.size();i++){
        str += " ";
        str += QString::number(g_x[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("g_y:\t");
    for(unsigned long i=0;i<g_y.size();i++){
        str += " ";
        str += QString::number(g_y[i]);
    }
    qDebug()<<str;
    str.clear();


    //array for edges
    qDebug()<<"";
    qDebug("right_polygon:\t");
    for(unsigned long i=0;i<right_polygon.size();i++){
        str += " ";
        str += QString::number(right_polygon[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("left_polygon:\t");
    for(unsigned long i=0;i<left_polygon.size();i++){
        str += " ";
        str += QString::number(left_polygon[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("start_vertex:\t");
    for(unsigned long i=0;i<start_vertex.size();i++){
        str += " ";
        str += QString::number(start_vertex[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("end_vertex:\t");
    for(unsigned long i=0;i<end_vertex.size();i++){
        str += " ";
        str += QString::number(end_vertex[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("cw_predecessor:\t");
    for(unsigned long i=0;i<cw_predecessor.size();i++){
        str += " ";
        str += QString::number(cw_predecessor[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("ccw_predecessor:\t");
    for(unsigned long i=0;i<ccw_predecessor.size();i++){
        str += " ";
        str += QString::number(ccw_predecessor[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("cw_successor:\t");
    for(unsigned long i=0;i<cw_successor.size();i++){
        str += " ";
        str += QString::number(cw_successor[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("ccw_successor:\t");
    for(unsigned long i=0;i<ccw_successor.size();i++){
        str += " ";
        str += QString::number(ccw_successor[i]);
    }
    qDebug()<<str;
    str.clear();

    //array for polygons
    qDebug("edge_around_polygon:\t");
    for(unsigned long i=0;i<edge_around_polygon.size();i++){
        str += " ";
        str += QString::number(edge_around_polygon[i]);
    }
    qDebug()<<str;
    str.clear();

    //array for vertices
    qDebug("edge_around_vertex:\t");
    for(unsigned long i=0;i<edge_around_vertex.size();i++){
        str += " ";
        str += QString::number(edge_around_vertex[i]);
    }
    qDebug()<<str;
    str.clear();


    qDebug("w:\t");
    for(unsigned long i=0;i<w.size();i++){
        str += " ";
        str += QString::number(w[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("x:\t");
    for(unsigned long i=0;i<x.size();i++){
        str += " ";
        str += QString::number(x[i]);
    }
    qDebug()<<str;
    str.clear();
    qDebug("y:\t");
    for(unsigned long i=0;i<y.size();i++){
        str += " ";
        str += QString::number(y[i]);
    }
    qDebug()<<str;
    str.clear();
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

        result.x1 = (x_1+x_2)/2;
        result.y1 = 0;
        result.x2 = (x_1+x_2)/2;
        result.y2 = 600;
    }
    else{
        result.a = (x_1-x_2)/(y_1-y_2);
        result.b = 1;
        result.c = (x_2*x_2+y_2*y_2-x_1*x_1-y_1*y_1)/(2*(y_1-y_2));

        result.x1 = 0;
        result.y1 = (-1)*result.c;
        result.x2 = 600;
        result.y2 = (-1)*(result.a*600+result.c);
    }

    return result;

}

void WingedEdge::combineWingedEdges(WingedEdge S_l, WingedEdge S_r)
{
    this->num_polygons = S_l.getNumPolygons() + S_r.getNumPolygons() + 1;
    this->num_vertices = S_l.getNum_vertices() + S_r.getNum_vertices();
    this->num_edges = S_l.getNum_edges() + S_r.getNum_edges();

    //Concatenate data structures

    //generating points' data structures
    vector<double> g_x_r = S_r.get_g_x(), g_y_r = S_r.get_g_y();
    this->g_x = S_l.get_g_x();
    this->g_y = S_l.get_g_y();

    for(unsigned long i=0; i<g_x_r.size();i++){
        this->g_x.push_back(g_x_r[i]);
        this->g_y.push_back(g_y_r[i]);
    }

    //arrays for edges
    vector<int> right_polygon_r = S_r.get_right_polygon();
    vector<int> left_polygon_r = S_r.get_left_polygon();
    vector<int> start_vertex_r = S_r.get_start_vertex();
    vector<int> end_vertex_r = S_r.get_end_vertex();
    vector<int> cw_predecessor_r = S_r.getCw_predecessor();
    vector<int> ccw_predecessor_r = S_r.getCcw_predecessor();
    vector<int> cw_successor_r = S_r.getCw_successor();
    vector<int> ccw_successor_r = S_r.getCcw_successor();


    this->right_polygon     = S_l.get_right_polygon();
    this->left_polygon      = S_l.get_left_polygon();
    this->start_vertex      = S_l.get_start_vertex();
    this->end_vertex        = S_l.get_end_vertex();
    this->cw_predecessor    = S_l.getCw_predecessor();
    this->ccw_predecessor   = S_l.getCcw_predecessor();
    this->cw_successor      = S_l.getCw_successor();
    this->ccw_successor     = S_l.getCcw_successor();

    for(unsigned long i=0,p_num = S_l.getNumPolygons();
                                                        i < S_l.getNum_edges() ; i++){
        //Adjust the numbering of polygon at INF
        if(this->right_polygon[i] == p_num){
            this->right_polygon[i] = this->getNumPolygons();
        }
        if(this->left_polygon[i] == p_num){
            this->left_polygon[i] = this->getNumPolygons();
        }
    }

    //Adjust S_r edges' numbering
    for(unsigned long i=0,p_num = S_l.getNumPolygons(), e_num = S_l.getNum_edges(), v_num = S_l.getNum_vertices() ;
                                                        i < S_r.getNum_edges() ; i++){
        //Adjust the numbering of polygon at INF
        this->right_polygon.push_back(right_polygon_r[i] + p_num);
        this->left_polygon.push_back(left_polygon_r[i] + p_num);

        this->start_vertex.push_back(start_vertex_r[i] + v_num);
        this->end_vertex.push_back(end_vertex_r[i] + v_num);

        this->cw_predecessor.push_back(cw_predecessor_r[i] + e_num );
        this->ccw_predecessor.push_back(ccw_predecessor_r[i] + e_num );
        this->cw_successor.push_back(cw_successor_r[i] + e_num ) ;
        this->ccw_successor.push_back(ccw_successor_r[i] + e_num );
    }

    /*
     * temperorily not used
    vector<int> edge_around_polygon_r = S_r.getEdge_around_polygon();
    this->edge_around_polygon = S_l.getEdge_around_polygon();

    for(unsigned long i=0, e_num = S_l.getNum_edges(); i < S_r.getNumPolygons() ; i++){
        this->edge_around_polygon.push_back(edge_around_polygon_r[i] + e_num);
    }
    vector<int> edge_around_vertex_r = S_r.getEdge_around_vertex();
    this->edge_around_vertex = S_l.getEdge_around_vertex();

    for(unsigned long i=0, e_num = S_l.getNum_edges(); i < S_r.getNum_vertices() ; i++){
        this->edge_around_vertex.push_back(edge_around_vertex_r[i] + e_num);
    }
    */

    vector<int> w_r =  S_r.get_w();
    vector<double> x_r = S_r.get_x();
    vector<double> y_r = S_r.get_y();

    this->w = S_l.get_w();
    this->x = S_l.get_x();
    this->y = S_l.get_y();

    for(unsigned long i=0; i<S_r.getNum_vertices(); i++){
        this->w.push_back(w_r[i]);
        this->x.push_back(x_r[i]);
        this->y.push_back(y_r[i]);
    }

}

vector<int> WingedEdge::getEdge_around_vertex() const
{
    return edge_around_vertex;
}

void WingedEdge::setEdge_around_vertex(const vector<int> &value)
{
    edge_around_vertex = value;
}

vector<int> WingedEdge::getEdge_around_polygon() const
{
    return edge_around_polygon;
}

void WingedEdge::setEdge_around_polygon(const vector<int> &value)
{
    edge_around_polygon = value;
}

int WingedEdge::getNum_vertices() const
{
    return num_vertices;
}

void WingedEdge::setNum_vertices(int value)
{
    num_vertices = value;
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
