#include "cactusGlobalsPrivate.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic group functions.
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

int group_constructP(const void *o1, const void *o2) {
    return cactusMisc_nameCompare(end_getName((End *) o1), end_getName((End *) o2));
}

Group *group_construct(Flower *flower, Flower *nestedFlower) {
    Group *group;

    group = group_construct3(flower, flower_getName(nestedFlower), 0);
    group_updateContainedEnds(group);
    flower_setParentGroup(nestedFlower, group);
    return group;
}

Group *group_construct2(Flower *flower) {
    Group *group;

    group = group_construct3(flower, cactusDisk_getUniqueID(flower_getCactusDisk(flower)), 1);
    return group;
}

bool group_isLeaf(Group *group) {
    return group->leafGroup;
}

static int32_t returnsTrue(Event *event) {
    assert(event != NULL);
    return 1;
}

static void copyAdjacencies(Group *group, Flower *nestedFlower) {
    assert(flower_getParentGroup(nestedFlower) == group);
    Group_EndIterator *endIterator = group_getEndIterator(group);
    End *end;
    while ((end = group_getNextEnd(endIterator)) != NULL) {
        End *nestedEnd = flower_getEnd(nestedFlower, end_getName(end));
        assert(nestedEnd != NULL);
        Cap *cap, *adjacentCap, *nestedCap, *nestedAdjacentCap;
        End_InstanceIterator *capIterator = end_getInstanceIterator(end);
        while ((cap = end_getNext(capIterator)) != NULL) {
            adjacentCap = cap_getAdjacency(cap);
            if (adjacentCap != NULL) {
                nestedCap = end_getInstance(nestedEnd, cap_getName(cap));
                nestedAdjacentCap = flower_getCap(nestedFlower, cap_getName(
                        adjacentCap));
                assert(nestedCap != NULL);
                assert(nestedAdjacentCap != NULL);
                nestedAdjacentCap
                        = cap_getOrientation(adjacentCap)
                                == cap_getOrientation(nestedAdjacentCap) ? nestedAdjacentCap
                                : cap_getReverse(nestedAdjacentCap);
                assert(cap_getOrientation(cap));
                assert(cap_getOrientation(cap) == cap_getOrientation(nestedCap));
                assert(cap_getOrientation(adjacentCap) == cap_getOrientation(nestedAdjacentCap));
                assert(end_getFlower(cap_getEnd(nestedCap)) == nestedFlower);
                assert(end_getFlower(cap_getEnd(nestedAdjacentCap)) == nestedFlower);
                cap_makeAdjacent(nestedCap, nestedAdjacentCap);
            }
        }
        end_destructInstanceIterator(capIterator);
    }
    group_destructEndIterator(endIterator);
}

void group_makeNestedFlower(Group *group) {
    assert(group_isLeaf(group));
    group->leafGroup = 0;
    Flower *nestedFlower = flower_construct2(group_getName(group), flower_getCactusDisk(
            group_getFlower(group)));
    flower_setParentGroup(nestedFlower, group);
    eventTree_copyConstruct(flower_getEventTree(group_getFlower(group)), nestedFlower,
            returnsTrue);
    Group *nestedGroup = group_construct2(nestedFlower);
    //Add the ends to the nested flower.
    Group_EndIterator *endIterator = group_getEndIterator(group);
    End *end;
    while ((end = group_getNextEnd(endIterator)) != NULL) {
        assert(end_getOrientation(end));
        end_setGroup(end_copyConstruct(end, nestedFlower), nestedGroup);
    }
    group_destructEndIterator(endIterator);
    //Now add adjacencies between the caps, mirroring the parent adjacencies.
    copyAdjacencies(group, nestedFlower);
    assert(group_getTotalBaseLength(group) == flower_getTotalBaseLength(nestedFlower));
    //Now copy any chain to the ends..
    if(group_getLink(group) != NULL) {
        Link *link = group_getLink(group);
        Chain *nestedChain = chain_construct(nestedFlower);
        End *_3End = group_getEnd(nestedGroup, end_getName(link_get3End(link)));
        End *_5End = group_getEnd(nestedGroup, end_getName(link_get5End(link)));
        assert(_3End != NULL);
        assert(_5End != NULL);
        link_construct(_3End, _5End, nestedGroup, nestedChain);
    }
}

