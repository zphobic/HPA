//-----------------------------------------------------------------------------
// $Id: tiling.cpp,v 1.37 2002/11/26 23:39:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
//-----------------------------------------------------------------------------
#include <assert.h>
#include "htiling.h"

#include <memory>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"
#include <algorithm>

using namespace std;
using namespace PathFind;

//double m_succTime;

//-----------------------------------------------------------------------------
// constructors/desctructors

HTiling::HTiling(int clusterSize, int maxLevel, int rows, int columns)
{
    auto_ptr<Search> fakeSearch, f1;
    fakeSearch.reset(new AStar(false));
    f1.reset(new AStar(false));
    m_hSearchStatistics = f1->createStatistics();
    init(ABSTRACT_OCTILE, clusterSize, rows, columns);
    m_maxLevel = maxLevel;
    m_clusterSize = clusterSize;
    for (int i = 0; i < MAX_LEVELS; i++)
    {
        m_storageStatistics[i] = createStorageStatistics();
        m_preStatistics[i] = fakeSearch->createStatistics();
        m_abSearchStatistics[i] = fakeSearch->createStatistics();
        m_stStatistics[i] = fakeSearch->createStatistics();
        m_tgStatistics[i] = fakeSearch->createStatistics();
    }
    //    m_succTime = 0.0;
}

HTiling::HTiling()
{
    AbsTiling();
}

const StatisticsCollection& HTiling::getHSearchStatistics() const
{
    return m_hSearchStatistics;
}


//-----------------------------------------------------------------------------
// method override

void HTiling::insertEdges2AbstractGraph(int nodeId, int nodeRow, int nodeCol)
{

    AbsTilingNodeInfo& nodeInfo = m_graph.getNodeInfo(m_absNodeIds[nodeId]);
    nodeInfo.setLevel(m_maxLevel);
    for (int level = 2; level <= m_maxLevel; level++)
    {
        m_currentLevel = level - 1;
        setCurrentCluster(nodeId, level);
        // combine nodes on vertical edges:
        {
            for (int i2 = m_currentRow1; i2 <= m_currentRow2; i2++)
            for (int j2 = m_currentCol1; j2 <= m_currentCol2; j2++)
            {
                if (m_absNodeIds[i2*m_columns+j2] == NO_NODE)
                    continue;
                if (nodeId == i2*m_columns+j2)
                    continue;
                const AbsTilingNodeInfo& nodeInfo2 = m_graph.getNodeInfo(m_absNodeIds[i2*m_columns+j2]);
                if (nodeInfo2.getLevel() < level)
                    continue;
                {
                    AStar search(false);
                    search.findPath(*this, m_absNodeIds[nodeId], m_absNodeIds[i2*m_columns+j2]);
                    const StatisticsCollection& searchStatistics = search.getStatistics();
                    m_stStatistics[level - 1].add(searchStatistics);
                    if (search.getPathCost() >= 0)
                    {
                        addOutEdge(m_absNodeIds[nodeId],
                                   m_absNodeIds[i2*m_columns+j2],
                                   search.getPathCost(), level);
                        addOutEdge(m_absNodeIds[i2*m_columns+j2],
                                   m_absNodeIds[nodeId],
                                   search.getPathCost(), level);
                        m_storageStatistics[level].get("intra_edges").add(1);
                    }
                }
            }
        }
    }
}

int HTiling::insertSTAL(int nodeId, int nodeRow, int nodeCol)
{
    int result = insertNode2AbstractGraph(nodeId, nodeRow, nodeCol);
    insertEdges2AbstractGraph(nodeId, nodeRow, nodeCol);
    return result;
}

