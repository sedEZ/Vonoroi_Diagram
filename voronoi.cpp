#include "voronoi.h"
#include <QDebug>

Voronoi::Voronoi()
{

}
//Constructor
Voronoi::Voronoi(vector<double> p_x, vector<double> p_y)
{
    //Create original generating points & push it into stack
    WingedEdge generating_points(p_x,p_y);
    s_stack.push(generating_points);
    num_polygons = p_x.size();

}

Voronoi::~Voronoi()
{
    while(!s_stack.empty())
        s_stack.pop();
    while(!done_stack.empty())
        s_stack.pop();
    delete this;
}

WingedEdge Voronoi::runOneStep()
{
    if(s_stack.empty()){
        qDebug()<<"Vonoroi : Stack is empty";
        exit(-1);
    }

    //If the next stack is not waiting for merging, then it is to be divide
    //Loop until a waiting-for-merging WingedEdge is to be run.
    while(!this->s_stack.empty() && !this->s_stack.top().IsWaitingMerge()){
        if(this->stack_leak()){
            qDebug()<<"Stack leak";
            exit(-1);
        }
        //Get top
        WingedEdge current_divide = s_stack.top(); s_stack.pop();

        if(current_divide.getNumPolygons() == 1){
            //If current_divide has just 1 point, returns
            current_divide.constructOnePointVoronoi();
            done_stack.push(current_divide);
            return current_divide;
        }
        else if(current_divide.getNumPolygons() == 2){
            //If current_divide has 2 points, find the vertical line & update the WingedEdge data structure
            current_divide.constructTwoPointsVoronoi();
            done_stack.push(current_divide);
            return current_divide;
        }
        else if(current_divide.getNumPolygons() == 3){
            //If current_divide has 3 points, construct directly & update the WingedEdge data structure
            current_divide.constructThreePointsVoronoi();
            done_stack.push(current_divide);
            return current_divide;
        }
        else{
            WingedEdge W_l,W_r;
            current_divide.divide(W_l,W_r);
            current_divide.setWaitingMerge(true);
            s_stack.push(current_divide);
            s_stack.push(W_l);
            s_stack.push(W_r);
        }
    }

    //In first demo, this step should be run only when 3 points
    WingedEdge current_merge = s_stack.top();s_stack.pop();
    //Assert if done_stack has more than 2 elements
    if(done_stack.size()<2){
        qDebug()<<"Merge error : done_stack has less than 2 elements.";
        exit(-1);
    }
    //Left WingedEdge
    WingedEdge S_l = done_stack.top(); done_stack.pop();
    //Right WingedEdge
    WingedEdge S_r = done_stack.top(); done_stack.pop();

    current_merge.merge(S_l,S_r);

    //Done merge, put current_merge into done_stack

    this->done_stack.push(current_merge);
    return current_merge;
}

bool Voronoi::empty()
{
    return this->s_stack.empty();
}

bool Voronoi::stack_leak()
{
    return this->s_stack.size() > this->num_polygons+1;
}

int Voronoi::getNum_polygons() const
{
    return num_polygons;
}

void Voronoi::setNum_polygons(int value)
{
    num_polygons = value;
}

WingedEdge Voronoi::getResult()
{
    return this->done_stack.top();
}