void group_updateContainedEnds(Group *group) {
    assert(!group_isLeaf(group));
    Flower *flower;
    Flower_EndIterator *iterator;
    End *end;
    End *end2;
    //wipe the slate clean.
    while (group_getEndNumber(group) != 0) {
        end_setGroup(group_getFirstEnd(group), NULL);
    }
    stSortedSet_destruct(group->ends);
    group->ends = stSortedSet_construct3(group_constructP, NULL);
    //now calculate the ends
    flower = group_getFlower(group);
    iterator = flower_getEndIterator(group_getNestedFlower(group));
    while ((end = flower_getNextEnd(iterator)) != NULL) {
        if ((end2 = flower_getEnd(flower, end_getName(end))) != NULL) {
            end_setGroup(end2, group);
        }
    }
    flower_destructEndIterator(iterator);
}

void group_addEnd(Group *group, End *end) {
    end = end_getPositiveOrientation(end);
    stSortedSet_insert(group->ends, end);
}

void group_destruct(Group *group) {
    //Detach from the parent flower.
    flower_removeGroup(group_getFlower(group), group);
    while (group_getEndNumber(group) != 0) {
        end_setGroup(group_getFirstEnd(group), NULL);
    }
    stSortedSet_destruct(group->ends);
    //Free the memory
    free(group);
}

Flower *group_getFlower(Group *group) {
    return group->flower;
}

Name group_getName(Group *group) {
    return group->name;
}

Flower *group_getNestedFlower(Group *group) {
    return group_isLeaf(group) ? NULL : cactusDisk_getFlower(flower_getCactusDisk(
            group_getFlower(group)), group->name);
}

Link *group_getLink(Group *group) {
    return group->link;
}

bool group_isTangle(Group *group) {
    return group_getLink(group) == NULL;
}

bool group_isLink(Group *group) {
    return group_getLink(group) != NULL;
}

End *group_getFirstEnd(Group *group) {
    return stSortedSet_getFirst(group->ends);
}

End *group_getEnd(Group *group, Name name) {
    static End end;
    static EndContents endContents;
    end.endContents = &endContents;
    endContents.name = name;
    return stSortedSet_search(group->ends, &end);
}

int32_t group_getEndNumber(Group *group) {
    return stSortedSet_size(group->ends);
}

Group_EndIterator *group_getEndIterator(Group *group) {
    return stSortedSet_getIterator(group->ends);
}

End *group_getNextEnd(Group_EndIterator *endIterator) {
    return stSortedSet_getNext(endIterator);
}

End *group_getPreviousEnd(Group_EndIterator *endIterator) {
    return stSortedSet_getPrevious(endIterator);
}

Group_EndIterator *group_copyEndIterator(Group_EndIterator *endIterator) {
    return stSortedSet_copyIterator(endIterator);
}

void group_destructEndIterator(Group_EndIterator *endIterator) {
    stSortedSet_destructIterator(endIterator);
}

int64_t group_getTotalBaseLength(Group *group) {
    Group_EndIterator *endIterator = group_getEndIterator(group);
    End *end;
    int64_t totalLength = 0;
    while ((end = group_getNextEnd(endIterator)) != NULL) {
        End_InstanceIterator *instanceIterator = end_getInstanceIterator(end);
        Cap *cap;
        while ((cap = end_getNext(instanceIterator)) != NULL) {
            cap = cap_getStrand(cap) ? cap : cap_getReverse(cap);
            if (!cap_getSide(cap) && cap_getSequence(cap) != NULL) {
                Cap *cap2 = cap_getAdjacency(cap);
                assert(cap2 != NULL);
                assert(cap_getStrand(cap2));
                assert(cap_getSide(cap2));
                assert(end_getGroup(cap_getEnd(cap2)) == group);
                int32_t length = cap_getCoordinate(cap2) - cap_getCoordinate(
                        cap) - 1;
                assert(length >= 0);
                totalLength += length;
            }
        }
        end_destructInstanceIterator(instanceIterator);
    }
    group_destructEndIterator(endIterator);
    return totalLength;
}

