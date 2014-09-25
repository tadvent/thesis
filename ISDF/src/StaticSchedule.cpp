#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <algorithm>

#include "input.h"
#include "DAG.h"
#include "StaticSchedule.h"
#include "ScheduleList.h"

using namespace std;

/*****************************************************
 * extern declaration
 *****************************************************/

// original read data
extern int nTask;
extern Task tasks[MAXTASKNUM];
extern int nProcessor;
extern int speed;
extern int connect[MAXPROCNUM][MAXPROCNUM];

// DAG
extern vector<DAGNode> dag[MAXTASKNUM];

/*****************************************************
 * calc b-level
 *****************************************************/

vector<int> bLevel[MAXTASKNUM]; // bLevel[i][j]: jth execution of task i;
                                // i: 0 ~ nTask;    j: 0 ~ qi - 1

int markNode(int iTask, int ith)
{
    if(bLevel[iTask][ith] > 0)
    {
        return bLevel[iTask][ith];
    }

    int maxLevel = 0;
    for(vector<DAGArc>::const_iterator itr = dag[iTask][ith].outArcs.begin();
            itr != dag[iTask][ith].outArcs.end(); ++itr)
    {
        if(itr->crossPeriod) continue;
        int level = markNode(itr->iTask, itr->ith);
        if(level > maxLevel)
        {
            maxLevel = level;
        }
    }

    return bLevel[iTask][ith] = tasks[iTask].wcet + maxLevel;
}

void calcBLevel()
{
    // init bLevel
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        bLevel[iTask].assign(dag[iTask].size(), 0);
    }

    // mark bLevel of all nodes
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        for(int ith = 0; ith < (int)bLevel[iTask].size(); ++ith)
        {
            markNode(iTask, ith);
        }
    }
}

void printBLevel(FILE *fp)
{
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        fprintf(fp, "Task %d:", iTask);
        for(int j = 0; j < (int)bLevel[iTask].size(); ++j)
        {
            fprintf(fp, " %d", bLevel[iTask][j]);
        }
        fputc('\n', fp);
    }
}

/*****************************************************
 * calc message route
 *****************************************************/

int nextHop[MAXPROCNUM][MAXPROCNUM];

void calcNextHop()
{
    // init
    memset(nextHop, 0, sizeof(nextHop));

    for(int i=1; i<=nProcessor; ++i)
    {
        nextHop[0][i] = i;
        nextHop[i][0] = 0;
        nextHop[i][i] = -1;
    }

    // calc
    deque< pair<int,int> > qProc;
    int u, v;
    bool isVisited[MAXPROCNUM];

    for(int i=1; i<=nProcessor; ++i)
    {
        memset(isVisited, 0, sizeof(isVisited));
        qProc.assign(1, make_pair(i, 0));

        while(!qProc.empty())
        {
            u = qProc.front().first;
            v = qProc.front().second;
            qProc.pop_front();
            isVisited[u] = true;

            if(v > 0)
            {
                for(int j=1; j<=nProcessor; ++j)
                {
                    if(nextHop[v][j] > 0 && nextHop[u][j] == 0)
                    {
                        nextHop[u][j] = v;
                    }
                }
            }
            for(int j=1; j<=nProcessor; ++j)
            {
                if(!isVisited[j] && connect[j][u] > 0 && nextHop[j][i] == 0)
                {
                    nextHop[j][u] = u;
                    qProc.push_back(make_pair(j, u));
                }
            }
        }
    }
}

void printNextHop(FILE *fp)
{
    for(int i=1; i<=nProcessor; ++i)
    {
        for(int j=1; j<=nProcessor; ++j)
        {
            fprintf(fp, "%d ", nextHop[i][j]);
        }
        fputc('\n', fp);
    }
}

/*****************************************************
 * sort Processors
 *****************************************************/

