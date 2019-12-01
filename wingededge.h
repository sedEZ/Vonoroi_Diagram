#ifndef WINGEDEDGE_H
#define WINGEDEDGE_H
#include "line.h"
#include <vector>
using namespace std;

/*
 * WingedEdge is a data structure representing a vonoroi diagram.
 * For details, please reference :
 *  Atsuyuki Okabe,    Barry Boots,    Kokichi Sugihara,   Sung Nok Chiu
 *  Spatial Tessellations : Concepts and Applications of Voronoi Diagrams
 *      QA278.2 O36 1992
 *      Sec. 4.2 Data Structure for Representing a Voronoi Diagram, pp.215~pp.223
 *
 * #To get number of ordinary polygons, use getNumPloygons instead of access variable num_polygons
 */
class WingedEdge
{
public:
    WingedEdge();
    WingedEdge(vector<double> p_x, vector<double>p_y);

    void constructOnePointVoronoi();
    void constructTwoPointsVoronoi();
    void constructThreePointsVoronoi();

    double cross_product(double x_0,double y_0,double x_1,double y_1,double x_2,double y_2);

    void divide(WingedEdge &W_l,WingedEdge &W_r);
    //Todo
    void merge(WingedEdge S_l,WingedEdge S_r);

    double find_k_th(vector<double> S,unsigned long k);//Find k-th element int S using Prune-and-Search
    void constructConvexHull(vector<int> &Hull, vector<int> &infinite_rays);//Construct convex hull of this WingedEdge ds &
                                        //Return the generating points numbers in counter clockwise order

    //Find outter tangent line
    void find_outter_tangent_top(int& Pa, int& Pb, WingedEdge Sl,vector<int> Hull_Sl,WingedEdge Sr, vector<int> Hull_Sr);
    void find_outter_tangent_bot(int& Pc, int& Pd, WingedEdge Sl,vector<int> Hull_Sl,WingedEdge Sr, vector<int> Hull_Sr);

    void erase_edges_by_HP(vector<struct bisector> HP, WingedEdge Sl,WingedEdge Sr);

    int getNumPolygons();
    bool threePointsSameLine();

    void setWaitingMerge(bool i);
    bool IsWaitingMerge();

    void changeArraysForEdges(int resize_size);
    void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);

    void output_all_data_structures();

    vector<int> getOrdinaryEdges();

    void getOridinaryEdgesCoordinates(int i, double& x_1,double& x_2,double& y_1,double& y_2);

    // Used in 2 & 3 points voronoi version.
    // May want to modified constructTwoPointsVoronoi() and constructThreePointsVoronoi() using another version below.
    void findPerpendicularBisector(double x_1,double y_1,double x_2, double y_2,double& m, double& b);

    //Used in general version.
    static Line findPerpendicularBisector(double x_1,double y_1,double x_2, double y_2);


//Variables
    vector<double> get_g_x();
    vector<double> get_g_y();
    vector<int> get_w();
    vector<double> get_x();
    vector<double> get_y();
    vector<int> get_right_polygon();
    vector<int> get_left_polygon();
    vector<int> get_start_vertex();
    vector<int> get_end_vertex();
    vector<int> getCw_predecessor() const;
    void setCw_predecessor(const vector<int> &value);

    vector<int> getCcw_predecessor() const;
    void setCcw_predecessor(const vector<int> &value);

    vector<int> getCw_successor() const;
    void setCw_successor(const vector<int> &value);

    vector<int> getCcw_successor() const;
    void setCcw_successor(const vector<int> &value);

    int getNum_edges() const;
    void setNum_edges(int value);

    int getNum_vertices() const;
    void setNum_vertices(int value);

    vector<int> getEdge_around_polygon() const;
    void setEdge_around_polygon(const vector<int> &value);

    vector<int> getEdge_around_vertex() const;
    void setEdge_around_vertex(const vector<int> &value);

private:
    //Combine 2 WingedEdge ds as this
    void combineWingedEdges(WingedEdge S_l, WingedEdge S_r );

    int num_polygons;//Last polygon is the polygon at INFINITY
    int num_vertices;
    int num_edges;

    //generating points' coordinates
    vector<double> g_x;
    vector<double> g_y;

    //array for edges
    vector<int> right_polygon;
    vector<int> left_polygon;
    vector<int> start_vertex;
    vector<int> end_vertex;
    vector<int> cw_predecessor;
    vector<int> ccw_predecessor;
    vector<int> cw_successor;
    vector<int> ccw_successor;

    //array for polygons
    vector<int> edge_around_polygon;

    //array for vertices
    vector<int> edge_around_vertex; //Record an ordinary edge of vertex
    vector<int> w;//w[i] = 1 ,if i is an ordinary point
                  //w[i] = 0 ,if i is a point at infinity
    vector<double> x;
    vector<double> y;

    vector<struct bisector> HP;
    //For storing : Check if the WingedEdge object is waiting merging.
    bool waiting_merge;
};

struct g_point{
    double x,y;
};

struct bisector{
    //DS for perpendicular pisector which belongs to hyperplane HP

    //[(x1,y1), (x2,y2)] of line l
    double x1,x2,y1,y2;
    Line line;
};

bool compare_g_point(const g_point a, const g_point b);

#endif // WINGEDEDGE_H
