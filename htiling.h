//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_HTILING_H
#define PATHFIND_HTILING_H

#include <math.h>
#include "pathfind.h"
#include "util.h"
#include "cluster.h"
#include "absnode.h"
#include "abstiling.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    // implements an abstract maze decomposition
    // the ultimate abstract representation is a weighted graph of
    // locations connected by precomputed paths
    class HTiling
        : public AbsTiling
    {
    public:

        HTiling(int clusterSize, int maxLevel, int rows, int columns);

        HTiling(LineReader& reader);

        HTiling();

        void getSuccessors(int nodeId, int lastNodeId,
                           vector<Successor>& result) const;

        void printSuccTime();

        void insertStalHEdges(int nodeId, int nodeRow, int nodeCol);

        int insertSTAL(int nodeId, int nodeRow, int nodeCol, int start);

        void printGraph(ostream& o);

        void createGraph();

        void doHierarchicalSearch(int startNodeId, int targetNodeId, vector<int>& result, int maxSearchLevel);

        void clearStatistics();

    protected:

        int m_currentLevel;

        int m_currentRow1;

        int m_currentRow2;

        int m_currentCol1;

        int m_currentCol2;

        //        double m_succTime;

        bool nodeInCurrentCluster(const AbsTilingNodeInfo& nodeInfo) const;

        bool pruneNode(int targetNodeId, int nodeId, int lastNodeId) const;

        int getOffset(int level) const
        {
            return m_clusterSize*(1 << (level - 1));
        }

        void setCurrentCluster(int nodeId, int level);

        void setCurrentCluster(int row, int col, int offset);

        int getHWidth(int level);

        int getHHeight(int level);

        bool sameCluster(int node1Id, int node2Id, int level) const;

        void doSearch(int startNodeId, int targetNodeId, int level, 
                      vector<int>& result, bool useTT);

        void refineAbsPath(vector<int>& path, int level, vector<int>& result);

        void createHEdges();
    
    };

}

//-----------------------------------------------------------------------------

#endif
