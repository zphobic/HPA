//-----------------------------------------------------------------------------
/** @file idastar.h
    IDA* search.

    Short desciption of IDA*:

    Each iteration is a depth-first search that keeps track of the cost
    f = g + h of each node generated. If a node is generated whose cost
    exceeds the threshold for that iteration, its path is cut off and
    the search backtracks. The cost threshold is initialized to the
    heuristic of the initial state and is increased in each iteration
    to the total cost of the lowest-cost node that was pruned during the
    previous iteration. The algorithm terminates when a goal state is
    reached whose total cost does not exceed the current threshold.

    $Id: idastar.h,v 1.4 2002/12/09 22:10:19 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/idastar.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_IDASTAR_H
#define PATHFIND_IDASTAR_H

#include "search.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    class IDAStar
        : public Search
    {
    public:
        IDAStar();

        StatisticsCollection createStatistics();

        bool findPath(const Environment& env, int start, int target);

        const vector<int>& getPath() const
        {
            return m_path;
        }

        const StatisticsCollection& getStatistics() const;

        const vector<char>& getVisitedNodes() const
        {
            return m_visitedNodes;
        }

    private:
        bool m_abortSearch;

        int m_target;

        int m_fLimit;

        int m_nextFLimit;

        const Environment* m_env;

        long long int m_nodesExpanded;

        long long int m_nodesVisited;

        vector<char> m_visitedNodes;

        vector<int> m_path;

        vector<vector<Environment::Successor> > m_successorStack;

        StatisticsCollection m_statistics;

        Statistics& m_branchingFactor;


        void findPathIdaStar(int start);

        bool searchPathIdaStar(int iteration, int node, int lastNode,
                               int depth, int g);
    };
}

//-----------------------------------------------------------------------------

#endif
