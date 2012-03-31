//-----------------------------------------------------------------------------
/** @file environment.h
    Search environment.

    $Id: environment.h,v 1.5 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/environment.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ABSWIZARD_H
#define PATHFIND_ABSWIZARD_H

#include <exception>
#include <string>
#include <vector>

#include "pathfind.h"
#include "htiling.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    const int MAX_ENTRANCE_WIDTH = 6;

    /** Interface to search environment. */
    class AbsWizard
    {
    public:

        typedef enum {MIDDLE_ENTRANCE, END_ENTRANCE} EntranceStyle;

        AbsWizard(const Tiling &tiling, int clusterSize, int maxLevel, EntranceStyle style);
        ~AbsWizard();

        void abstractMaze();

        HTiling& getAbsTiling()
        {
            return m_absTiling;
        }

        const Tiling& getTiling()
        {
            return m_tiling;
        }

        void setEntranceStyle(int style)
        {
            m_entranceStyle = style;
        }
        int getEntranceStyle() const
        {
            return m_entranceStyle;
        }

    protected:
        Tiling m_tiling;
        HTiling m_absTiling;
        int m_abstractionRate;
        int m_clusterSize;
        int m_entranceStyle;

        void createEntrancesAndClusters();
        void createAbstractGraph();
        void createHorizEntrances(int start, int end, int latitude, int row, int col, int *lastId);
        void createVertEntrances(int start, int end, int meridian, int row, int col, int *lastId);
        void createDHEntrances(int start, int end, int latitude, int row, int col, int *lastId);
        void createDVEntrances(int start, int end, int meridian, int row, int col, int *lastId);
    };
}

#endif
