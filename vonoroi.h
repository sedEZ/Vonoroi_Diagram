#ifndef VONOROI_H
#define VONOROI_H
#include "wingededge.h"
#include <stack>

using namespace std;

/*
 * To run a vonoria diagram program using divide-and-conquer,
 * a stack is needed for recursion.
 * Because we need to run program step-by-step,
 * this Vonoroi class just for convenicence.
 * The true structure of (augumented) Vonoroi diagram is WingedEdge
 * Find it in "wingededge.h" .
 */
class Vonoroi
{
public:
    Vonoroi();
    Vonoroi(vector<double> p_x, vector<double>p_y);
    WingedEdge runOneStep();

private:
    //Stack for run Vonoroi's diagram with divide-and-conquer strategy
    stack<WingedEdge> s_stack;
};

#endif // VONOROI_H
