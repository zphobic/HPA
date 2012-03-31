//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_HCLUSTER_H
#define PATHFIND_HCLUSTER_H

#include <math.h>
#include "pathfind.h"
#include "util.h"
#include "localentrance.h"
#include "statistics.h"

#define MAX_CLENTRANCES 50

//-----------------------------------------------------------------------------

namespace PathFind
{
    class HCluster
    {

    public:
        HCluster(int id, 
                int row, int col,
                int horizOrigin, int vertOrigin, 
                int width, int height);
        ~HCluster();

        int getClusterId() const
        {
            return m_id;
        }

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

    private:

    protected:
        int m_id;
        int m_row; // abstract row of this cluster (e.g., 1 for the second clusters horizontally)
        int m_column; // abstract col of this cluster (e.g., 1 for the second clusters vertically)
        int m_horizOrigin;
        int m_vertOrigin;
        int m_width; // width of this hierarchical cluster
        int m_height; // high of this hierarchical cluster
        vector<int> m_nodeIds;
    };
}

//-----------------------------------------------------------------------------

#endif
