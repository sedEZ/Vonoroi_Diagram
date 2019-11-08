#include "vonoroi.h"

Vonoroi::Vonoroi()
{

}
//Constructor
Vonoroi::Vonoroi(vector<double> p_x, vector<double>p_y)
{
    //Create original generating points & push it into stack
    WingedEdge generating_points(p_x,p_y);
    s_stack.push(generating_points);
}

WingedEdge Vonoroi::runOneStep()
{
    //Todo:Run one step and stop, return result vonoroi's diagram
    //difficulty: How can I merge 2 vonoroi's diagram which had run already using stack?
    //            With recursion, they will return.
    //      ******With stack, it may need extra data structure(stack for example) to collect already-ran vonoroi's diagram(More practical)
    //* Maybe nothing to worry about, just extract element until reaching to the end.
}