/*static void group_mergeGroupsP(Flower *flower) {
 Flower_EndIterator *endIterator = flower_getEndIterator(flower);
 End *end;
 Group *group = group_construct2(flower);
 while((end = flower_getNextEnd(endIterator)) != NULL) {
 end_setGroup(end, group);
 }
 flower_destructEndIterator(endIterator);
 }

 Group *group_mergeGroups(Group *group1, Group *group2) {
 //Check they are in the same flower..
 assert(group_getFlower(group1) == group_getFlower(group2));
 assert(group1 != group2);
 if(group_getLink(group1) != NULL) { //we have to break these links..
 link_split(group_getLink(group1));
 }
 assert(group_getLink(group1) == NULL);
 if(group_getLink(group2) != NULL) {
 link_split(group_getLink(group2));
 }
 assert(group_getLink(group2) == NULL);

 if(!group_isTerminal(group1) || !group_isTerminal(group2)) { //We must first merge the nested flowers
 if(group_isTerminal(group1)) { //Need to make a nested flower to merge with the other
 group_makeNonTerminal(group1);
 group_mergeGroupsP(group_getNestedFlower(group1));
 assert(!group_isTerminal(group2));
 Flower *nestedFlower = group_getNestedFlower(group1), *otherFlower = group_getNestedFlower(group2);
 flower_setBuiltBlocks(nestedFlower, flower_builtBlocks(otherFlower));
 flower_setBuiltTrees(nestedFlower, flower_builtTrees(otherFlower));
 }
 if(group_isTerminal(group2)) { //Need to make a nested flower to merge with the other
 group_makeNonTerminal(group2);
 group_mergeGroupsP(group_getNestedFlower(group2));
 assert(!group_isTerminal(group1));
 Flower *nestedFlower = group_getNestedFlower(group2), *otherFlower = group_getNestedFlower(group1);
 flower_setBuiltBlocks(nestedFlower, flower_builtBlocks(otherFlower));
 flower_setBuiltTrees(nestedFlower, flower_builtTrees(otherFlower));
 }
 assert(group_getNestedFlower(group1) != NULL);
 assert(group_getNestedFlower(group2) != NULL);
 flower_mergeFlowersP(group_getNestedFlower(group1), group_getNestedFlower(group2));
 }
 End *end;
 while((end = group_getFirstEnd(group1)) != NULL) {
 end_setGroup(end, group2);
 }
 group_destruct(group1);

 #ifdef BEN_DEBUG
 if(!group_isTerminal(group2)) {
 assert(flower_getParentGroup(group_getNestedFlower(group2)) == group2);
 }
 #endif

 return group2;
 }
 */

