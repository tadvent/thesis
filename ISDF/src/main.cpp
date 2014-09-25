/* 
 * File:   main.cpp
 * Author: Wang Jinlin
 */

#include <cstdio>
#include <cstdlib>

#include "input.h"
#include "SDF.h"
#include "DAG.h"
#include "StaticSchedule.h"

/*****************************************************
 * main function
 *****************************************************/
int main(int argc, char** argv) 
{
    freopen("i.txt", "r", stdin);
    freopen("o.txt", "w", stdout);

    // read input file
    if(input(stdin))
    {
        printf("Read successfully!\n");
    }
    putchar('\n');

    // construct SDF graph
    constructSDF();
    printMatrix(stdout);
    putchar('\n');

    // solve vector q
    printf("==== Solve Matrix ====\n");
    if(solveMatrix())
    {
        printf("Solve Matrix successfully!\n");
        printq(stdout);
        putchar('\n');
    }
    else
    {
        printf("Can't solve Matrix...\n");
        exit(1);
    }
    putchar('\n');

    // translate SDF to DAG
    printf("======== DAG ========\n");
    if(SDF2DAG())
    {
        printDAG(stdout);
    }
    else
    {
        printf("DAG error!\n");
        exit(2);
    }
    putchar('\n');

#ifndef NDEBUG
    // calc Static b-level
    printf("======== SBL ========\n");
    calcBLevel();
    printBLevel(stdout);
    putchar('\n');
#endif

    // calc shortest Message Route
    printf("====== NextHop ======\n");
    calcNextHop();
    printNextHop(stdout);
    putchar('\n');

    // Schedules
    printf("===== Schedules =====\n");
    if(doDLS())
    {
        printSchedules(stdout);
        putchar('\n');
        printf("  ---- Task ----\n");
        printTaskSchedules(stdout);

        FILE *datafile = fopen("data.txt", "w");
        if(datafile == NULL){
            printf("Error open data.txt file!\n");
            exit(3);
        }
        printScheduleData(datafile);
        fclose(datafile);
    }
    else
    {
        printf("Schedule failed...\n");
        exit(4);
    }
    putchar('\n');


    return 0;
}

