/*
 * The script builds a cactus tree representation of the chains and nets.
 * The format of the output graph is dot format.
 */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "cactus.h"
#include "avl.h"
#include "commonC.h"
#include "hashTableC.h"

/*
 * Global variables.
 */
static double totalProblemSize;
static bool scaleNodeSizes = 1;
static bool nameLabels = 0;

static void usage() {
	fprintf(stderr, "cactus_treeViewer, version 0.2\n");
	fprintf(stderr, "-a --logLevel : Set the log level\n");
	fprintf(stderr, "-c --netDisk : The location of the net disk directory\n");
	fprintf(stderr, "-d --netName : The name of the net (the key in the database)\n");
	fprintf(stderr, "-e --outputFile : The file to write the dot graph file in.\n");
	fprintf(stderr, "-f --scaleNodeSizes : Scale the node sizes according to the volume of sequence they contained.\n");
	fprintf(stderr, "-g --nameLabels : Give chain and net nodes name labels.\n");
	fprintf(stderr, "-h --help : Print this help screen\n");
}

static void addNodeToGraph(const char *nodeName, FILE *graphFileHandle,
		double scalingFactor, const char *shape, const char *nodeLabel) {
    /*
     * Adds a node to the graph, scaling it's size.
     */

    double height = 1;
    double width = 0.25;
    scalingFactor *= 100.0;
    if(scaleNodeSizes && scalingFactor >= 1) {
        height = 4 * sqrt(scalingFactor);
        width = 1.0 * sqrt(scalingFactor);
    }
    graphViz_addNodeToGraph(nodeName, graphFileHandle, nameLabels ? nodeLabel : "", width, height, shape, "black", 14);
}

void makeCactusTree_terminalNode(Group *group, FILE *fileHandle, const char *parentNodeName, const char *parentEdgeColour) {
	char *groupNameString = netMisc_nameToString(group_getName(group));
	double scalingFactor = group_getTotalBaseLength(group)/totalProblemSize;
	assert(scalingFactor <= 1.001);
	assert(scalingFactor >= -0.001);
	addNodeToGraph(groupNameString, fileHandle,
			scalingFactor,
			"triangle", groupNameString);
	//Write in the parent edge.
	if(parentNodeName != NULL) {
		graphViz_addEdgeToGraph(parentNodeName, groupNameString, fileHandle, "", parentEdgeColour, 10, 1, "forward");
	}
	free(groupNameString);
}

void makeCactusTree_net(Net *net, FILE *fileHandle, const char *parentNodeName, const char *parentEdgeColour);

void makeCactusTree_chain(Chain *chain, FILE *fileHandle, const char *parentNodeName, const char *parentEdgeColour) {
	//Write the net nodes.
	char *chainNameString = netMisc_nameToString(chain_getName(chain));
	const char *edgeColour = graphViz_getColour();
	addNodeToGraph(chainNameString, fileHandle, chain_getAverageInstanceBaseLength(chain)/totalProblemSize, "box", chainNameString);
	//Write in the parent edge.
	if(parentNodeName != NULL) {
		graphViz_addEdgeToGraph(parentNodeName, chainNameString, fileHandle, "", parentEdgeColour, 10, 1, "forward");
	}
	//Create the linkers to the nested nets.
	int32_t i;
	for(i=0; i<chain_getLength(chain); i++) {
		Group *group = link_getGroup(chain_getLink(chain, i));
		assert(group != NULL);
		if(group_getNestedNet(group) != NULL) {
			makeCactusTree_net(group_getNestedNet(group),
					fileHandle, chainNameString, edgeColour);
		}
		else {
			makeCactusTree_terminalNode(group, fileHandle, chainNameString, edgeColour);
		}
	}
	free(chainNameString);
}

