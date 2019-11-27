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
};

#endif // LINE_H