void HTiling::getSuccessors(int nodeId, int lastNodeId,
                           vector<Successor>& result) const
{
    result.reserve(getMaxEdges());
    result.clear();
    const AbsTilingNode& node = m_graph.getNode(nodeId);
    const vector<AbsTilingEdge>& edges = node.getOutEdges();
    for (vector<AbsTilingEdge>::const_iterator i = edges.begin();
         i != edges.end(); ++i)
    {
        if (i->getInfo().getLevel() != m_currentLevel)
            continue;
        int targetNodeId = i->getTargetNodeId();
        assert(isValidNodeId(targetNodeId));
        const AbsTilingNodeInfo& targetNodeInfo = m_graph.getNodeInfo(targetNodeId);
        if (targetNodeInfo.getLevel() < m_currentLevel)
            continue;
        if (!nodeInCurrentCluster(targetNodeInfo))
            continue;
        if (lastNodeId != NO_NODE)
            if (pruneNode(targetNodeId, nodeId, lastNodeId))
                continue;
        result.push_back(Successor(targetNodeId, i->getInfo().getCost()));
    }
}


void HTiling::printSuccTime()
{
    //    cout << "get successors time: " << m_succTime << "\n";
}

//------------------------------------------------------------------------------
// set/get methods

void HTiling::setCurrentCluster(int nodeId, int level)
{
    if (level > m_maxLevel)
    {
        m_currentRow1 = 0;
        m_currentRow2 = m_rows - 1;
        m_currentCol1 = 0;
        m_currentCol2 = m_columns - 1;
        return;
    }
    int offset = getOffset(level);
    int nodeRow = nodeId/m_columns;
    int nodeCol = nodeId%m_columns;
    m_currentRow1 = nodeRow - (nodeRow%offset);
    m_currentRow2 = min(m_rows - 1, m_currentRow1 + offset - 1);
    m_currentCol1 = nodeCol - (nodeCol%offset);
    m_currentCol2 = min(m_columns - 1, m_currentCol1 + offset - 1);
}

bool HTiling::sameCluster(int node1Id, int node2Id, int level)
{
    const AbsTilingNodeInfo& node1Info = m_graph.getNodeInfo(node1Id);
    const AbsTilingNodeInfo& node2Info = m_graph.getNodeInfo(node2Id);
    int offset = getOffset(level);
    int node1Row = node1Info.getCenterRow();
    int node1Col = node1Info.getCenterCol();
    int node2Row = node2Info.getCenterRow();
    int node2Col = node2Info.getCenterCol();
    int currentRow1 = node1Row - (node1Row%offset);
    int currentRow2 = node2Row - (node2Row%offset);
    int currentCol1 = node1Col - (node1Col%offset);
    int currentCol2 = node2Col - (node2Col%offset);

    if (currentRow1 != currentRow2)
    {
        return false;
    }
    if (currentCol1 != currentCol2)
    {
        return false;
    }
    return true;
}

void HTiling::setCurrentCluster(int row, int col, int offset)
{
    m_currentRow1 = row;
    m_currentRow2 = max(m_rows, row + offset - 1);
    m_currentCol1 = col;
    m_currentCol2 = max(m_columns, col + offset - 1);
}

int HTiling::getHWidth(int level)
{
    int result;
    int offset = getOffset(level);
    result = m_columns/offset;
    if (m_columns%offset > 0)
        result++;
    return result;
}

int HTiling::getHHeight(int level)
{
    int result;
    int offset = getOffset(level);
    result = m_rows/offset;
    if (m_rows%offset > 0)
        result++;
    return result;
}