void group_check(Group *group) {
    Flower *flower = group_getFlower(group);

    //Check flower and group properly connected.
    assert(flower_getGroup(flower, group_getName(group)) == group);

    Group_EndIterator *endIterator = group_getEndIterator(group);
    End *end;
    int32_t nonFree = 0;
    while ((end = group_getNextEnd(endIterator)) != NULL) {
        //That the ends of the groups are doubly linked to the ends (so every end is in only one link).
        assert(end_getGroup(end) == group);
        if (end_isAttached(end) || end_isBlockEnd(end)) {
            assert(end_isBlockEnd(end) || (end_isStubEnd(end) && end_isAttached(end)));
            nonFree++;
        }
    }
    group_destructEndIterator(endIterator);

    Link *link = group_getLink(group);
    if (nonFree == 2) {
        assert(link != NULL); // has only two non-free ends, is a link therefore
        assert(group_isLink(group));
    } else {
        assert(group_isTangle(group));
        assert(link == NULL); // can not be a link!
    }

    if (group_isLeaf(group)) { //If terminal has no nested flower
        assert(group_getNestedFlower(group) == NULL);
    } else { //else that any nested flower contains the correct set of stub ends.
        Flower *nestedFlower = group_getNestedFlower(group);
        assert(nestedFlower != NULL);
        endIterator = group_getEndIterator(group);
        while ((end = group_getNextEnd(endIterator)) != NULL) {
            End *end2 = flower_getEnd(nestedFlower, end_getName(end));
            assert(end2 != NULL);
            assert(end_isStubEnd(end2));
            if (end_isBlockEnd(end) || end_isAttached(end)) {
                end_isAttached(end2);
            } else {
                end_isFree(end2);
            }
        }
        group_destructEndIterator(endIterator);
    }
}

/*
 * Private functions.
 */

Group *group_construct3(Flower *flower, Name name, bool terminalGroup) {
    Group *group;
    group = st_malloc(sizeof(Group));

    group->flower = flower;
    group->link = NULL;
    group->name = name;
    group->ends = stSortedSet_construct3(group_constructP, NULL);
    group->leafGroup = terminalGroup;
    flower_addGroup(flower, group);

    return group;
}

void group_setLink(Group *group, Link *link) {
    //argument may be NULL
    group->link = link;
    if (link != NULL) {
        assert(group_getEnd(group, end_getName(link_get3End(link))) == link_get3End(link));
        assert(group_getEnd(group, end_getName(link_get5End(link))) == link_get5End(link));
    }
}

void group_removeEnd(Group *group, End *end) {
    assert(group_getEnd(group, end_getName(end)) == end);
    stSortedSet_remove(group->ends, end);
}

void group_setFlower(Group *group, Flower *flower) {
    flower_removeGroup(group_getFlower(group), group);
    group->flower = flower;
    flower_addGroup(flower, group);
    Flower *nestedFlower = group_getNestedFlower(group);
    if (nestedFlower != NULL) { //we re-do this link, because the parent flower has changed.
        flower_setParentGroup(nestedFlower, group);
    }
}

/*
 * Serialisation functions
 */

void group_writeBinaryRepresentation(Group *group, void(*writeFn)(
        const void * ptr, size_t size, size_t count)) {
    End *end;
    Group_EndIterator *iterator;

    binaryRepresentation_writeElementType(CODE_GROUP, writeFn);
    binaryRepresentation_writeBool(group_isLeaf(group), writeFn);
    binaryRepresentation_writeName(group_getName(group), writeFn);
    iterator = group_getEndIterator(group);
    while ((end = group_getNextEnd(iterator)) != NULL) {
        binaryRepresentation_writeElementType(CODE_GROUP_END, writeFn);
        binaryRepresentation_writeName(end_getName(end), writeFn);
    }
    group_destructEndIterator(iterator);
}

Group *group_loadFromBinaryRepresentation(void **binaryString, Flower *flower) {
    Group *group;

    group = NULL;
    if (binaryRepresentation_peekNextElementType(*binaryString) == CODE_GROUP) {
        binaryRepresentation_popNextElementType(binaryString);
        bool terminalGroup = binaryRepresentation_getBool(binaryString);
        Name name = binaryRepresentation_getName(binaryString);
        group = group_construct3(flower, name, terminalGroup);
        while (binaryRepresentation_peekNextElementType(*binaryString)
                == CODE_GROUP_END) {
            binaryRepresentation_popNextElementType(binaryString);
            end_setGroup(flower_getEnd(flower, binaryRepresentation_getName(
                    binaryString)), group);
        }
    }
    return group;
}

Group *group_getStaticNameWrapper(Name flowerName) {
    static Group group;
    group.name = flowerName;
    return &group;
}
