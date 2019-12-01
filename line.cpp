#include "line.h"
#include<QDebug>
using namespace std;
Line::Line()
{
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->x1=0;
    this->y1=0;
    this->x2=0;
    this->y2=0;
}

Line::Line(double x1, double y1, double x2, double y2)
{
    //Use (x1,y1) and (x2,y2) to construct a line ax + by + c = 0

    this->x1=x1;
    this->y1=y1;
    this->x2=x2;
    this->y2=y2;

    if(fabs(x1-x2) < 1e-8){
        //Vertical line : x = x1;
        this->a = 1;
        this->b = 0;
        this->c = (-1)*x1;
    }
    else{
        this->a = (y1-y2)/(x2-x1);
        this->b = 1;
        this->c = (-1)*(a*x1+b*y1);
    }
}

bool Line::find_intersect(Line m, Line n, double &inter_x, double &inter_y)
{
    //Use Cramer's rule to fine intersection

    if(fabs(m.x1-m.x2)<1e-8 && fabs(m.y1-m.y2)<1e-8){
        return false;//Same point;
    }
    else if(fabs(n.x1-n.x2)<1e-8 && fabs(n.y1-n.y2)<1e-8){
        return false;//Same point;
    }

    double determinant = m.a*n.b - m.b*n.a;
    //qDebug()<<"determinant = "<<determinant;

    if(fabs(determinant) < 1e-8)//no solution
        return false;

    inter_x = (n.c*m.b - m.c*n.b) / determinant;
    inter_y = (n.a*m.c - m.a*n.c) / determinant;
/*
    if(m.w1 || m.w2){
        double side_of_point_1 = Line::cross_product(n.x1,n.y1,n.x2,n.y2,m.x1,m.y1);
        double side_of_point_2 = Line::cross_product(n.x1,n.y1,n.x2,n.y2,m.x2,m.y2);

        //same side => intersrction is not on m
        if(side_of_point_1*side_of_point_2 > 0){
            return false;
        }
    }

    if(n.w1 || n.w2){
        double side_of_point_1 = Line::cross_product(m.x1,m.y1,m.x2,m.y2,n.x1,n.y1);
        double side_of_point_2 = Line::cross_product(m.x1,m.y1,m.x2,m.y2,n.x2,n.y2);

        //same side => intersrction is not on n
        if(side_of_point_1*side_of_point_2 > 0){
            return false;
        }
    }
*/
    return true;
}

double Line::cross_product(double x_0, double y_0, double x_1, double y_1, double x_2, double y_2)
{

    return (x_1-x_0)*(y_2-y_0) - (x_2-x_0)*(y_1-y_0);

}
