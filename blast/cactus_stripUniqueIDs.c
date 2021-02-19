#include <getopt.h>
#include "cactus.h"
#include "sonLib.h"
#include "bioioC.h"

static void usage(void)
{
    fprintf(stderr, "cactus_stripUniqueIDs --cactusDisk cactusDisk\n");
    fprintf(stderr, "Strips unique IDs that are temporarily prepended "
            "to sequence headers in the root flower of the cactus DB.\n");
}

int main(int argc, char *argv[])
{
    char *cactusDiskString = NULL;
    stKVDatabaseConf *kvDatabaseConf;
    CactusDisk *cactusDisk;
    Flower *flower;
    Flower_SequenceIterator *flowerIt;
    Sequence *sequence;
    struct option longopts[] = { {"cactusDisk", required_argument, NULL, 'c' },
                                 {0, 0, 0, 0} };
    int flag;
    while((flag = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch(flag) {
        case 'c':
            cactusDiskString = stString_copy(optarg);
            break;
        case '?':
        default:
            usage();
            return 1;
        }
    }
    if (cactusDiskString == NULL) {
        st_errAbort("--cactusDisk option must be provided");
    }
    kvDatabaseConf = stKVDatabaseConf_constructFromString(cactusDiskString);
    cactusDisk = cactusDisk_construct(kvDatabaseConf, false, true);
    // Get top-level flower.
    flower = cactusDisk_getFlower(cactusDisk, 0);
    flowerIt = flower_getSequenceIterator(flower);
    while((sequence = flower_getNextSequence(flowerIt)) != NULL) {
        const char *header;
        char *firstToken, *newHeader;
        stList *tokens;
        // Strip the ID token from the header (should be the first
        // |-separated token) and complain if there isn't one.
        header = sequence_getHeader(sequence);
        tokens = fastaDecodeHeader(header);
        assert(stList_length(tokens) > 1);
        firstToken = stList_removeFirst(tokens);
        assert(!strncmp(firstToken, "id=", 3)); // this first token will be id=
        free(firstToken);
        newHeader = fastaEncodeHeader(tokens);
        sequence_setHeader(sequence, newHeader);
        stList_destruct(tokens);
    }
    cactusDisk_write(cactusDisk);
}