int procBFS[MAXPROCNUM];
void sortProc()
{
    bool visited[MAXPROCNUM];
    int maxdegree = 0;
    int maxProc = 1;
    for(int i = 1; i <= nProcessor; ++i)
    {
        int degree = 0;
        for(int j = 1; j <= nProcessor; ++j)
        {
            if(connect[i][j] == 1)
            {
                ++degree;
            }
        }
        if(degree > maxdegree)
        {
            maxdegree = degree;
            maxProc = i;
        }
    }

    memset(visited, 0, sizeof(visited));
    procBFS[0] = maxProc;
    int beg = 0, end = 1;

    while(beg < end)
    {
        int i = procBFS[beg++];
        visited[i] = true;

        for(int j = 1; j <= nProcessor; ++j)
        {
            if(connect[i][j] == 1 && !visited[j])
            {
                procBFS[end++] = j;
            }
        }
    }
}

void sortProc2()
{
    for(int i = 0; i < nProcessor; ++i)
    {
        procBFS[i] = i+1;
    }
}

/*****************************************************
 * DLS Algorithm
 *****************************************************/

ScheduleList<TaskEx> procSchedules[MAXPROCNUM];
list< ScheduleList<MessagePass> > linkSchedulesList;
ScheduleList<MessagePass> *linkSchedule[MAXPROCNUM][MAXPROCNUM];
vector<TaskSchedule> taskSchedules[MAXTASKNUM];

void initDLS()
{
    // calc Static b-level
    calcBLevel();

    // calc nextHop
    calcNextHop();

    sortProc();

    // init Schedules
    double period = double(dag[0].size());
    for(int iProc = 0; iProc <= nProcessor; ++iProc)
    {
        procSchedules[iProc].reset(period);
    }

    linkSchedulesList.clear();
    memset(linkSchedule, 0, sizeof(linkSchedule));
    for(int iProc = 1; iProc <= nProcessor; ++iProc)
    {
        for(int jProc = iProc + 1; jProc <= nProcessor; ++jProc)
        {
            if(connect[iProc][jProc] == 1)
            {
                linkSchedulesList.push_back(ScheduleList<MessagePass>());
                linkSchedulesList.back().reset(period);
                linkSchedule[iProc][jProc] = linkSchedule[jProc][iProc] = &linkSchedulesList.back();
            }
        }
    }
    
    for(int i = 0; i <= nTask; ++i)
    {
        taskSchedules[i].assign(dag[i].size(), TaskSchedule());
    }
}

void drawbackSavedSchedule(set<ScheduleList<MessagePass> *> &savedLinks)
{
    for(set<ScheduleList<MessagePass> *>::const_iterator itr = savedLinks.begin();
            itr != savedLinks.end(); ++itr)
    {
        (*itr)->drawback();
    }
    savedLinks.clear();
}

// return -1.0 if data cannot be transfered to the target processor
double earlyStartMessage(
    int fromITask, int fromIth,
    int toITask, int toIth,
    int fromProc, int toProc,
    int data, double stTime,
    set<ScheduleList<MessagePass> *> *savedLinks,
    bool crossPeriod = false)
{
    if(fromProc == toProc || fromProc == 0 || toProc == 0 || data == 0)
    {
        return stTime;
    }
    assert(data > 0);
    while(fromProc != toProc)
    {
        int nextProc = nextHop[fromProc][toProc];
        ScheduleList<MessagePass> *curLinkSch = linkSchedule[fromProc][nextProc];
        if(savedLinks != NULL)
        {
            if(savedLinks->find(curLinkSch) == savedLinks->end())
            {
                curLinkSch->save();
                savedLinks->insert(curLinkSch);
            }
        }
        stTime = curLinkSch->findSlot(stTime, double(data)/speed);
        if(stTime < -PRECISION)
        {
            return -1.0;
        }
        if(!curLinkSch->insertWork(stTime, double(data)/speed,
            MessagePass(fromITask, fromIth, toITask, toIth, fromProc, toProc, data, crossPeriod)))
        {
            return -1.0;
        }
        fromProc = nextProc;
    }
    return stTime + double(data)/speed;
}

// return -1.0 if this task node cannot be scheduled on toProc Processor (data route blocked)
double earlyStartTaskProc(int iTask, int ith, int toProc, set<ScheduleList<MessagePass> *> *savedLinks)
{
    if(savedLinks != NULL)
    {
        savedLinks->clear();
    }
    double stTime = 0.0;
    for(vector<DAGArc>::const_iterator itr = dag[iTask][ith].inArcs.begin();
            itr != dag[iTask][ith].inArcs.end(); ++itr)
    {
        int fromProc = taskSchedules[itr->iTask][itr->ith].onProcIdx;
        double msgTime = taskSchedules[itr->iTask][itr->ith].finishTime;
        msgTime = earlyStartMessage(itr->iTask, itr->ith, iTask, ith, fromProc, toProc, itr->data, msgTime, savedLinks);
        if(msgTime < 0)
        {
            return -1.0;
        }
        if(msgTime - stTime > PRECISION)
        {
            stTime = msgTime;
        }
    }

    return procSchedules[toProc].findSlot(stTime, tasks[iTask].wcet);
}

