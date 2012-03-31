//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ABSTILING_H
#define PATHFIND_ABSTILING_H

#include <math.h>
#include "pathfind.h"
#include "util.h"
#include "cluster.h"
#include "absnode.h"

//-----------------------------------------------------------------------------

namespace PathFind
{

    static const int MAX_LEVELS = 6;
    // implements edges in the abstract graph
    class AbsTilingEdgeInfo
    {
    public:
        AbsTilingEdgeInfo(int cost, int level = 1, bool inter = true)
            : m_cost(cost),
              m_level(level),
              m_inter(inter)
        {
            ;
        }

        int getCost() const
        {
            return m_cost;
        }

        int getLevel() const
        {
            return m_level;
        }

        void setLevel(int level)
        {
            m_level = level;
        }

        bool getInter() const
        {
            return m_inter;
        }

        void printInfo(ostream& o) const;

    private:
        int m_cost;
        int m_level;
        bool m_inter;
    };

    // implements nodes in the abstract graph
    class AbsTilingNodeInfo
    {
    public:
        AbsTilingNodeInfo();

        AbsTilingNodeInfo(int id, int level, int clId,
                        int centerRow, int centerCol, int centerId,
                        int localIdxCluster);

        int getNodeId() const
        {
            return m_id;
        }

        int getCenterRow() const
        {
            return m_centerRow;
        }

        int getCenterCol() const
        {
            return m_centerCol;
        }

        int getClusterId() const
        {
            return m_clusterId;
        }

        int getCenterId() const
        {
            return m_centerId;
        }

        int getLevel() const
        {
            return m_level;
        }

        int getLocalIdxCluster() const
        {
            return m_localIdxCluster;
        }

        void printInfo(ostream& o) const;

        void setLevel(int level)
        {
            m_level = level;
        }

    private:
        int m_id;
        int m_level;
        int m_clusterId;
        int m_centerRow;
        int m_centerCol;
        int m_centerId;
        int m_localIdxCluster;
    };

    // implements an abstract maze decomposition
    // the ultimate abstract representation is a weighted graph of
    // locations connected by precomputed paths
    class AbsTiling
        : public Environment
    {
    public:
        typedef enum {
            ABSTRACT_TILE,
            ABSTRACT_OCTILE,
            ABSTRACT_OCTILE_UNICOST
        } AbsType;

        AbsTiling(int clusterSize, int maxLevel, int rows, int columns);

        AbsTiling(LineReader& reader);

        AbsTiling();

        int getHeuristic(int start, int target) const;

        int getMaxCost() const;

        int getMinCost() const;

        int getNumberNodes() const;

        void getSuccessors(int nodeId, int lastNodeId,
                           vector<Successor>& result) const;

        bool isValidNodeId(int nodeId) const;

        void printFormatted(ostream& o) const;

        void printFormatted(ostream& o, int start, int target) const;

        void printFormatted(ostream& o, const vector<int>& path) const;

        /** Print formatted map with char labels.
            Space characters are not printed on the map.
        */
        void printLabels(ostream& o, const vector<char>& labels) const;

        void printPathAndLabels(ostream& o, const vector<int>& path,
				const vector<char>& labels) const;

        void addCluster(const Cluster &cluster);

        void addEntrance(const Entrance &entrance);

        void createGraph();

        void linkEntrancesAndClusters(); // set clusters' entrances and viceversa

        void addAbsNodes();

        void computeClusterPaths();

        void setType(Tiling::Type type);

        void clearStatistics();

        AbsType getType()
        {
            return m_type;
        }

        void setRows(int rows)
        {
            m_rows = rows;
        }

        void setColumns(int cols)
        {
            m_columns = cols;
        }

        int getClusterId(int row, int col) const
        {
            assert(0 <= row && row <= m_rows && 0 <= col && col <= m_columns);
            int cols = (m_columns/m_clusterSize);
            if (m_columns%m_clusterSize)
                cols++;
            return row*cols + col;
        }

        void absPath2llPath(const vector<int> &absPath, vector<int>& result, int cols) const;

        void absPath2llPath2(const vector<int> &absPath, vector<int>& result, int cols);

        void convertVisitedNodes(const vector<char> &absNodes, vector<char> &llVisitedNodes, int size);

        // insert a new node, such as start or target, to the abstract graph and
        // returns the id of the newly created node in the abstract graph
        int insertStal(int nodeId, int nodeRow, int nodeCol, int stal);
        void removeStal(int nodeId, int stal);

        void printAbsNodes();

        const StatisticsCollection& getStorageStatistics(int level) const;

        const StatisticsCollection& getPreStatistics(int level) const;

        const StatisticsCollection& getStStatistics(int level) const;

        const StatisticsCollection& getTgStatistics(int level) const;

        const StatisticsCollection& getAbMainSearchStatistics(int level) const;

        const StatisticsCollection& getAbInterSearchStatistics(int level) const;
        
        StatisticsCollection createStorageStatistics();

    protected:
        typedef Graph<AbsTilingNodeInfo, AbsTilingEdgeInfo>::Edge AbsTilingEdge;

        typedef Graph<AbsTilingNodeInfo, AbsTilingEdgeInfo> AbsTilingGraph;

        typedef Graph<AbsTilingNodeInfo, AbsTilingEdgeInfo>::Node AbsTilingNode;

        int m_columns;

        int m_maxEdges;

        int m_rows;

        AbsType m_type;

        AbsTilingGraph m_graph;

        int m_stalLevel[2];

        bool m_stalUsed[2];

        vector<AbsTilingEdge> m_stalEdges[2];

        vector<Cluster> m_clusters; // used to build the m_graph member

        vector<Entrance> m_entrances; // same

        int m_nrAbsNodes;

        int m_absNodeIds[1000000];

        StatisticsCollection m_storageStatistics[MAX_LEVELS];

        StatisticsCollection m_preStatistics[MAX_LEVELS];

        StatisticsCollection m_abMainSearchStatistics[MAX_LEVELS];

        StatisticsCollection m_abInterSearchStatistics[MAX_LEVELS];

        StatisticsCollection m_stStatistics[MAX_LEVELS];

        StatisticsCollection m_tgStatistics[MAX_LEVELS];

        StatisticsCollection m_postStatistics;

        int m_clusterSize;

        int m_maxLevel;

        void addOutEdge(int initNodeId, int destNodeId, int cost, int level = 1, bool inter = false);

        void getNodeOutEdges(int nodeId, vector<AbsTilingEdge>& edges) const;

        void createEdges();

        void createNodes();

        vector<char> getCharVector() const;

        static int getMaxEdges();

        int determineLevel(int row);

        void init(AbsType type, int clusterSize, int rows, int columns);

        void printFormatted(ostream& o, const vector<char>& chars) const;

        bool pruneNode(int targetNodeId, int nodeId, int lastNodeId) const;

        Cluster& getCluster(int id)
        {
            assert (0 <= id && id < (int)m_clusters.size());
            return m_clusters[id];
        }

        int localId2GlobalId(int localId, const Cluster& cluster, int cols) const;

        int globalId2LocalId(int globalId, const Cluster& cluster, int cols) const;

    };
}

//-----------------------------------------------------------------------------

#endif
