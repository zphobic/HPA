//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#ifndef PATHFIND_SMOOTHWIZARD_H
#define PATHFIND_SMOOTHWIZARD_H

#include "pathfind.h"
#include "util.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    class SmoothWizard
    {
    public:
        typedef enum {
            NORTH,
            EAST,
            SOUTH,
            WEST,
            NE,
            SE,
            SW,
            NW} Direction;

        SmoothWizard(Tiling tiling, vector<int> path);
        SmoothWizard();
        ~SmoothWizard();
        void smoothPath();
        const vector<int>& getInitPath() const
        {
            return m_initPath;
        }
        const vector<int>& getSmoothPath() const
        {
            return m_smoothPath;
        }
        StatisticsCollection createStatistics();
        const StatisticsCollection& getStatistics() const;
    private:
        Tiling m_tiling;
        vector<int> m_initPath;
        vector<int> m_smoothPath;
        int m_pathMap[1000000];//262144];
        StatisticsCollection m_statistics;

    private:
        bool checkPathSequence();
        int getPathNodeId(const int origin, int direction);
        int advanceNode(int nodeId, int direction);
        int addPathPortion(int originId, int finalId, int direction);
    };

}

//-----------------------------------------------------------------------------

#endif