// return false if this task node cannot be scheduled (data route blocked)
bool earlyStartTask(int iTask, int ith, int &procIdx, double &earlyTime)
{
    assert(iTask > 0);  // only calc normal tasks, not virtual task
    bool ret = false;
    set<ScheduleList<MessagePass> *> savedLinks;
    earlyTime = numeric_limits<double>::max() / 2;
    for(int i = 0; i < nProcessor; ++i)
    {
        double stTime = earlyStartTaskProc(iTask, ith, procBFS[i], &savedLinks);
        if(stTime > -PRECISION && earlyTime - stTime > PRECISION)
        {
            ret = true;
            earlyTime = stTime;
            procIdx = procBFS[i];
        }
        drawbackSavedSchedule(savedLinks);
    }
    return ret;
}


bool doDLS()
{
    initDLS();

    vector<int> nDepends[MAXTASKNUM];

    vector<pair<TaskEx, vector<DAGArc>::const_iterator> > crossMessages;

    list<TaskEx> readyTasks;
    for(int iTask = 0; iTask <= nTask; ++iTask)
    {
        nDepends[iTask].assign(dag[iTask].size(), 0);
        for(int ith = 0; ith < (int)dag[iTask].size(); ++ith)
        {
            nDepends[iTask][ith] = dag[iTask][ith].inArcs.size();
            if(nDepends[iTask][ith] == 0)
            {
                readyTasks.push_back(TaskEx(iTask, ith));
            }
        }
    }

    while(!readyTasks.empty())
    {
        list<TaskEx>::iterator exItr = readyTasks.begin();
        int exProc = -1;
        double exTime = -1.0;

        if(exItr->iTask == 0)   // virtual task node
        {
            exProc = 0;
            exTime = procSchedules[0].lastSlot();
            if(abs(earlyStartTaskProc(0, exItr->ith, 0, NULL) - exTime) > PRECISION)
            {
                return false;
            }
        }
        else    // normal task node
        {
            double maxLevel = -numeric_limits<double>::max() / 2;

            int iProc;
            double stTime;
            for(list<TaskEx>::iterator itr = readyTasks.begin();
                    itr != readyTasks.end(); ++itr)
            {
                if(!earlyStartTask(itr->iTask, itr->ith, iProc, stTime))
                {
                    return false;
                }
                if(bLevel[itr->iTask][itr->ith] - stTime - maxLevel > PRECISION)
                {
                    maxLevel = bLevel[itr->iTask][itr->ith] - stTime;
                    exProc = iProc;
                    exTime = stTime;
                    exItr = itr;
                }
            }

            earlyStartTaskProc(exItr->iTask, exItr->ith, exProc, NULL);
        }

        if(!procSchedules[exProc].insertWork(exTime, tasks[exItr->iTask].wcet, TaskEx(exItr->iTask, exItr->ith)))
        {
            return false;
        }
        taskSchedules[exItr->iTask][exItr->ith].onProcIdx = exProc;
        taskSchedules[exItr->iTask][exItr->ith].startTime = exTime;
        taskSchedules[exItr->iTask][exItr->ith].finishTime = exTime + tasks[exItr->iTask].wcet;

        // renew readyTasks
        for(vector<DAGArc>::const_iterator itr = dag[exItr->iTask][exItr->ith].outArcs.begin();
                itr != dag[exItr->iTask][exItr->ith].outArcs.end(); ++itr)
        {
            if(itr->crossPeriod)
            {
                // push
                crossMessages.push_back(make_pair(TaskEx(exItr->iTask, exItr->ith), itr));
                continue;
            }
            if(--nDepends[itr->iTask][itr->ith] == 0)
            {
                if(itr->iTask == 0)
                {
                    readyTasks.push_front(TaskEx(0, itr->ith));
                }
                else
                {
                    readyTasks.push_back(TaskEx(itr->iTask, itr->ith));
                }
            }
        }

        // remove exItr
        readyTasks.erase(exItr);
    }

    for(vector<pair<TaskEx, vector<DAGArc>::const_iterator> >::const_iterator itr = crossMessages.begin();
        itr != crossMessages.end(); ++itr)
    {
        int fTsk = itr->first.iTask;
        int fith = itr->first.ith;
        int tTsk = itr->second->iTask;
        int tith = itr->second->ith;
        if(!earlyStartMessage(fTsk, fith, tTsk, tith,
            taskSchedules[fTsk][fith].onProcIdx, taskSchedules[tTsk][tith].onProcIdx,
            itr->second->data, taskSchedules[fTsk][fith].finishTime, NULL, true))
        {
            return false;
        }
    }

    return true;
}

