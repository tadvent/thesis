#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <algorithm>

#include "input.h"
#include "SDF.h"
#include "DAG.h"

using namespace std;

/*****************************************************
 * extern declaration
 *****************************************************/

// original read data
extern int nTask;
extern Task tasks[MAXTASKNUM];
extern int J;

// SDF Graph
extern list<AdjNode> sdf[MAXTASKNUM];
extern int q[MAXTASKNUM];

/*****************************************************
 * transmit the SDF to DAG
 *****************************************************/


vector<DAGNode> dag[MAXTASKNUM];    // dag[i][j] means the jth execution of task i;
                                    // i: 0 ~ nTask;    j: 0 ~ qi - 1

void addCycArcs()
{
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        for(list<AdjNode>::const_iterator itr = sdf[iTask].begin();
            itr != sdf[iTask].end(); ++itr)
        {
            if(itr->produce > 0 && itr->delay > 0)
            {
                int prd = (itr->delay - 1) % itr->produce + 1;
                int prdith = dag[iTask].size() - (((itr->delay - 1) / itr->produce) % dag[iTask].size()) - 1;
                int csm = itr->consume;
                int csmith = 0;
                int trs = 0;
                if(prd < csm)
                {
                    trs = prd;
                    prd = 0;
                    csm -= trs;
                }
                else
                {
                    trs = csm;
                    csm = 0;
                    prd -= trs;
                }
                while(prdith < (int)dag[iTask].size() && csmith < (int)dag[itr->toTaskIdx].size())
                {
                    if(iTask == 0 || itr->toTaskIdx == 0)
                    {
                        trs = 0;
                    }
                    dag[iTask][prdith].outArcs.push_back(DAGArc(trs, itr->toTaskIdx, csmith, true));
                    if(prd == 0)
                    {
                        prd = itr->produce;
                        ++prdith;
                    }
                    if(csm == 0)
                    {
                        csm = itr->consume;
                        ++csmith;
                    }
                    if(prd < csm)
                    {
                        trs = prd;
                        prd = 0;
                        csm -= trs;
                    }
                    else
                    {
                        trs = csm;
                        csm = 0;
                        prd -= trs;
                    }
                }
            }
        }
    }
}

bool isReady(int iTask)
{
    bool noInwardArc = true;
    bool dataSufficient = true;

    for(list<AdjNode>::const_iterator itr = sdf[iTask].begin();
            itr != sdf[iTask].end(); ++itr)
    {
        if(itr->produce < 0)   // inward arc
        {
            noInwardArc = false;
            if(itr->revarc->buffer < itr->revarc->consume)
            {
                dataSufficient  = false;
                break;
            }
        }
    }

    return (noInwardArc || dataSufficient) && ((int)dag[iTask].size() < (J * q[iTask]));
}

