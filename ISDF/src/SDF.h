#ifndef SDF_H
#define SDF_H

/*****************************************************
 * Node type of SDF Graph
 *****************************************************/

struct AdjNode
{
    int toTaskIdx;
    int produce;    // < 0 on revarcs in sdf
    int consume;    // < 0 on revarcs in sdf
    int delay;      // always 0 on revarcs in sdf
    int buffer;     // always 0 on revarcs in sdf
    AdjNode *revarc;

    AdjNode(){}
    AdjNode(int ToTaskIdx, int Produce, int Consume, int Delay)
            :
            toTaskIdx(ToTaskIdx),
            produce(Produce),
            consume(Consume),
            delay(Delay),
            buffer(Delay),
            revarc(NULL) {}
};


// construct SDF Graph from read data
void constructSDF();

// print the Matrix of the SDF Graph
void printMatrix(FILE *fp);

// calculate the q vector
bool solveMatrix();

// print the q vector
void printq(FILE *fp);

#endif