void printSchedules(FILE *fp)
{
    char space[] = "    ";
    for(int i = 1; i <= nProcessor; ++i)        // i = 0 to include virtual Processor
    {
        fprintf(fp, "Processor %d:\n", i);
        procSchedules[i].print(fp, space);
        fputc('\n', fp);
    }

    for(int i = 1; i <= nProcessor; ++i)
    {
        for(int j = i+1; j <= nProcessor; ++j)
        {
            if(linkSchedule[i][j] == NULL)
            {
                continue;
            }
            fprintf(fp, "Link %d --- %d:\n", i, j);
            linkSchedule[i][j]->print(fp, space);
            fputc('\n', fp);
        }
    }
}

void printScheduleData(FILE *fp)
{
    fprintf(fp, "%d\n", nTask);
    fprintf(fp, "%d\n", nProcessor);
    for(int i = 1; i <= nProcessor; ++i)
    {
        fprintf(fp, "%d\n", procSchedules[i].workListNum());
        procSchedules[i].printData(fp);
    }

    int numLink = 0;
    for(int i = 1; i <= nProcessor; ++i)
    {
        for(int j = i+1; j <= nProcessor; ++j)
        {
            if(linkSchedule[i][j] == NULL)
            {
                continue;
            }
            ++numLink;
            fprintf(fp, "%d %d %d\n", i, j, linkSchedule[i][j]->workListNum());
            linkSchedule[i][j]->printData(fp);
            fputc('\n', fp);
        }
    }
    fprintf(fp, "0 0 0\n");

    // print schedule span and number of schedule lists
    fprintf(fp, "%f %d\n", (float)procSchedules[0].workListNum(), nProcessor+numLink+1);
}

void printTaskSchedules(FILE *fp)
{
    for(int iTask = 1; iTask <= nTask; ++iTask) // iTask = 0 to inc virtual Task
    {
        fprintf(fp, "Task %d:\n", iTask);
        for(int j = 0; j < (int)taskSchedules[iTask].size(); ++j)
        {
            fprintf(fp, "    (%d %d): ", iTask, j);
            taskSchedules[iTask][j].print(fp);
            fputc('\n', fp);
        }
        fputc('\n', fp);
    }
}


/*****************************************************
 * header class member function difinition
 *****************************************************/

void TaskEx::print(FILE *fp) const
{
    fprintf(fp, "Task (%d %d), time %d", iTask, ith, tasks[iTask].wcet);
}

void TaskEx::printData(FILE *fp) const
{
    fprintf(fp, "(%d,%d)", iTask, ith);
}

void MessagePass::print(FILE *fp) const
{
    fprintf(fp, "Task (%d %d) --> (%d %d), Processor %d --> %d, data %d",
        fromNode.iTask, fromNode.ith, toNode.iTask, toNode.ith, fromProc, toProc, data);
    if(crossPeriod)
    {
        fprintf(fp, " to Next Period");
    }
}

void MessagePass::printData(FILE *fp) const
{
    fprintf(fp, "(%d,%d) (%d,%d) %d %d %d",
        fromNode.iTask, fromNode.ith, toNode.iTask, toNode.ith, fromProc, toProc, data);
}

void TaskSchedule::print(FILE *fp) const
{
    fprintf(fp, "[Processor %3d] time %5.2f --> %5.2f", onProcIdx, startTime, finishTime);
}
