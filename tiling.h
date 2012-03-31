//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.32 2003/04/17 16:01:11 yngvi Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_TILING_H
#define PATHFIND_TILING_H

#include <math.h>
#include "pathfind.h"
#include "util.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    const int COST_ONE = 100;

    const int COST_SQRT2 = 142;

    class TilingEdgeInfo
    {
    public:
        TilingEdgeInfo(int cost)
            : m_cost(cost)
        {
            assert(cost == COST_ONE || cost == COST_SQRT2);
        }

        int getCost() const
        {
            return m_cost;
        }

    private:
        int m_cost;
    };
    
    class TilingNodeInfo
    {
    public:
        TilingNodeInfo();

        TilingNodeInfo(bool isObstacle, int row, int column);

        int getColumn() const
        {
            return m_column;
        }

        int getRow() const
        {
            return m_row;
        }

        bool isObstacle() const
        {
            return m_isObstacle;
        }

        void setObstacle(bool isObstacle)
        {
            m_isObstacle = isObstacle;
        }

    private:
        bool m_isObstacle;

        int m_column;

        int m_row;
    };

    class Tiling
        : public Environment
    {
    public:
        typedef enum {
            HEX,

            /** Octiles with cost 1 to adjacent and sqrt(2) to diagonal. */
            OCTILE,

            /** Octiles with uniform cost 1 to adjacent and diagonal. */
            OCTILE_UNICOST,

            TILE
        } Type;

        Tiling(Type type, int rows, int columns);

        Tiling(const Tiling & tiling, int horizOrigin, int vertOrigin, int width, int height);


        Tiling(LineReader& reader);

        void clearObstacles();

        int getHeuristic(int start, int target) const;

        int getMaxCost() const;

        int getMinCost() const;

        int getNodeId(int row, int column) const
        {
            //            cerr << row << " " << column << "\n";
            assert(row >= 0 && row < m_rows);
            assert(column >= 0 && column < m_columns);
            return row * m_columns + column;
        }

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

        void setObstacles(float obstaclePercentage, bool avoidDiag=false);

        // 17/01/2003 AdiB
        int getWidth() const
        {
            return m_columns;
        }

        int getHeight() const
        {
            return m_rows;
        }

        TilingNodeInfo & getNodeInfo(int nodeId) const
        {
            return (TilingNodeInfo &)m_graph.getNodeInfo(nodeId);
        }

        Type getType() const
        {
            return m_type;
        }

        int getPathCost(const vector<int> &path);

        bool canJump(int p1, int p2) const;

        const StatisticsCollection& getStorageStatistics() const;

        void clearStatistics();

        StatisticsCollection createStorageStatistics();

    private:
        typedef Graph<TilingNodeInfo, TilingEdgeInfo>::Edge TilingEdge;

        typedef Graph<TilingNodeInfo, TilingEdgeInfo> TilingGraph;

        typedef Graph<TilingNodeInfo, TilingEdgeInfo>::Node TilingNode;


        int m_columns;

        int m_maxEdges;

        int m_rows;

        Type m_type;

        TilingGraph m_graph;

        StatisticsCollection m_storageStatistics;

        void addOutEdge(int nodeId, int row, int col, int cost);

        bool conflictDiag(int row, int col, int roff, int coff );

        void createEdges();

        void createNodes();

        vector<char> getCharVector() const;

        static int getMaxEdges(Type type);

        void init(Type type, int rows, int columns);

        void printFormatted(ostream& o, const vector<char>& chars) const;

        bool pruneNode(int targetNodeId, int lastNodeId) const;

        void readObstacles(LineReader& reader);

        bool areAligned(int p1, int p2) const;

        void countRealEdges();

    };
}

//-----------------------------------------------------------------------------

#endif
