//-----------------------------------------------------------------------------
/** @file tiling.h
    Search environment using tilings and obstacles.

    $Id: tiling.h,v 1.29 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_LOCALENTRANCE_H
#define PATHFIND_LOCALENTRANCE_H

#include <math.h>
#include "pathfind.h"
#include "util.h"

//-----------------------------------------------------------------------------

namespace PathFind
{

    class LocalEntrance
    {
    public:
        LocalEntrance();
        LocalEntrance(int nodeId, int absNodeId, int localIdx, int centerRow, int centerCol, int length);
        ~LocalEntrance();

        int getLength() const
        {
            return m_length;
        }

        int getAbsNodeId() const
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

        int getEntranceLocalIdx() const
        {
            return m_localIdx;
        }

        void setEntranceLocalIdx(int idx)
        {
            m_localIdx = idx;
        }

        int getAbsAbsNodeId() const
        {
            return m_absNodeId;
        }

    protected:
        int m_id; // id of the global abstract node
        int m_absNodeId;
        int m_localIdx; // local id
        int m_centerRow; // center row in local coordinates
        int m_centerCol; // center col in local coordinates
        int m_length; // length of the entrance
    };
}

//-----------------------------------------------------------------------------

#endif
