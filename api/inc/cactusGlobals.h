/*
 * Copyright (C) 2009-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef CACTUS_GLOBALS_H_
#define CACTUS_GLOBALS_H_

#include <inttypes.h>

/*
 * For the basic lib stuff
 */
#include "sonLib.h"
/*
 * For lists
 */
#include "commonC.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic data structure declarations (contents hidden)
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef int64_t Name;
#define NULL_NAME INT64_MAX

typedef struct _metaEvent MetaEvent;
typedef struct _event Event;
typedef struct _eventTree EventTree;
typedef struct _sequence Sequence;
typedef struct _end End;
typedef struct _cap Cap;
typedef struct _cap Segment;
typedef struct _end Block;
typedef struct _group Group;
typedef struct _link Link;
typedef struct _chain Chain;
typedef struct _face Face;
typedef struct _faceEnd FaceEnd;
typedef struct _flower Flower;
typedef struct _cactusDisk CactusDisk;
typedef struct _flowerWriter FlowerWriter;

typedef stSortedSetIterator EventTree_Iterator;
typedef struct _end_instanceIterator End_InstanceIterator;
typedef struct _block_instanceIterator Block_InstanceIterator;
//typedef stSortedSetIterator Group_EndIterator;
typedef struct _group_endIterator Group_EndIterator;

typedef stListIterator Flower_SequenceIterator;
typedef stListIterator Flower_CapIterator;
typedef stListIterator Flower_EndIterator;
typedef stListIterator Flower_GroupIterator;
typedef stListIterator Flower_ChainIterator;
typedef stSortedSetIterator CactusDisk_FlowerIterator;

typedef stSortedSetIterator Flower_FaceIterator;
typedef stSortedSetIterator Flower_SegmentIterator;
typedef stSortedSetIterator Flower_BlockIterator;

typedef stSortedSetIterator Reference_PseudoChromosomeIterator;
typedef stListIterator PseudoChromsome_PseudoAdjacencyIterator;
typedef struct _face_FaceEndIterator Face_FaceEndIterator;
typedef struct _faceEndIterator FaceEnd_BottomNodeIterator;


#endif
