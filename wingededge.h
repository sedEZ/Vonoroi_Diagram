#ifndef WINGEDEDGE_H
#define WINGEDEDGE_H
#include <vector>
#include <algorithm>
#include <math.h>
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

    void constructTOnePointsVoronoi();
    void constructTwoPointsVoronoi();

    void divide(WingedEdge* W_l,WingedEdge* W_r);

    //Todo
    void merge(WingedEdge S_l,WingedEdge S_r);

    //Temporary version for first demo
    double findMedianLine();

    int getNumPolygons();

    bool threePointsVertical();

    void setWaitingMerge(bool i);
    bool IsWaitingMerge();

    void changeArraysForEdges(int resize_size);
    void getOrdinaryEdges(vector<int>&);

    void getOridinaryEdgesCoordinates(int i, double& x_1,double& x_2,double& y_1,double& y_2);



//Variables
private:
    int num_polygons;
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

    //For storing : Check if the WingedEdge object is waiting merging.
    bool waiting_merge;
};

#endif // WINGEDEDGE_H
