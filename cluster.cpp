//-----------------------------------------------------------------------------
// $Id: tiling.cpp,v 1.37 2002/11/26 23:39:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
//-----------------------------------------------------------------------------
#include <assert.h>
#include "cluster.h"
#include <memory>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"
#include <algorithm>

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

Entrance::Entrance()
{}


Entrance::Entrance(int id, int cl1Id, int cl2Id, 
                   int center1Row, int center1Col, 
                   int center1Id, int center2Id,
                   int row, int col, int length,
                   Orientation orientation)
    :m_id(id),
     m_cluster1Id(cl1Id),
     m_cluster2Id(cl2Id),
     m_center1Row(center1Row),
     m_center1Col(center1Col),
     m_center1Id(center1Id),
     m_center2Id(center2Id),
     m_row(row),
     m_col(col),
     m_length(length),
     m_orientation(orientation)
{}

Entrance::~Entrance()
{}


Cluster::Cluster(const Tiling &tiling, int id, 
                 int row, int col,
                 int horizOrigin, int vertOrigin, int width, int height)
    : m_tiling(tiling, horizOrigin, vertOrigin, width, height),
      m_id(id),
      m_row(row),
      m_column(col),
      m_horizOrigin(horizOrigin),
      m_vertOrigin(vertOrigin),
      m_width(width),
      m_height(height)
{
}

Cluster::~Cluster()
{}

int Entrance::getCenter1Row() const
{
    if (m_orientation == VDIAG2)
        return m_center1Row + 1;
    else
        return m_center1Row;
}

int Entrance::getCenter1Col() const
{
    if (m_orientation == HDIAG2)
        return m_center1Col + 1;
    else
        return m_center1Col;
}

int Entrance::getCenter2Row() const
{
    switch (m_orientation)
    {
    case HORIZONTAL:
    case HDIAG1:
    case HDIAG2:
    case VDIAG1:
        return m_center1Row + 1;
        break;
    case VERTICAL:
    case VDIAG2:
        return m_center1Row;
        break;
    default:
        assert (false);
        return -1;
    }
    assert (false);
    return -1;
}

int Entrance::getCenter2Col() const
{
    switch (m_orientation)
    {
    case HORIZONTAL:
    case HDIAG2:
        return m_center1Col;
        break;
    case VERTICAL:
    case VDIAG2:
    case VDIAG1:
    case HDIAG1:
        return m_center1Col + 1;
        break;
    default:
        assert (false);
        return -1;
    }
    assert (false);
    return -1;
}

int Cluster::getEntranceCenter(const LocalEntrance& entrance)
{
    return entrance.getCenterRow()*m_width + entrance.getCenterCol();
}

int Cluster::getLocalCenter(int localIdx) const
{
    const LocalEntrance& entrance = m_entrances[localIdx];
    return entrance.getCenterRow()*m_width + entrance.getCenterCol();
}

void Cluster::computePaths(StatisticsCollection &statistics)
{
    for (int i = 0; i < MAX_CLENTRANCES; i++)
        for (int j = 0; j < MAX_CLENTRANCES; j++)
        {
            m_boolPathMap[i][j] = (char)0;
        }
    for (vector<LocalEntrance>::const_iterator i = m_entrances.begin();
         i != m_entrances.end(); ++i)
    for (vector<LocalEntrance>::const_iterator j = m_entrances.begin();
         j != m_entrances.end(); ++j)
     {
         assert (i->getEntranceLocalIdx() >= 0);
         assert (j->getEntranceLocalIdx() >= 0);
         assert (i->getEntranceLocalIdx() < MAX_CLENTRANCES);
         assert (j->getEntranceLocalIdx() < MAX_CLENTRANCES);
         computeAddPath(*i, *j, statistics);
     }
}

void Cluster::removeLastEntranceRecord()
{
    m_entrances.pop_back();
    int idx = m_entrances.size();
    for (int i = 0; i < MAX_CLENTRANCES; i++)
    {
        m_boolPathMap[idx][i] = m_boolPathMap[i][idx] = (char)0;
        m_distances[idx][i] = m_distances[i][idx] = -1;
    }
}

void Cluster::updatePaths(int entranceId, StatisticsCollection &statistics)
{
    const LocalEntrance& entrance = m_entrances[entranceId];
    for (vector<LocalEntrance>::const_iterator j = m_entrances.begin();
         j != m_entrances.end(); ++j)
     {
         computeAddPath(entrance, *j, statistics);
     }
}

void Cluster::computeAddPath(const LocalEntrance& e1, const LocalEntrance& e2, 
                             StatisticsCollection &statistics)
{
    int start = getEntranceCenter(e1);
    int target = getEntranceCenter(e2);
    int startIdx = e1.getEntranceLocalIdx();
    int targetIdx = e2.getEntranceLocalIdx();

    if (m_boolPathMap[startIdx][targetIdx])
        return;
    if (startIdx == targetIdx)
        return;
    assert(start != target);
//     if (start == target)
//     {
//         m_distances[startIdx][targetIdx] = 0;
//         m_distances[targetIdx][startIdx] = 0;
//     } else
    if (checkPathExists(start, target))
    {
        m_distances[startIdx][targetIdx] = 
        m_distances[targetIdx][startIdx] = 
        computeDistance(start, target, statistics);
    }
    else
    {
        addNoPath(startIdx, targetIdx);
    }
    m_boolPathMap[startIdx][targetIdx] = (char)1;
    m_boolPathMap[targetIdx][startIdx] = (char)1;
}

const vector<int>& Cluster::computePath(int start, int target,
                                        StatisticsCollection &statistics)
{
    auto_ptr<Search> search;
    search.reset(new AStar(false));
    search->findPath(m_tiling, target, start);
    const StatisticsCollection& searchStatistics = search->getStatistics();
    statistics.add(searchStatistics);
    m_workingPath = search->getPath();
//      if (m_id == 0 || m_id == 1)
//      {
//          cout << "path from " << start << " to " << target << "\n";
//          m_tiling.printFormatted(cout, m_workingPath);
//      }
    return m_workingPath;
}

int Cluster::computeDistance(int start, int target, 
                             StatisticsCollection& statistics)
{
    auto_ptr<AStar> search;
    search.reset(new AStar(false));
    search->setNodesLimit(1000000);
    search->findPath(m_tiling, target, start);
    const StatisticsCollection& searchStatistics = search->getStatistics();
    statistics.add(searchStatistics);
    return search->getPathCost();
}

const vector<int>& Cluster::buildPath(int start, int target)
{
    auto_ptr<Search> search;
    search.reset(new AStar(false));
    search->setNodesLimit(1000000);
    search->findPath(m_tiling, target, start);
    m_workingPath = search->getPath();
    return m_workingPath;
}

bool Cluster::checkPathExists(int start, int target)
{
    SearchUtils searchUtils;
    return searchUtils.checkPathExists(m_tiling, start, target);
}

void Cluster::addPath(const vector<int> &path, int startIdx, int targetIdx)
{
//     m_paths[startIdx][targetIdx] = path;
//     m_paths[targetIdx][startIdx] = path;
//     reverse(m_paths[targetIdx][startIdx].begin(),m_paths[targetIdx][startIdx].end());
//    m_distances[startIdx][targetIdx] = m_distances[targetIdx][startIdx] = m_tiling.getPathCost(path);
}

void Cluster::addNoPath(int startIdx, int targetIdx)
{
    m_distances[startIdx][targetIdx] = m_distances[targetIdx][startIdx] = s_Infinity;
}

//-----------------------------------------------------------------------------
