/*
 * pinchGraphManipulation.h
 *
 *  Created on: 11 Sep 2010
 *      Author: benedictpaten
 */

#ifndef PINCHGRAPHMANIPULATION_H_
#define PINCHGRAPHMANIPULATION_H_


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Methods for 'pinching' the graph.
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void pinchMergePiece(struct PinchGraph *graph,
                       struct Piece *piece1,
                       struct Piece *piece2,
                       stHash *vertexToAdjacencyComponentsHash,
                       stList *adjacencyComponentGraph,
                       int32_t adjacencyComponentOverlap);

void pinchMerge(struct PinchGraph *graph, struct PairwiseAlignment *pairwiseAlignment,
        void (*addFunction)(struct PinchGraph *pinchGraph, struct Piece *, struct Piece *, stHash *, stList *, int32_t, void *),
        void *extraParameter,
        stHash *vertexToAdjacencyComponentsHash, stList *adjacencyComponentGraph, int32_t adjacencyComponentOverlap);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Core methods for manipulating graph
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


void removeOverAlignedEdges(struct PinchGraph *pinchGraph, float minimumTreeCoverage, int32_t maxDegree, struct List *extraEdgesToUndo,
        int32_t extensionSteps, Flower *flower);

struct List *getRecursiveComponents(struct PinchGraph *pinchGraph, int32_t (*excludedEdgesFn)(void *));

struct List *getRecursiveComponents2(struct PinchGraph *pinchGraph, struct List *edgesToExclude);

/*
 * Ensure the graph is one connected component by linking components not part of the sink component
 * by there dead stub ends.
 */
void linkStubComponentsToTheSinkComponent(struct PinchGraph *pinchGraph, Flower *flower, int32_t attachEnds);

/*
 * Disconnects the dead ends of free stub ends from the sink component.
 */
void unlinkStubComponentsFromTheSinkComponent(struct PinchGraph *pinchGraph, Flower *flower);

void removeTrivialGreyEdgeComponents(struct PinchGraph *graph, struct List *listOfVertices, Flower *flower);

float treeCoverage(struct PinchVertex *vertex, Flower *flower);

#endif /* PINCHGRAPHMANIPULATION_H_ */