#ifndef SCHEDULE_LIST_H
#define SCHEDULE_LIST_H

#include <cstdio>
#include <cmath>
#include <limits>
#include <utility>
#include <map>
#include <deque>

#include "Config.h"

class DoubleLess
{
public:
    bool operator()(const double l, const double r) const
    {
        return (r - l) > PRECISION;
    }
};

template<class DataType>
class ScheduleList
{
public:

    ScheduleList(void)
    {
        idleList.insert(std::make_pair(0.0, std::numeric_limits<double>::max()));
    }

    ~ScheduleList(void)
    {
    }

    void reset(double idleSpan = std::numeric_limits<double>::max())
    {
        idleList.clear();
        idleList.insert(std::make_pair(0.0, idleSpan));

        workList.clear();
    }

    double lastSlot() const
    {
        return idleList.rbegin()->first;
    }

    double findSlot(double stTime, double span) const
    {
        if(std::abs(span) < PRECISION)
        {
            return stTime;
        }
        std::map<double, double, DoubleLess>::const_iterator itr = idleList.upper_bound(stTime);
        if(itr != idleList.begin())
        {
            --itr;
            if(itr->first + itr->second - (stTime + span) > -PRECISION)
            {
                return stTime;
            }
            ++itr;
        }
        for(; itr != idleList.end(); ++itr)
        {
            if(itr->second - span > -PRECISION)
            {
                return itr->first;
            }
        }
        return -1.0;
    }

    bool insertWork(double time, double span, const DataType &data)
    {
        if(std::abs(span) < PRECISION)
        {
            return true;
        }

        // check if idle valid
        std::map<double, double, DoubleLess>::iterator itr = idleList.upper_bound(time);
        if(itr == idleList.begin())
        {
            return false;
        }

        --itr;
        if(time - itr->first > PRECISION)   // time > slot's beginning
        {
            double idleSpan = itr->first + itr->second - time;
            itr->second = time - itr->first;

            if(idleSpan - span > PRECISION) // work end < slot end
            {
                workList.insert(std::make_pair(time, WorkNode(span, data)));
                idleList.insert(itr, std::make_pair(time + span, idleSpan - span));
            }
            else if(idleSpan - span > -PRECISION)    // work end == slot end
            {
                workList.insert(std::make_pair(time, WorkNode(span, data)));
            }
            else    // slot span not enough
            {
                return false;
            }
        }
        else if(time - itr->first > -PRECISION) // time == slots's beginning
        {
            double idleSpan = itr->second;
            idleList.erase(itr++);

            if(idleSpan - span > PRECISION) // work end < slot end
            {
                workList.insert(std::make_pair(time, WorkNode(span, data)));
                idleList.insert(itr, std::make_pair(time + span, idleSpan - span));
            }
            else if(idleSpan -span > -PRECISION)    // work end == slot end
            {
                workList.insert(std::make_pair(time, WorkNode(span, data)));
            }
            else    // slot span not enough
            {
                return false;
            }
        }
        else    // time < slot's beginning, never reach here
        {
            return false;
        }

        return true;
    }

    // save current state
    void save()
    {
        idleListSave = idleList;
        workListSave = workList;
    }

    // drawback to last saved state
    void drawback()
    {
        idleList.swap(idleListSave);
        workList.swap(workListSave);
    }

    void print(FILE *fp, char *preStr) const
    {
        for(typename std::map<double, WorkNode, DoubleLess>::const_iterator itr = workList.begin();
                itr != workList.end(); ++itr)
        {
            fprintf(fp, "%s%5.2f --> %5.2f: ", preStr, itr->first, itr->first + itr->second.timeSpan);
            itr->second.data.print(fp);
            fputc('\n', fp);
        }
    }

    int workListNum(){
        return workList.size();
    }

    void printData(FILE *fp) const
    {
        for(typename std::map<double, WorkNode, DoubleLess>::const_iterator itr = workList.begin();
                itr != workList.end(); ++itr)
        {
            fprintf(fp, "%20.15f %20.15f ", itr->first, itr->first + itr->second.timeSpan);
            itr->second.data.printData(fp);
            fputc('\n', fp);
        }
    }
private:

    struct WorkNode
    {
        double timeSpan;
        DataType data;

        WorkNode(): timeSpan(0.0), data() {}
        WorkNode(double TimeSpan, const DataType &Data)
            :timeSpan(TimeSpan), data(Data) {}
    };

    std::map<double, double, DoubleLess> idleList;
    std::map<double, WorkNode, DoubleLess> workList;

    std::map<double, double, DoubleLess> idleListSave;
    std::map<double, WorkNode, DoubleLess> workListSave;
};


#endif
