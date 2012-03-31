
//-----------------------------------------------------------------------------
// $Id: environment.cpp,v 1.1 2002/07/23 19:19:52 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/environment.cpp,v $
//-----------------------------------------------------------------------------

#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <sstream>

#include "abswizard.h"

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

AbsWizard::AbsWizard(const Tiling &tiling, int clusterSize, int maxLevel, EntranceStyle style)
    :m_tiling(tiling, 0, 0, tiling.getWidth(), tiling.getHeight()),
     m_absTiling(clusterSize, maxLevel, tiling.getHeight(), tiling.getWidth()),
     m_clusterSize(clusterSize),
     m_entranceStyle(style)
{
}

AbsWizard::~AbsWizard()
{
    ;
}

void AbsWizard::abstractMaze()
{
    createEntrancesAndClusters();
    createAbstractGraph();
}

void AbsWizard::createEntrancesAndClusters()
{
    // now build clusters
    int row = 0, col = 0, contor = 0;
    int entranceId = 0;
    int horizSize, vertSize;

    cerr << "Creating entrances and clusters...\n";
    m_absTiling.setType(m_tiling.getType());
    for (int j = 0; j < m_tiling.getHeight(); j+= m_clusterSize)
    {
        col = 0;
        for (int i = 0; i < m_tiling.getWidth(); i+= m_clusterSize)
        {
            horizSize = min(m_clusterSize,  m_tiling.getWidth() - i);
            vertSize = min(m_clusterSize, m_tiling.getHeight() - j);
            Cluster cluster(m_tiling, contor++, row, col, i, j, horizSize, vertSize);
            m_absTiling.addCluster(cluster);
            // add entrances
            if (j > 0 && j < m_tiling.getHeight())
                createHorizEntrances(i, i + horizSize - 1, j - 1, row - 1, col, &entranceId);
            if(i > 0 && i < m_tiling.getWidth())
                createVertEntrances(j, j + vertSize - 1, i - 1, row, col - 1, &entranceId);
//             if (m_absTiling.getType() == AbsTiling::ABSTRACT_OCTILE)
//             {
//                 if (j > 0 && j < m_tiling.getHeight())
//                     createDHEntrances(i, i + horizSize - 2, j - 1, row - 1, col, &entranceId);
//                 if(i > 0 && i < m_tiling.getWidth())
//                     createDVEntrances(j, j + vertSize - 2, i - 1, row, col - 1, &entranceId);
//             }
            col++;
        }
        row++;
    }
    // set the abstract size of the abstract tiling (e.g., # of cluster rows & cols)
//     m_absTiling.setRows(row);
//     m_absTiling.setColumns(col);
    m_absTiling.linkEntrancesAndClusters();
    m_absTiling.addAbsNodes();
    m_absTiling.computeClusterPaths();
}

void AbsWizard::createHorizEntrances(int start, int end, int latitude, int row, int col, int *lastId)
{
    int node1Id, node2Id;

    for (int i = start; i <= end; i++)
    {
        node1Id = m_tiling.getNodeId(latitude, i);
        node2Id = m_tiling.getNodeId(latitude + 1, i);
        const TilingNodeInfo &node1Info = m_tiling.getNodeInfo(node1Id);
        const TilingNodeInfo &node2Info = m_tiling.getNodeInfo(node2Id);
        // get the next communication spot
        if (node1Info.isObstacle() || node2Info.isObstacle())
        {
            continue;
        }
        // start building the entrance
        int entranceStart = i;
        while (true)
        {
            i++;
            if (i >= end)
                break;
            node1Id = m_tiling.getNodeId(latitude, i);
            node2Id = m_tiling.getNodeId(latitude + 1, i);
            const TilingNodeInfo &node1Info = m_tiling.getNodeInfo(node1Id);
            const TilingNodeInfo &node2Info = m_tiling.getNodeInfo(node2Id);
            if ((node1Info.isObstacle() || node2Info.isObstacle()) || i >= end)
                break;
        }
        if (m_entranceStyle == END_ENTRANCE && (i - entranceStart) > MAX_ENTRANCE_WIDTH)
        {
            // create two new entrances, one for each end
            Entrance entrance1((*lastId)++, -1, -1, latitude, entranceStart,
                               m_tiling.getNodeId(latitude, entranceStart),
                               m_tiling.getNodeId(latitude + 1, entranceStart),
                              row, col, 1, HORIZONTAL);
            m_absTiling.addEntrance(entrance1);
            Entrance entrance2((*lastId)++, -1, -1, latitude, (i - 1),
                               m_tiling.getNodeId(latitude, i - 1),
                               m_tiling.getNodeId(latitude + 1, i - 1),
                              row, col, 1, HORIZONTAL);
            m_absTiling.addEntrance(entrance2);
        }
        else
        {
            // create one entrance in the middle 
            Entrance entrance((*lastId)++, -1, -1, latitude, ((i - 1) + entranceStart)/2,
                              m_tiling.getNodeId(latitude, ((i - 1) + entranceStart)/2),
                              m_tiling.getNodeId(latitude + 1, ((i - 1) + entranceStart)/2),
                              row, col, (i - entranceStart), HORIZONTAL);
            m_absTiling.addEntrance(entrance);
        }
    }
}

