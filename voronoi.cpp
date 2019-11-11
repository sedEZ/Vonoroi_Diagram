#include "voronoi.h"
#include <QDebug>

Voronoi::Voronoi()
{

}
//Constructor
Voronoi::Voronoi(vector<double> p_x, vector<double>p_y)
{
    //Create original generating points & push it into stack
    WingedEdge generating_points(p_x,p_y);
    s_stack.push(generating_points);
}


//Todo: finish the function implementations
WingedEdge Voronoi::runOneStep()
{
    if(s_stack.empty()){
        qDebug()<<"Vonoroi : Stack is empty";
        exit(-1);
    }

    //If the next stack is not waiting for merging, then it is to be divide
    //Loop until a waiting-for-merging WingedEdge is to be run.
    while(!this->s_stack.top().IsWaitingMerge()){
        //Get top
        WingedEdge current_divide = s_stack.top(); s_stack.pop();
        if(current_divide.getNumPolygons() == 1){
            //If current_divide has just 1 point, returns
            done_stack.push(current_divide);
        }
        else if(current_divide.getNumPolygons() == 2){
            //If current_divide has 2 points, find the vertical line & update the WingedEdge data structure
            current_divide.constructTwoPointsVoronoi();
            done_stack.push(current_divide);
        }
        else{
            //m = find_median_line(current_divide) , m is the x-coordinate of median line
            //W_l = {(i,current_divide.g_x[i],current_divide.g_y[i]) | current_divide.g[i] < m}
            //W_r = {(i,current_divide.g_x[i],current_divide.g_y[i]) | current_divide.g[i] >=m}

            //current_divide.setWaitingMerge(true);
            //s_stack.push(current_divide);
            //s_stack.push(WingeEdge(W_l));
            //s_stack.push(WingeEdge(W_r));
        }
    }

    WingedEdge current_merge = this->s_stack.top(); s_stack.pop();

    //Assert if done_stack has more than 2 elements
    if(done_stack.size()<2){
        qDebug()<<"Merge error : done_stack has less than 2 elements.";
        exit(-1);
    }

    //Left WingedEdge
    WingedEdge S_l = done_stack.top(); done_stack.pop();
    //Right WingedEdge
    WingedEdge S_r = done_stack.top(); done_stack.pop();

    //current_merge.MERGE(S_l,S_r);

    //Done merge, put current_merge into done_stack
    done_stack.push(current_merge);

    return current_merge;
}
