#ifndef INPUT_H
#define INPUT_H

#include <cstdio>

#include "Config.h"

/*****************************************************
 * input data-type declaration
 *****************************************************/

// time constraints
struct Task
{
    int wcet;
    int T;
    int offset;
    int deadline;
    int selfdelay;   // >0: if this task is self-dependent, data transmit
                     // =0: must be sequentially executed, but no data transmit
                     // <0: no self-dependent
};

// data precedence constraints
struct Arc
{
    int src;
    int produce;
    int snk;
    int consume;
    int delay;
};

// read data
bool input(FILE *fp);

#endif