void AbsWizard::createVertEntrances(int start, int end, int meridian, int row, int col, int *lastId)
{
    int node1Id, node2Id;

    for (int i = start; i <= end; i++)
    {
        node1Id = m_tiling.getNodeId(i, meridian);
        node2Id = m_tiling.getNodeId(i, meridian + 1);
        const TilingNodeInfo &node1Info = m_tiling.getNodeInfo(node1Id);
        const TilingNodeInfo &node2Info = m_tiling.getNodeInfo(node2Id);
        // get the next communication spot
        if (node1Info.isObstacle() || node2Info.isObstacle())
        {
            continue;
        }
        // start building the entrance
        int entranceStart = i;
        while (true)
        {
            i++;
            if (i >= end)
                break;
            node1Id = m_tiling.getNodeId(i, meridian);
            node2Id = m_tiling.getNodeId(i, meridian + 1);
            const TilingNodeInfo &node1Info = m_tiling.getNodeInfo(node1Id);
            const TilingNodeInfo &node2Info = m_tiling.getNodeInfo(node2Id);
            if ((node1Info.isObstacle() || node2Info.isObstacle()) || i >= end)
                break;
        }
        if (m_entranceStyle == END_ENTRANCE && (i - entranceStart) > MAX_ENTRANCE_WIDTH)
        {
            // create two entrances, one for each end
            Entrance entrance1((*lastId)++, -1, -1, entranceStart, meridian,
                               m_tiling.getNodeId(entranceStart, meridian),
                               m_tiling.getNodeId(entranceStart, meridian + 1),
                              row, col, 1, VERTICAL);
            m_absTiling.addEntrance(entrance1);
            Entrance entrance2((*lastId)++, -1, -1, (i - 1), meridian,
                               m_tiling.getNodeId(i - 1, meridian),
                               m_tiling.getNodeId(i - 1, meridian + 1),
                              row, col, 1, VERTICAL);
            m_absTiling.addEntrance(entrance2);
        }
        else
        {
            // create one entrance
            Entrance entrance((*lastId)++, -1, -1, ((i - 1) + entranceStart)/2, meridian,
                              m_tiling.getNodeId(((i - 1) + entranceStart)/2, meridian),
                              m_tiling.getNodeId(((i - 1) + entranceStart)/2, meridian + 1),
                              row, col, (i - entranceStart), VERTICAL);
            m_absTiling.addEntrance(entrance);
        }
    }
}

void AbsWizard::createDHEntrances(int start, int end, int latitude, int row, int col, int *lastId)
{
    int node11Id, node12Id, node21Id, node22Id;

    for (int i = start; i <= end; i++)
    {
        node11Id = m_tiling.getNodeId(latitude, i);
        node12Id = m_tiling.getNodeId(latitude, i + 1);
        node21Id = m_tiling.getNodeId(latitude + 1, i);
        node22Id = m_tiling.getNodeId(latitude + 1, i + 1);
        const TilingNodeInfo &node11Info = m_tiling.getNodeInfo(node11Id);
        const TilingNodeInfo &node12Info = m_tiling.getNodeInfo(node12Id);
        const TilingNodeInfo &node21Info = m_tiling.getNodeInfo(node21Id);
        const TilingNodeInfo &node22Info = m_tiling.getNodeInfo(node22Id);

        // get the next communication spot
        if (node11Info.isObstacle() && !node12Info.isObstacle() &&
            !node21Info.isObstacle() && node22Info.isObstacle())
        {
            // create one entrance in the middle 
            Entrance entrance((*lastId)++, -1, -1, latitude, i,
                              m_tiling.getNodeId(latitude, i + 1),
                              m_tiling.getNodeId(latitude + 1, i),
                              row, col, 2, HDIAG2);
            m_absTiling.addEntrance(entrance);
        }
        else if (!node11Info.isObstacle() && node12Info.isObstacle() &&
                 node21Info.isObstacle() && !node22Info.isObstacle())
        {
            // create one entrance in the middle 
            Entrance entrance((*lastId)++, -1, -1, latitude, i,
                              m_tiling.getNodeId(latitude, i),
                              m_tiling.getNodeId(latitude + 1, i + 1),
                              row, col, 2, HDIAG1);
            m_absTiling.addEntrance(entrance);
        }
    }
}

void AbsWizard::createDVEntrances(int start, int end, int meridian, int row, int col, int *lastId)
{
    int node11Id, node12Id, node21Id, node22Id;

    for (int i = start; i <= end; i++)
    {
        node11Id = m_tiling.getNodeId(i, meridian);
        node12Id = m_tiling.getNodeId(i, meridian + 1);
        node21Id = m_tiling.getNodeId(i + 1, meridian);
        node22Id = m_tiling.getNodeId(i + 1, meridian + 1);
        const TilingNodeInfo &node11Info = m_tiling.getNodeInfo(node11Id);
        const TilingNodeInfo &node12Info = m_tiling.getNodeInfo(node12Id);
        const TilingNodeInfo &node21Info = m_tiling.getNodeInfo(node21Id);
        const TilingNodeInfo &node22Info = m_tiling.getNodeInfo(node22Id);

        // get the next communication spot
        if (node11Info.isObstacle() && !node12Info.isObstacle() &&
            !node21Info.isObstacle() && node22Info.isObstacle())
        {
            // create one entrance in the middle 
            Entrance entrance((*lastId)++, -1, -1, i, meridian,
                              m_tiling.getNodeId(i + 1, meridian),
                              m_tiling.getNodeId(i, meridian + 1),
                              row, col, 2, VDIAG2);
            m_absTiling.addEntrance(entrance);
        }
        else if (!node11Info.isObstacle() && node12Info.isObstacle() &&
                 node21Info.isObstacle() && !node22Info.isObstacle())
        {
            // create one entrance in the middle 
            Entrance entrance((*lastId)++, -1, -1, i, meridian,
                              m_tiling.getNodeId(i, meridian),
                              m_tiling.getNodeId(i + 1, meridian + 1),
                              row, col, 2, VDIAG1);
            m_absTiling.addEntrance(entrance);
        }
    }
}

void AbsWizard::createAbstractGraph()
{
    m_absTiling.createGraph();
}

//-----------------------------------------------------------------------------
