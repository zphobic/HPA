//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_CLUSTER_H
#define PATHFIND_CLUSTER_H

#include <math.h>
#include "pathfind.h"
#include "util.h"
#include "localentrance.h"
#include "statistics.h"

#define MAX_CLENTRANCES 50

//-----------------------------------------------------------------------------

static const int s_Infinity = 10000;

namespace PathFind
{

    typedef enum{HORIZONTAL, VERTICAL, HDIAG1, HDIAG2, VDIAG1, VDIAG2} Orientation;
    class Entrance
    {
    public:
        Entrance();
        Entrance(int id, int cl1Id, int cl2Id, 
                 int center1Row, int center1Col,
                 int center1Id, int center2Id,
                 int row, int col, int length,
                 Orientation orientation);
        ~Entrance();

        int getRow() const
        {
            return m_row;
        }

        int getCol() const
        {
            return m_col;
        }

        int getLength() const
        {
            return m_length;
        }

        int getEntranceId() const
        {
            return m_id;
        }

        int getCluster1Id() const
        {
            return m_cluster1Id;
        }

        int getCluster2Id() const
        {
            return m_cluster2Id;
        }

        int getCenter1Id() const
        {
            return m_center1Id;
        }

        int getCenter2Id() const
        {
            return m_center2Id;
        }

        int getCenter1Row() const;

        int getCenter1Col() const;

        int getCenter2Row() const;

        int getCenter2Col() const;
 
        Orientation getOrientation() const
        {
            return m_orientation;
        }

        void setCluster1Id(int id)
        {
            m_cluster1Id = id;
        }

        void setCluster2Id(int id)
        {
            m_cluster2Id = id;
        }

    protected:
        int m_id;
        int m_cluster1Id;
        int m_cluster2Id;
        int m_center1Row;
        int m_center1Col;
        int m_center1Id;
        int m_center2Id;
        int m_row; // abstract row of the leftmost adjacent cluster
        int m_col; // abstract col of the uppermost adjacent cluster
        int m_length; // length of the entrance
        Orientation m_orientation;
    };

    class Cluster
    {

    public:
        Cluster(const Tiling &tiling, int id, 
                int row, int col,
                int horizOrigin, int vertOrigin, 
                int width, int height);
        ~Cluster();

        int getClusterId() const
        {
            return m_id;
        }

        void computePaths(StatisticsCollection &statistics);

        void updatePaths(int entranceId, StatisticsCollection &statistics);

        int getGlobalAbsNodeId(int localIdx) const
        {
            assert(0 <= localIdx && (unsigned int)localIdx <= m_entrances.size());
            return m_entrances[localIdx].getAbsAbsNodeId();
        }

        int getDistance(int localIdx1, int localIdx2) const
        {
            assert(0 <= localIdx1 && (unsigned int)localIdx1 <= m_entrances.size());
            assert(0 <= localIdx2 && (unsigned int)localIdx2 <= m_entrances.size());
            return m_distances[localIdx1][localIdx2];
        }

        bool areConnected(int localIdx1, int localIdx2) const
        {
            assert(0 <= localIdx1 && (unsigned int)localIdx1 <= m_entrances.size());
            assert(0 <= localIdx2 && (unsigned int)localIdx2 <= m_entrances.size());
            return (m_distances[localIdx1][localIdx2] != s_Infinity);
        }

        int getNrEntrances() const
        {
            return m_entrances.size();
        }

        void addEntrance(const LocalEntrance &entrance)
        {
            m_entrances.push_back(entrance);
            m_entrances[m_entrances.size() - 1].setEntranceLocalIdx(m_entrances.size() - 1);
        }

        void removeLastEntranceRecord();

        int getHeight() const
        {
            return m_height;
        }

        int getWidth() const
        {
            return m_width;
        }

        int getHorizOrigin() const
        {
            return m_horizOrigin;
        }

        int getVertOrigin() const
        {
            return m_vertOrigin;
        }

        int getRow() const
        {
            return m_row;
        }

        int getCol() const
        {
            return m_column;
        }

//          const vector<int>& getPath(int idx1, int idx2) const
//          {
//             assert(m_distances[idx1][idx2] != s_Infinity);
//             return m_paths[idx1][idx2];
//          }

        int getLocalCenter(int localIndex) const;

        const vector<int>& computePath(int start, int target, StatisticsCollection &statistics);

        int computeDistance(int start, int target, StatisticsCollection& statistics);

        const vector<int>& buildPath(int start, int target);

    private:

        void addPath(const vector<int> &path, int start, int target);

        void addNoPath(int start, int target);

        int getPointId(int row, int col) const
        {
            return row*m_width + col;
        }

        int getEntranceCenter(const LocalEntrance& entrance);

        void computeAddPath(const LocalEntrance& e1, const LocalEntrance& e2,
                            StatisticsCollection &statistics);

        bool checkPathExists(int start, int target);

        vector<int> m_workingPath;

    protected:
        Tiling m_tiling;
        int m_id;
        int m_row; // abstract row of this cluster (e.g., 1 for the second clusters horizontally)
        int m_column; // abstract col of this cluster (e.g., 1 for the second clusters vertically)
        int m_horizOrigin;
        int m_vertOrigin;
        int m_width; // width of this cluster
        int m_height; // high of this cluster
        vector<LocalEntrance> m_entrances;
        //        vector<int> m_paths[MAX_CLENTRANCES][MAX_CLENTRANCES];
        short int m_distances[MAX_CLENTRANCES][MAX_CLENTRANCES];
        char m_boolPathMap[MAX_CLENTRANCES][MAX_CLENTRANCES];
    };
}

//-----------------------------------------------------------------------------

#endif
