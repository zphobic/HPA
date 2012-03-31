//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#include <assert.h>
#include "smoothwizard.h"

#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"

using namespace std;
using namespace PathFind;

#define NO_INDEX 0

//-----------------------------------------------------------------------------

SmoothWizard::SmoothWizard(Tiling tiling, vector<int> path)
    :m_tiling(tiling, 0, 0, tiling.getWidth(), tiling.getHeight()),
     m_initPath(path),
     m_statistics(createStatistics())
{
    memset(m_pathMap, 0, sizeof(int)*1000000);//262144);
    for (unsigned int i = 0; i < m_initPath.size(); i++)
    {
        m_pathMap[m_initPath[i]] = i + 1;
    }
}

SmoothWizard::SmoothWizard()
    :m_tiling(Tiling(Tiling::TILE, 0, 0))
{

}

SmoothWizard::~SmoothWizard()
{

}


//-----------------------------------------------------------------------------

void SmoothWizard::smoothPath()
{
    clock_t startTime = clock();
    m_statistics.clear();
    if (m_tiling.getPathCost(m_initPath) == m_tiling.getHeuristic(m_initPath[0],
                                                                  m_initPath[m_initPath.size() - 1]))
    {
        m_smoothPath = m_initPath;
    }
    else
    {
        m_statistics.get("number_runs").add(1);
        for (unsigned int j = 0; j < m_initPath.size(); j++)
        {
            // add this node to the smoothed path
            if (m_smoothPath.size() == 0)
                m_smoothPath.push_back(m_initPath[j]);
            if (m_smoothPath.size() > 0 && m_smoothPath[m_smoothPath.size() - 1] != m_initPath[j])
                m_smoothPath.push_back(m_initPath[j]);
            for (int dir = NORTH; dir <= NW; dir++)
            {
                if (m_tiling.getType() == Tiling::TILE && dir > WEST)
                    break;
                int seenPathNode = getPathNodeId(m_initPath[j], dir);
                if (seenPathNode == NO_NODE)
                    continue;
                if (j > 0 && seenPathNode == m_initPath[j - 1])
                    continue;
                if (j < m_initPath.size() - 1 && seenPathNode == m_initPath[j + 1])
                    continue;
                int newLength = addPathPortion(m_initPath[j], seenPathNode, dir);
                int oldLength = COST_ONE*(m_pathMap[seenPathNode] - (j+1));
                j = m_pathMap[seenPathNode] - 2;
                // count the path reduction (e.g., 2)
                m_statistics.get("nr_improvements").add(1);
                m_statistics.get("improvement").add((oldLength - newLength));
                break;
            }
        }
    }

    double timeDiff =
        static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    m_statistics.get("cpu_time").add(timeDiff);
    m_statistics.get("path_cost").add(m_tiling.getPathCost(m_smoothPath));
}

int SmoothWizard::getPathNodeId(const int originId, int direction)
{
    int nodeId = originId;
    int lastNodeId = originId;
    while (true)
    {
        // advance in the given direction
        nodeId = advanceNode(nodeId, direction);
        if (nodeId == NO_NODE)
            return NO_NODE;
        if (!m_tiling.canJump(nodeId, lastNodeId))
            return NO_NODE;
        if (m_pathMap[nodeId] != NO_INDEX && m_pathMap[nodeId] > m_pathMap[originId])
        {
            return nodeId;
        }
        const TilingNodeInfo &newNodeInfo = m_tiling.getNodeInfo(nodeId);
        if (newNodeInfo.isObstacle())
        {
            return NO_NODE;
        }
        lastNodeId = nodeId;
    }
    return NO_NODE;
}

int SmoothWizard::advanceNode(int nodeId, int direction)
{
    const TilingNodeInfo &nodeInfo = m_tiling.getNodeInfo(nodeId);
    int currentRow = nodeInfo.getRow();
    int currentCol = nodeInfo.getColumn();
    switch(direction)
    {
    case NORTH:
        if (currentRow == 0)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow - 1, currentCol);
        break;
    case EAST:
        if (currentCol == m_tiling.getWidth() - 1)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow, currentCol + 1);
        break;
    case SOUTH:
        if (currentRow == m_tiling.getHeight() - 1)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow + 1, currentCol);
        break;
    case WEST:
        if (currentCol == 0)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow, currentCol - 1);
        break;
    case NE:
        if (currentRow == 0 || currentCol == m_tiling.getWidth() - 1)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow - 1, currentCol + 1);
        break;
    case SE:
        if (currentRow == m_tiling.getHeight() - 1 || currentCol == m_tiling.getWidth() - 1)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow + 1, currentCol + 1);
        break;
    case SW:
        if (currentRow == m_tiling.getHeight() - 1 || currentCol == 0)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow + 1, currentCol - 1);
        break;
    case NW:
        if (currentRow == 0 || currentCol == 0)
            return NO_NODE;
        return m_tiling.getNodeId(currentRow - 1, currentCol - 1);
        break;
    default:
        return NO_NODE;
    }
}

int SmoothWizard::addPathPortion(int originId, int finalId, int direction)
{
    int result = 0;
    int nodeId;
    int increaseRate = 0;
    if (direction <= WEST || m_tiling.getType() != Tiling::OCTILE_UNICOST)
        increaseRate += COST_ONE;
    else
        increaseRate += COST_SQRT2;

    for (nodeId = advanceNode(originId, direction); 
         nodeId != finalId; 
         nodeId = advanceNode(nodeId, direction))
    {
        m_smoothPath.push_back(nodeId);
        result += increaseRate;
    }
    m_smoothPath.push_back(finalId);
    result += increaseRate;
    return result;
}
//-----------------------------------------------------------------------------
// statistics

StatisticsCollection SmoothWizard::createStatistics()
{
    StatisticsCollection collection;
    collection.create("cpu_time");
    collection.create("nr_improvements");
    collection.create("improvement");
    collection.create("path_cost");
    collection.create("number_runs");
    return collection;
}

const StatisticsCollection& SmoothWizard::getStatistics() const
{
    return m_statistics;
}
