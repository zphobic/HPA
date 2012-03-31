//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ABSNODE_H
#define PATHFIND_ABSNODE_H

#include <math.h>
#include "pathfind.h"
#include "util.h"
#include "cluster.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    // implements edges in the abstract graph
    class AbsNode
    {
    public:
        AbsNode(int id, int clusterId, int row, int column, int center);
        AbsNode();
        ~AbsNode();

        int getClusterId() const
        {
            return m_clusterId;
        }
        int getNodeId() const
        {
            return m_id;
        }
        int getRow() const
        {
            return m_row;
        }
        int getColumn() const
        {
            return m_column;
        }
        int getCenter() const
        {
            return m_center;
        }

        void setLocalIdxCluster(int idx)
        {
            m_localIdxCluster = idx;
        }

        int getLocalIdxCluster() const
        {
            return m_localIdxCluster;
        }
        void setLevel(int level)
        {
            m_level = level;
        }
        int getLevel() const
        {
            return m_level;
        }

    private:
        int m_id;
        int m_level;
        int m_clusterId;
        int m_row;
        int m_column;
        int m_center;
        int m_localIdxCluster;
    };
}

//-----------------------------------------------------------------------------

#endif
