//-----------------------------------------------------------------------------
/** @file search.h
    Abstract search engine.

    $Id: search.h,v 1.20 2002/12/09 22:10:19 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/search.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_SEARCH_H
#define PATHFIND_SEARCH_H

#include <list>
#include <queue>
#include "environment.h"
#include "statistics.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    static const int NO_NODE = -1;

    /** Abstract search engine. */
    class Search
    {
    public:
        Search();

        virtual ~Search();

        /** Create a StatisticsCollection.
            Contains Statistics instances for all values tracked.
            Useful for keeping accumulated statistics by creating
            the collection and merging the statistics of a search
            as returned by getStatistics().
        */
        virtual StatisticsCollection createStatistics() = 0;

        /** Find a path.
            @return false, if search was aborted due to node limit.
        */
        virtual bool findPath(const Environment& env, int start,
                              int target) = 0;

        long long int getNodesLimit() const
        {
            return m_nodesLimit;
        }

        virtual const vector<int>& getPath() const = 0;

        /** Get a vector with char labels for each visited node.
            Space char means not visited, otherwise the char
            can have different values and meanings depending on
            the concrete search engine.
        */
        virtual const vector<char>& getVisitedNodes() const = 0;

        /** Get statistics of last search. */
        virtual const StatisticsCollection& getStatistics() const = 0;

        /** Set nodes limit for search engine.
            The default is -1 and means unlimited search.
        */
        void setNodesLimit(long long int nodesLimit)
        {
            m_nodesLimit = nodesLimit;
        }


    public:
        long long int m_nodesLimit;
    };
}

//-----------------------------------------------------------------------------

#endif
