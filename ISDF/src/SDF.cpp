#include <list>

#include "input.h"
#include "SDF.h"
#include "Rational.h"

using namespace std;


/*****************************************************
 * extern declaration
 *****************************************************/

// original read data
extern int nTask;
extern Task tasks[MAXTASKNUM];
extern int nArc;
extern Arc arcs[MAXARCNUM + MAXTASKNUM];

/*****************************************************
 * translate the constraits into SDF, which saved as adjacent list.
 *****************************************************/

list<AdjNode> sdf[MAXTASKNUM];    // sdf[0] is the virtual node,
                                  // sdf[1]~sdf[N] are task nodes.

void constructSDF()
{
    // reset all lists
    for(int i=0; i<=nTask; ++i)
    {
        sdf[i].clear();
    }

    // add virtual node
    tasks[0].wcet = 1;
    tasks[0].T = 1;
    tasks[0].offset = 0;
    tasks[0].deadline = 1;
    tasks[0].selfdelay = 0;

    // add time constraints
    for(int i=1; i<=nTask; ++i)
    {
        if(tasks[i].T != 0 && tasks[i].deadline != 0)
        {
            // T constraints:
            sdf[0].push_back(AdjNode(i,
                    1, tasks[i].T, tasks[i].T - tasks[i].offset));
            // add reverse arc to solve matrix
            sdf[i].push_back(AdjNode(0,
                    -tasks[i].T, -1, 0));
            sdf[0].back().revarc = &sdf[i].back();
            sdf[i].back().revarc = &sdf[0].back();

            // deadline constraints:
            sdf[i].push_back(AdjNode(0,
                    tasks[i].T, 1, tasks[i].deadline + tasks[i].offset));
            // add reverse arc
            sdf[0].push_back(AdjNode(i,
                    -1, -tasks[i].T, 0));
            sdf[0].back().revarc = &sdf[i].back();
            sdf[i].back().revarc = &sdf[0].back();
        }
    }

    // add data precedence constraints
    for(int i=0; i<nArc; ++i)
    {
        sdf[arcs[i].src].push_back(AdjNode(arcs[i].snk, arcs[i].produce,
                arcs[i].consume, arcs[i].delay));
        // add reverse arc to solve matrix
        sdf[arcs[i].snk].push_back(AdjNode(arcs[i].src, -arcs[i].consume,
                -arcs[i].produce, 0));
        sdf[arcs[i].src].back().revarc = &sdf[arcs[i].snk].back();
        sdf[arcs[i].snk].back().revarc = &sdf[arcs[i].src].back();
    }
}

void printMatrix(FILE *fp)
{
    for(int i=0; i<=nTask; ++i)
    {
        for(list<AdjNode>::const_iterator itr = sdf[i].begin();
                itr != sdf[i].end(); ++itr)
        {
            if(itr->produce < 0)continue;
            for(int k=0; k<=nTask; ++k)
            {
                if(k == i)
                {
                    fprintf(fp, "%d ", itr->produce);
                }
                else if(k == itr->toTaskIdx)
                {
                    fprintf(fp, "%d ", -itr->consume);
                }
                else
                {
                    fprintf(fp, "0 ");
                }
            }
            fputc('\n', fp);
        }
    }
}


/*****************************************************
 * solve the SDF Matrix
 *****************************************************/

int q[MAXTASKNUM];  // number of executions of each task in a minimal period

bool sm_dfs(const Rational &level, int iTask, Rational r[])
{
    r[iTask] = level;

    Rational l;
    for(list<AdjNode>::const_iterator i = sdf[iTask].begin();
            i != sdf[iTask].end(); ++i)
    {
        l = level * Rational(i->produce, i->consume);
        if(r[i->toTaskIdx] != Rational(0,1))
        {
            if(r[i->toTaskIdx] == l)
            {
                continue;
            } else return false;
        }
        if(!sm_dfs(l, i->toTaskIdx, r))
        {
            return false;
        }
    }

    return true;
}

// return false if rank(Matrix) != nTask
bool solveMatrix()
{
    Rational r[MAXTASKNUM]; // 

    // reset r
    for(int i = 0; i <= nTask; ++i)
    {
        r[i] = Rational(0,1);
    }

    if(!sm_dfs(Rational(1,1), 0, r))
    {
        return false;
    }

    for(int i = 0; i <= nTask; ++i)
    {
        if(r[i] == Rational(0,1))
        {
            return false;
        }
    }
    
    // multiply to integer
    int l = 1;
    for(int i = 0; i <= nTask; ++i)
    {
        l = lcm(l, r[i].getBop());
    }
    for(int i = 0; i <= nTask; ++i)
    {
        q[i] = l * r[i].getTop() / r[i].getBop();
    }

    return true;
}

void printq(FILE *fp)
{
    for(int i=0; i<=nTask; ++i)
    {
        fprintf(fp, "%d ", q[i]);
    }
}
