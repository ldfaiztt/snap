/*++

Module Name:

    mapq.h

Abstract:

    Support functions for mapping quality

Authors:

    Bill Bolosky, December, 2012

Environment:

    User mode service.

Revision History:

 
--*/

#pragma once

#include "SimilarityMap.h"

void initializeMapqTables();

double mapqToProbability(int mapq);

inline int computeMAPQ(
    double probabilityOfAllCandidates,
    double probabilityOfBestCandidate,
    int score,
    int firstPassSeedsNotSkipped,
    int firstPassRCSeedsNotSkipped,
    unsigned smallestSkippedSeed,
    unsigned smallestSkippedRCSeed,
    unsigned location,
    int popularSeedsSkipped,
    SimilarityMap *similarityMap,
    unsigned biggestClusterScored)
{
    _ASSERT(probabilityOfAllCandidates >= probabilityOfBestCandidate);
    _ASSERT(probabilityOfBestCandidate >= 0.0);

    /*
    //
    // Skipped seeds are ones that the aligner didn't look at because they had too many hits during the first
    // pass throygh the read (i.e., they're disjoint).  Any genome location that was ignored because of
    // maxHits could have at least score of this number (because it would have to have a difference in each
    // of them).  Assume that there are as many reads as the smallest of the sets at this edit distance
    // away from the read (i.e., assume the worst case).  Use a probability of .001 for migrating an edit
    // distance (this is, of course, just a guess since it really depends on the base call qualities, etc.,
    // but since we didn't look at the genome locations at all, this will have to do).
    //

    double probabilityOfSkippedLocations = 0.0;
    if (0xffffffff != smallestSkippedSeed) {
        probabilityOfSkippedLocations = pow(.001, firstPassSeedsNotSkipped) * smallestSkippedSeed;
    }
    if (0xffffffff != smallestSkippedRCSeed) {
        probabilityOfSkippedLocations += pow(.001, firstPassRCSeedsNotSkipped) * smallestSkippedRCSeed;
    }
    */

    double probabilityOfSkippedLocations = 0;

    double correctnessProbability = probabilityOfBestCandidate / (probabilityOfAllCandidates + probabilityOfSkippedLocations);
    int baseMAPQ;
    if (correctnessProbability >= 1) {
        baseMAPQ =  70;
    } else {
        baseMAPQ = __min(70, (int)(-10 * log10(1 - correctnessProbability)));
    }

    if (similarityMap != NULL) {
        int clusterSize = (int) similarityMap->getNumClusterMembers(location);
#ifdef TRACE_ALIGNER
        printf("Cluster size at %u: %d\n", location, clusterSize);
#endif
        //baseMAPQ = __max(0, baseMAPQ - clusterSize / 2000);
        //if (biggestClusterScored > 2 * clusterSize) {
        //    baseMAPQ = __max(0, baseMAPQ - biggestClusterScored / 2000);
        //}
        baseMAPQ = __max(0, baseMAPQ - biggestClusterScored / 4000);
    }

    //baseMAPQ = __max(0, baseMAPQ - popularSeedsSkipped);
    
    //if (score + 1 > __max(firstPassSeedsNotSkipped, firstPassRCSeedsNotSkipped)) {
    //    baseMAPQ = __max(0, baseMAPQ - 4 * (score + 1 - __max(firstPassSeedsNotSkipped, firstPassRCSeedsNotSkipped)));
    //}

    //
    // Apply a penalty based on the absolute difference between the read and the place it matched, as expressed
    // by its score.
    //
    baseMAPQ = __max(0, baseMAPQ - 2 * score);

#ifdef TRACE_ALIGNER
    printf("computeMAPQ called at %u: score %d, pThis %g, pAll %g, result %d\n",
            location, score, probabilityOfBestCandidate, probabilityOfAllCandidates, baseMAPQ);
#endif

    return baseMAPQ;
}
