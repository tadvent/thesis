#include <cstdio>
#include <cstring>

#include "input.h"

/*****************************************************
 * input data-type definition
 *****************************************************/

int nTask;
Task tasks[MAXTASKNUM];
int nArc;
Arc arcs[MAXARCNUM + MAXTASKNUM];   // if every task has a time constraint,
                                    // there will be MAXTASKNUM more arcs.

int J;  // the number of minimal period that a large period contains

int nProcessor;     // the number of processors
int speed;          // data transmited between processors per unit time
int nConnection;    // number of connections between processors

int connect[MAXPROCNUM][MAXPROCNUM];    // connect[i][j] == 1 if i j are connected
                                        // else 0
                                        // connect[i][i] == 0

/*****************************************************
 * read data
 *****************************************************/

bool input(FILE *fp)
{
    if(fscanf(fp, "%d", &nTask) != 1 || nTask < 1)
    {
        fprintf(stderr, "task number error!\n");
        return false;
    }
    for(int i=1; i<=nTask; ++i)
    {
        if(fscanf(fp, "%d%d%d%d%d", &tasks[i].wcet, &tasks[i].T,
                &tasks[i].offset, &tasks[i].deadline, &tasks[i].selfdelay) != 5)
        {
            fprintf(stderr, "task parameter error!\n");
            return false;
        }
    }
    if(fscanf(fp, "%d", &nArc) != 1 || nArc < 0)
    {
        fprintf(stderr, "arc number error!\n");
        return false;
    }
    for(int i=0; i<nArc; ++i)
    {
        if(fscanf(fp, "%d%d%d%d%d", &arcs[i].src, &arcs[i].produce,
                &arcs[i].snk, &arcs[i].consume, &arcs[i].delay) != 5)
        {
            fprintf(stderr, "arc parameter error!\n");
            return false;
        }
    }

    if(fscanf(fp, "%d", &J) != 1)
    {
        fprintf(stderr, "read J error!\n");
        return false;
    }

    if(fscanf(fp, "%d", &nProcessor) != 1)
    {
        fprintf(stderr, "read nProcessor error!\n");
        return false;
    }
    if(fscanf(fp, "%d", &speed) != 1)
    {
        fprintf(stderr, "read speed error!\n");
        return false;
    }
    if(fscanf(fp, "%d", &nConnection) != 1)
    {
        fprintf(stderr, "read nConnection error!\n");
        return false;
    }

    memset(connect, 0, sizeof(connect));
    for(int i = 1; i <= nProcessor; ++i)
    {
        connect[i][0] = connect[0][i] = 1;
    }
    for(int i = 0; i < nConnection; ++i)
    {
        int p1, p2;
        if(fscanf(fp, "%d%d", &p1, &p2) != 2)
        {
            fprintf(stderr, "read connections error!\n");
            return false;
        }
        connect[p1][p2] = connect[p2][p1] = 1;
    }

    return true;
}



