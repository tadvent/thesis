#ifndef STATIC_SCHEDULE_H
#define STATIC_SCHEDULE_H

#include <cstdio>

struct TaskEx
{
    int iTask;
    int ith;

    TaskEx() {}
    TaskEx(int Task, int Ith)
        :iTask(Task), ith(Ith) {}
    void print(FILE *fp) const;
    void printData(FILE *fp) const;
};

struct MessagePass
{
    TaskEx fromNode;
    TaskEx toNode;
    int fromProc;
    int toProc;
    int data;
    bool crossPeriod;

    MessagePass() {}
    MessagePass(int fromITask, int fromIth, int toITask, int toIth,
        int FromProcessor, int ToProcessor, int Data, bool CrossPeriod = false)
        :fromNode(fromITask, fromIth), toNode(toITask, toIth),
        fromProc(FromProcessor), toProc(ToProcessor), data(Data), crossPeriod(CrossPeriod) {}
    void print(FILE *fp) const;
    void printData(FILE *fp) const;
};

struct TaskSchedule
{
    int onProcIdx;
    double startTime;
    double finishTime;

    TaskSchedule() {}
    TaskSchedule(int OnProcessor, double StartTime, double FinishTime)
        :onProcIdx(OnProcessor), startTime(StartTime), finishTime(FinishTime) {}
    void print(FILE *fp) const;
};

#ifndef NDEBUG
void calcBLevel();
void printBLevel(FILE *fp);
#endif


void calcNextHop();
void printNextHop(FILE *fp);

bool doDLS();
void printSchedules(FILE *fp);
void printScheduleData(FILE *fp);
void printTaskSchedules(FILE *fp);


#endif