bool HTiling::nodeInCurrentCluster(const AbsTilingNodeInfo& nodeInfo) const
{
    int nodeRow = nodeInfo.getCenterRow();
    int nodeCol = nodeInfo.getCenterCol();
    if (nodeRow < m_currentRow1 || nodeRow > m_currentRow2)
    {
        return false;
    }
    if (nodeCol < m_currentCol1 || nodeCol > m_currentCol2)
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------
// building hierarchy

void HTiling::createHEdges()
{
    cerr << "Adding hierarchical edges";
    for (int level = 2; level <= m_maxLevel; level++)
    {
        cerr << " level " << level << "...";
        int offset = getOffset(level);
        m_currentLevel = level - 1;
        // for each cluster
        for (int row = 0; row < m_rows; row += offset)
        for (int col = 0; col < m_columns; col += offset)
        {
            setCurrentCluster(row, col, offset);
            // combine nodes on vertical edges:
            for (int i1 = row; i1 < row + offset; i1++)
            for (int j1 = col; j1 < col + offset; j1 += offset - 1)
            {
                if (m_absNodeIds[i1*m_columns+j1] == NO_NODE)
                    continue;
                const AbsTilingNodeInfo& nodeInfo1 = m_graph.getNodeInfo(m_absNodeIds[i1*m_columns+j1]);
                if (nodeInfo1.getLevel() < level)
                    continue;
                for (int i2 = row; i2 < row + offset; i2++)
                for (int j2 = col; j2 < col + offset; j2 += offset - 1)
                {
                    if (i1*m_columns+j1 >= i2*m_columns+j2)
                        continue;
                    if (m_absNodeIds[i2*m_columns+j2] == NO_NODE)
                        continue;
                    const AbsTilingNodeInfo& nodeInfo2 = m_graph.getNodeInfo(m_absNodeIds[i2*m_columns+j2]);
                    if (nodeInfo2.getLevel() < level)
                        continue;
                    {
                        AStar search(false);
                        search.findPath(*this, m_absNodeIds[i1*m_columns+j1], m_absNodeIds[i2*m_columns+j2]);
                        const StatisticsCollection& searchStatistics = search.getStatistics();
                        m_preStatistics[level - 1].add(searchStatistics);
                        if (search.getPathCost() >= 0)
                        {
                            addOutEdge(m_absNodeIds[i1*m_columns+j1],
                                       m_absNodeIds[i2*m_columns+j2],
                                       search.getPathCost(), level);
                            addOutEdge(m_absNodeIds[i2*m_columns+j2],
                                       m_absNodeIds[i1*m_columns+j1],
                                       search.getPathCost(), level);
                            m_storageStatistics[level].get("intra_edges").add(1);
                        }
                    }
                }
            }
            for (int i1 = row; i1 < row + offset; i1 += offset - 1)
            for (int j1 = col; j1 < col + offset; j1++)
            {
                if (m_absNodeIds[i1*m_columns+j1] == NO_NODE)
                    continue;
                const AbsTilingNodeInfo& nodeInfo1 = m_graph.getNodeInfo(m_absNodeIds[i1*m_columns+j1]);
                if (nodeInfo1.getLevel() < level)
                    continue;
                for (int i2 = row; i2 < row + offset; i2 += offset - 1)
                for (int j2 = col; j2 < col + offset; j2++)
                {
                    if (i1*m_columns+j1 >= i2*m_columns+j2)
                        continue;
                    if (m_absNodeIds[i2*m_columns+j2] == NO_NODE)
                        continue;
                    const AbsTilingNodeInfo& nodeInfo2 = m_graph.getNodeInfo(m_absNodeIds[i2*m_columns+j2]);
                    if (nodeInfo2.getLevel() < level)
                        continue;
                    {
                        AStar search(false);
                        search.findPath(*this, m_absNodeIds[i1*m_columns+j1], m_absNodeIds[i2*m_columns+j2]);
                        const StatisticsCollection& searchStatistics = search.getStatistics();
                        m_preStatistics[level - 1].add(searchStatistics);
                        if (search.getPathCost() >= 0)
                        {
                            addOutEdge(m_absNodeIds[i1*m_columns+j1],
                                       m_absNodeIds[i2*m_columns+j2],
                                       search.getPathCost(), level);
                            addOutEdge(m_absNodeIds[i2*m_columns+j2],
                                       m_absNodeIds[i1*m_columns+j1],
                                       search.getPathCost(), level);
                            m_storageStatistics[level].get("intra_edges").add(1);
                        }
                    }
                }
            }
            for (int i1 = row; i1 < row + offset; i1 += offset - 1)
            for (int j1 = col; j1 < col + offset; j1++)
            {
                if (m_absNodeIds[i1*m_columns+j1] == NO_NODE)
                    continue;
                const AbsTilingNodeInfo& nodeInfo1 = m_graph.getNodeInfo(m_absNodeIds[i1*m_columns+j1]);
                if (nodeInfo1.getLevel() < level)
                    continue;
                for (int i2 = row; i2 < row + offset; i2++)
                for (int j2 = col; j2 < col + offset; j2 += offset - 1)
                {
                    if (i1*m_columns+j1 == i2*m_columns+j2)
                        continue;
                    if (m_absNodeIds[i2*m_columns+j2] == NO_NODE)
                        continue;
                    const AbsTilingNodeInfo& nodeInfo2 = m_graph.getNodeInfo(m_absNodeIds[i2*m_columns+j2]);
                    if (nodeInfo2.getLevel() < level)
                        continue;
                    {
                        AStar search(false);
                        search.findPath(*this, m_absNodeIds[i1*m_columns+j1], m_absNodeIds[i2*m_columns+j2]);
                        const StatisticsCollection& searchStatistics = search.getStatistics();
                        m_preStatistics[level - 1].add(searchStatistics);
                        if (search.getPathCost() >= 0)
                        {
                            addOutEdge(m_absNodeIds[i1*m_columns+j1],
                                       m_absNodeIds[i2*m_columns+j2],
                                       search.getPathCost(), level);
                            addOutEdge(m_absNodeIds[i2*m_columns+j2],
                                       m_absNodeIds[i1*m_columns+j1],
                                       search.getPathCost(), level);
                            m_storageStatistics[level].get("intra_edges").add(1);
                        }
                    }
                }
            }
        }
    }
    cerr << "\n";
}

void HTiling::createGraph()
{
    createNodes();
    createEdges();
    createHEdges();
}

//-----------------------------------------------------------------------------

// hierarchical search
void HTiling::doSearch(int startNodeId, int targetNodeId, int level, vector<int>& result)
{
    auto_ptr<AStar> search;
    search.reset(new AStar((level == m_maxLevel)));
    m_currentLevel = level;
    const AbsTilingNodeInfo& nodeInfo = m_graph.getNodeInfo(startNodeId);
    setCurrentCluster(nodeInfo.getCenterId(), level + 1);
    search->findPath(*this, startNodeId, targetNodeId);
    const StatisticsCollection& searchStatistics = search->getStatistics();
    m_abSearchStatistics[level].add(searchStatistics);
    m_hSearchStatistics.add(searchStatistics);
    if (search->getPathCost() == -1)
    {
    }
    else
    {
        result = search->getPath();
        reverse(result.begin(), result.end());
    }
}

void HTiling::doHierarchicalSearch(int startNodeId, int targetNodeId, vector<int>& result)
{
    vector<int> tmppath, path;
    doSearch(startNodeId, targetNodeId, m_maxLevel, path);
    for (int level = m_maxLevel; level > 1; level--)
    {
        refineAbsPath(path, level, tmppath);
        path = tmppath;
    }
    result = path;
}

void HTiling::refineAbsPath(vector<int>& path, int level, vector<int>& result)
{
    result.clear();
    vector<int> tmp;
    // add first elem
    result.push_back(path[0]);
    for (unsigned int i = 0; i < path.size() - 1; i++)
    {
        if (sameCluster(path[i], path[i+1], level))
        {
            doSearch(path[i], path[i+1], level - 1, tmp);
            for (unsigned int k = 0; k < tmp.size(); k++)
            {
                if (result[result.size() - 1] != tmp[k])
                    result.push_back(tmp[k]);
            }
        }
    }
    // make sure last elem is added
    if (result[result.size() - 1] != path[path.size() - 1])
        result.push_back(path[path.size() - 1]);
}

void HTiling::printGraph(ostream& o)
{
    o << "Printing abstract graph:\n";
    for (int id = 0; id < m_nrAbsNodes; id++)
    {
        const vector<AbsTilingEdge>& edges = m_graph.getOutEdges(id);
        o << "Node " << id << "; BF " << edges.size() << "\n";
        const AbsTilingNodeInfo& nodeInfo = m_graph.getNodeInfo(id);
        nodeInfo.printInfo(o);
        for (vector<AbsTilingEdge>::const_iterator i = edges.begin();
             i != edges.end(); ++i)
        {
            o << "Edge to node " << i->getTargetNodeId() << ": ";
            i->getInfo().printInfo(o);
        }
        cout << "\n";
    }
}
