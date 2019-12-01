#ifndef LINE_H
#define LINE_H

#include <algorithm>
#include <math.h>


class Line
{
public:
    Line();
    Line(double x1, double y1,double x2, double y2);

    //find_intersect return true if 2 lines have intersection point
    //                      false if 2 lines intersect at infinity
    static bool find_intersect(Line m, Line n, double &inter_x, double &inter_y );


//Variables
    //ax + by + c = 0
    double a,b,c;

    //the value of points' coordinates when setting the line
    double x1,y1,x2,y2;

    int w1,w2;//if w is 0, the point is at infinity i.e. the line is a ray to (x,y)
};

#endif // LINE_H
