/*
 * pinchGraphs2.h
 *
 *  Created on: 9 Mar 2012
 *      Author: benedictpaten
 */

#ifndef ST_PINCH_GRAPHS_H_
#define ST_PINCH_GRAPHS_H_

#include "sonLib.h"

//The exception string
extern const char *ST_PINCH_GRAPH_EXCEPTION_ID;

//Datastructures

typedef struct _stThreadSet stThreadSet;

typedef struct _stThreadIt {
    stThreadSet *threadSet;
    int32_t index;
} stThreadIt;

typedef struct _stThread stThread;

typedef struct _stSegment stSegment;

typedef struct _stBlock stBlock;

typedef struct _stBlockIt {
    stSegment *segment;
} stBlockIt;

//Thread set

stThreadSet *stThreadSet_construct();

void stThreadSet_destruct(stThreadSet *threadSet);

stThread *stThreadSet_addThread(stThreadSet *threadSet, int64_t name, int64_t start, int64_t length);

stThread *stThreadSet_getThread(stThreadSet *threadSet, int64_t name);

stSegment *stThreadSet_getSegment(stThreadSet *threadSet, int64_t name, int64_t coordinate);

int32_t stThreadSet_getSize(stThreadSet *threadSet);

stThreadIt stThreadSet_getIterator(stThreadSet *threadSet);

stThread *stThreadIt_getNext(stThreadIt *);

void stThreadSet_joinTrivialBoundaries(stThreadSet *threadSet);

stSegment *stThreadSet_getSegment(stThreadSet *threadSet, int64_t name, int64_t coordinate);

//Thread

int64_t stThread_getName(stThread *stThread);

int64_t stThread_getStart(stThread *stThread);

int64_t stThread_getLength(stThread *stThread);

stSegment *stThread_getSegment(stThread *stThread, int64_t coordinate);

stSegment *stThread_getFirst(stThread *stThread);

stSegment *stThread_get3Prime(stThread *thread, stSegment *segment);

stSegment *stThread_get5Prime(stThread *thread, stSegment *segment);

void stThread_split(stThread *thread, int64_t leftSideOfSplitPoint);

void stThread_joinTrivialBoundaries(stThread *thread);

//Segments

int64_t stSegment_getStart(stSegment *segment);

int64_t stSegment_getLength(stSegment *segment);

stBlock *stSegment_getBlock(stSegment *segment);

bool stSegment_getBlockOrientation(stSegment *segment);

//Blocks

stBlock *stBlock_construct(stSegment *segment1, bool orientation1, stSegment *segment2, bool orientation2);

stBlock *stBlock_pinch(stBlock *block1, stBlock *block2, bool orientation);

stBlock *stBlock_pinch2(stBlock *block1, stSegment *segment, bool orientation);

stBlockIt stBlock_getSegmentIterator(stBlock *block);

stSegment *stBlockIt_getNext(stBlockIt *stBlockIt);

void stBlock_destruct(stBlock *block);

int64_t stBlock_getLength(stBlock *block);

stSegment *stBlock_getFirst(stBlock *block);

uint32_t stBlock_getDegree(stBlock *block);

#endif /* ST_PINCH_GRAPHS_H_ */