bool SDF2DAG()
{
    // reset DAG
    for(int i=0; i<=nTask; ++i)
    {
        dag[i].clear();
    }

    // define & init the queue of ready tasks
    set<int> readyTasks;        // tasks that are ready to run
                                // 1. tasks with no in arcs
                                // 2. data are ready on all the inward arcs
                                // 3. task has not ran q_i times

    for(int iTask=0; iTask<=nTask; ++iTask)
    {
        if(isReady(iTask))
        {
            readyTasks.insert(iTask);
        }
    }

    // make & run the ready nodes
    while(!readyTasks.empty())
    {
        int iTask = *readyTasks.begin();
        int ith = dag[iTask].size();

        dag[iTask].push_back(DAGNode());
        dag[iTask].back().iTask = iTask;
        dag[iTask].back().ith = ith;
        
        if(ith > 0 && tasks[iTask].selfdelay >= 0) // self-dependent
        {
            dag[iTask][ith-1].outArcs.push_back(
                    DAGArc(tasks[iTask].selfdelay, iTask, ith));
            dag[iTask][ith].inArcs.push_back(
                    DAGArc(tasks[iTask].selfdelay, iTask, ith-1));
        }

        for(list<AdjNode>::iterator itr = sdf[iTask].begin();
                itr != sdf[iTask].end(); ++itr)
        {
            if(itr->produce > 0)    // outward arc
            {
                itr->buffer += itr->produce;
                if(itr->buffer >= itr->consume && isReady(itr->toTaskIdx))
                {
                    readyTasks.insert(itr->toTaskIdx);
                }
            }
            else if(itr->produce < 0)   // inward arc
            {
                itr->revarc->buffer -= itr->revarc->consume;

                // calc the dependence on which executions of task itr->toTaskIdx
                int produce = -itr->consume;
                int consume = -itr->produce;
                int delay = itr->revarc->delay;

                div_t dep1 = div(consume*ith - delay, produce);
                if(dep1.rem < 0) {--dep1.quot; dep1.rem += produce;}
                div_t dep2 = div(consume*(ith+1) - delay, produce);
                if(dep2.rem <= 0) {--dep2.quot; dep2.rem += produce;}

                if(itr->toTaskIdx == 0) // task node (iTask) dependent on virtual node
                {
                    if(dep2.quot >= 0)
                    {
                        dag[0][dep2.quot].outArcs.push_back(
                                DAGArc(0, iTask, ith));
                        dag[iTask][ith].inArcs.push_back(
                                DAGArc(0, 0, dep2.quot));
                    }
                }
                else if(iTask == 0)     // virtual node (iTask) dependent on task node
                {
                    if(dep2.quot >= 0)
                    {
                        div_t last_dep2 = div(consume * ith - delay, produce);
                        if(last_dep2.rem <= 0) {--last_dep2.quot; last_dep2.rem += produce;}

                        if(last_dep2.quot != dep2.quot)
                        {
                            dag[itr->toTaskIdx][dep2.quot].outArcs.push_back(
                                DAGArc(0, 0, ith));
                            dag[0][ith].inArcs.push_back(
                                DAGArc(0, itr->toTaskIdx, dep2.quot));
                        }
                    }
                }
                // task node (iTask) dependent on task node
                else if(dep1.quot == dep2.quot)
                {
                    if(dep1.quot >= 0 && dep1.rem < dep2.rem)   // data on delay are not enough
                    {
                        assert(dep2.rem - dep1.rem == consume);
                        if(iTask != 0)
                        {
                            dag[itr->toTaskIdx][dep1.quot].outArcs.push_back(
                                    DAGArc(consume, iTask, ith));
                            dag[iTask][ith].inArcs.push_back(
                                    DAGArc(consume, itr->toTaskIdx, dep1.quot));
                        }
                        else    // this is virtual node
                        {
                            dag[itr->toTaskIdx][dep1.quot].outArcs.push_back(
                                    DAGArc(0, iTask, ith));
                            dag[iTask][ith].inArcs.push_back(
                                    DAGArc(0, itr->toTaskIdx, dep1.quot));
                        }
                    }
                }
                else    // dep1.quot < dep2.quot
                {
                    if(dep1.quot >= 0)
                    {
                        dag[itr->toTaskIdx][dep1.quot].outArcs.push_back(
                                DAGArc(produce - dep1.rem, iTask, ith));
                        dag[iTask][ith].inArcs.push_back(
                                DAGArc(produce - dep1.rem, itr->toTaskIdx, dep1.quot));
                    }

                    for(int k = (dep1.quot+1 >= 0? dep1.quot+1: 0); k < dep2.quot; ++k)
                    {
                        dag[itr->toTaskIdx][k].outArcs.push_back(
                                DAGArc(produce, iTask, ith));
                        dag[iTask][ith].inArcs.push_back(
                                DAGArc(produce, itr->toTaskIdx, k));
                    }

                    if(dep2.quot >= 0)
                    {
                        dag[itr->toTaskIdx][dep2.quot].outArcs.push_back(
                                DAGArc(dep2.rem, iTask, ith));
                        dag[iTask][ith].inArcs.push_back(
                                DAGArc(dep2.rem, itr->toTaskIdx, dep2.quot));
                    }
                }
            }
            else    // itr->produce == 0, never reach here
            {
                return false;
            }
        }

        if(!isReady(iTask))
        {
            readyTasks.erase(iTask);
        }
    }

    // sort inArcs according to the data transffered
    for(int i = 1; i <= nTask; ++i)
    {
        if((int)dag[i].size() != J * q[i])
        {
            return false;
        }
        for(int j = 0; j < (int)dag[i].size(); ++j)
        {
            sort(dag[i][j].inArcs.begin(), dag[i][j].inArcs.end());
        }
    }

    addCycArcs();

    return true;
}

void printDAG(FILE *fp)
{
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        fprintf(fp, "Task %d:\n", iTask);
        for(int ith = 0; ith < J * q[iTask]; ++ith)
        {
            fprintf(fp, "  (%d, %d):", iTask, ith);
            for(vector<DAGArc>::const_iterator itr = dag[iTask][ith].outArcs.begin();
                    itr != dag[iTask][ith].outArcs.end(); ++itr)
            {
                if(itr->crossPeriod)
                {
                    fprintf(fp, " NP");
                }
                fprintf(fp, " (%d %d) %d;", itr->iTask, itr->ith, itr->data);
            }
            fputc('\n', fp);
        }
        fputc('\n', fp);
    }
}