void makeCactusTree_net(Net *net, FILE *fileHandle, const char *parentNodeName, const char *parentEdgeColour) {
	//Write the net nodes.
	char *netNameString = netMisc_nameToString(net_getName(net));
	const char *edgeColour = graphViz_getColour();
	addNodeToGraph(netNameString, fileHandle, net_getTotalBaseLength(net)/totalProblemSize, "ellipse", netNameString);
	//Write in the parent edge.
	if(parentNodeName != NULL) {
		graphViz_addEdgeToGraph(parentNodeName, netNameString, fileHandle, "", parentEdgeColour, 10, 1, "forward");
	}
	//Create the chains.
	Net_ChainIterator *chainIterator = net_getChainIterator(net);
	Chain *chain;
	while((chain = net_getNextChain(chainIterator)) != NULL) {
		makeCactusTree_chain(chain, fileHandle, netNameString, edgeColour);
	}
	net_destructChainIterator(chainIterator);

	//Create the diamond node
	char *diamondNodeNameString = malloc(sizeof(char)*(strlen(netNameString) + 2));
	sprintf(diamondNodeNameString, "z%s", netNameString);
	const char *diamondEdgeColour = graphViz_getColour();
	//Create all the groups linked to the diamond.
	Net_GroupIterator *groupIterator = net_getGroupIterator(net);
	Group *group;
	double size = 0.0; //get the size of the group organising node..
	int32_t nonTrivialGroupCount = 0;
	while((group = net_getNextGroup(groupIterator)) != NULL) {
		if(group_getLink(group) == NULL) {
			size += group_getTotalBaseLength(group);
			nonTrivialGroupCount++;
		}
	}
	net_destructGroupIterator(groupIterator);
	if(nonTrivialGroupCount) {
		addNodeToGraph(diamondNodeNameString, fileHandle, size/totalProblemSize, "diamond", "");
		graphViz_addEdgeToGraph(netNameString, diamondNodeNameString, fileHandle, "", edgeColour, 10, 1, "forward");
		groupIterator = net_getGroupIterator(net);
		while((group = net_getNextGroup(groupIterator)) != NULL) {
			if(group_getLink(group) == NULL) {
				if(group_getNestedNet(group) != NULL) { //linked to the diamond node.
					makeCactusTree_net(group_getNestedNet(group), fileHandle, diamondNodeNameString, diamondEdgeColour);
				}
				else {
					makeCactusTree_terminalNode(group, fileHandle, diamondNodeNameString, diamondEdgeColour);
				}
			}
		}
		net_destructGroupIterator(groupIterator);
	}
	free(netNameString);
	free(diamondNodeNameString);
}

int main(int argc, char *argv[]) {
	NetDisk *netDisk;
	Net *net;
	FILE *fileHandle;

	/*
	 * Arguments/options
	 */
	char * logLevelString = NULL;
	char * netDiskName = NULL;
	char * netName = NULL;
	char * outputFile = NULL;

	///////////////////////////////////////////////////////////////////////////
	// (0) Parse the inputs handed by genomeCactus.py / setup stuff.
	///////////////////////////////////////////////////////////////////////////

	while(1) {
		static struct option long_options[] = {
			{ "logLevel", required_argument, 0, 'a' },
			{ "netDisk", required_argument, 0, 'c' },
			{ "netName", required_argument, 0, 'd' },
			{ "outputFile", required_argument, 0, 'e' },
			{ "scaleNodeSizes", no_argument, 0, 'f' },
			{ "nameLabels", no_argument, 0, 'g' },
			{ "help", no_argument, 0, 'h' },
			{ 0, 0, 0, 0 }
		};

		int option_index = 0;

		int key = getopt_long(argc, argv, "a:c:d:e:fgh", long_options, &option_index);

		if(key == -1) {
			break;
		}

		switch(key) {
			case 'a':
				logLevelString = stringCopy(optarg);
				break;
			case 'c':
				netDiskName = stringCopy(optarg);
				break;
			case 'd':
				netName = stringCopy(optarg);
				break;
			case 'e':
				outputFile = stringCopy(optarg);
				break;
			case 'f':
				scaleNodeSizes = !scaleNodeSizes;
				break;
			case 'g':
				nameLabels = !nameLabels;
				break;
			case 'h':
				usage();
				return 0;
			default:
				usage();
				return 1;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// (0) Check the inputs.
	///////////////////////////////////////////////////////////////////////////

	assert(netDiskName != NULL);
	assert(netName != NULL);
	assert(outputFile != NULL);

	//////////////////////////////////////////////
	//Set up logging
	//////////////////////////////////////////////

	if(logLevelString != NULL && strcmp(logLevelString, "INFO") == 0) {
		setLogLevel(LOGGING_INFO);
	}
	if(logLevelString != NULL && strcmp(logLevelString, "DEBUG") == 0) {
		setLogLevel(LOGGING_DEBUG);
	}

	//////////////////////////////////////////////
	//Log (some of) the inputs
	//////////////////////////////////////////////

	logInfo("Net disk name : %s\n", netDiskName);
	logInfo("Net name : %s\n", netName);
	logInfo("Output graph file : %s\n", outputFile);

	//////////////////////////////////////////////
	//Load the database
	//////////////////////////////////////////////

	netDisk = netDisk_construct(netDiskName);
	logInfo("Set up the net disk\n");

	///////////////////////////////////////////////////////////////////////////
	// Parse the basic reconstruction problem
	///////////////////////////////////////////////////////////////////////////

	net = netDisk_getNet(netDisk, netMisc_stringToName(netName));
	logInfo("Parsed the top level net of the cactus tree to build\n");

	///////////////////////////////////////////////////////////////////////////
	// Build the graph.
	///////////////////////////////////////////////////////////////////////////

	totalProblemSize = net_getTotalBaseLength(net);
	fileHandle = fopen(outputFile, "w");
	graphViz_setupGraphFile(fileHandle);
	makeCactusTree_net(net, fileHandle, NULL, NULL);
	graphViz_finishGraphFile(fileHandle);
	fclose(fileHandle);
	logInfo("Written the tree to file\n");

	///////////////////////////////////////////////////////////////////////////
	// Clean up.
	///////////////////////////////////////////////////////////////////////////

	netDisk_destruct(netDisk);

	return 0;
}