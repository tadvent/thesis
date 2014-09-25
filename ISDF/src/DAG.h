#ifndef DAG_H
#define DAG_H

#include <vector>

/*****************************************************
 * declaration of Arc and Node type of DAG
 *****************************************************/

struct DAGArc
{
    int data;   // amount of data transmited on the arc, >= 0
    int iTask;  // index of the other task, when < 0 in next period
    int ith;    // ith execution of the task
    bool crossPeriod;
    DAGArc() {}
    DAGArc(int Data, int ITask, int Ith, bool CrossPeriod = false)
            :
            data(Data),
            iTask(ITask),
            ith(Ith),
            crossPeriod(CrossPeriod) {}

    bool operator<(const DAGArc &r) const
    {
        return data > r.data;
    }
};

struct DAGNode
{
    int iTask;  // index of this task
    int ith;    // ith execution of the task
    std::vector<DAGArc> inArcs;  // arcs that transmit data to this node
    std::vector<DAGArc> outArcs; // arcs with data go out of this node
};

// translate SDF to DAG
bool SDF2DAG();

// print the DAG
void printDAG(FILE *fp);

#endif
