//-----------------------------------------------------------------------------
// $Id: tiling.cpp,v 1.37 2002/11/26 23:39:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
//-----------------------------------------------------------------------------
#include <assert.h>
#include "abstiling.h"

#include <memory>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"

using namespace std;
using namespace PathFind;

#define MAXLINE 2048

#define MAX_EDGES 100 // (max number of entrances for one cluster) - 1
#define MAX_COST 200*COST_ONE // max cost of an edge == max path inside a cluster

//-----------------------------------------------------------------------------

AbsTilingNodeInfo::AbsTilingNodeInfo()
    : m_id(-1),
      m_level(-1),
      m_clusterId(-1),
      m_centerRow(-1),
      m_centerCol(-1),
      m_localIdxCluster(-1)
{
}

AbsTilingNodeInfo::AbsTilingNodeInfo(int id, int level,
                                 int clId,
                                 int centerRow, int centerCol, int centerId,
                                 int localIdxCluster)
    : m_id(id),
      m_level(level),
      m_clusterId(clId),
      m_centerRow(centerRow),
      m_centerCol(centerCol),
      m_centerId(centerId),
      m_localIdxCluster(localIdxCluster)
{
}

void AbsTilingNodeInfo::printInfo(ostream &o) const
{
    o << "id: " << m_id;
    o << "; level: " << m_level;
    o << "; cluster: " << m_clusterId;
    o << "; row: " << m_centerRow;
    o << "; col: " << m_centerCol;
    o << "; center: " << m_centerId;
    o << "; local idx: " << m_localIdxCluster;
    o << "\n";
}

void AbsTilingEdgeInfo::printInfo(ostream &o) const
{
    o << "cost: " << m_cost << "; level: " << m_level << "; inter: " << m_inter;
    o << "\n";
}

//-----------------------------------------------------------------------------

AbsTiling::AbsTiling(int clusterSize, int maxLevel, int rows, int columns)
    :m_clusterSize(clusterSize),
     m_maxLevel(maxLevel)
{
    auto_ptr<Search> fakeSearch;
    fakeSearch.reset(new AStar(false));
    for (int i = 0; i < MAX_LEVELS; i++)
    {
        m_storageStatistics[i] = createStorageStatistics();
        m_preStatistics[i] = fakeSearch->createStatistics();
        m_abMainSearchStatistics[i] = fakeSearch->createStatistics();
        m_abInterSearchStatistics[i] = fakeSearch->createStatistics();
        m_stStatistics[i] = fakeSearch->createStatistics();
        m_tgStatistics[i] = fakeSearch->createStatistics();
    }
    init(ABSTRACT_OCTILE, clusterSize, rows, columns);
}

AbsTiling::AbsTiling(LineReader& reader)
{
}

AbsTiling::AbsTiling()
{
}

void AbsTiling::clearStatistics()
{
    for (int i = 0; i < MAX_LEVELS; i++)
    {
        m_storageStatistics[i].clear();
        m_preStatistics[i].clear();
        m_abMainSearchStatistics[i].clear();
        m_abInterSearchStatistics[i].clear();
        m_stStatistics[i].clear();
        m_tgStatistics[i].clear();
    }
}

void AbsTiling::setType(Tiling::Type type)
{
    switch(type)
    {
    case Tiling::TILE:
        m_type = ABSTRACT_TILE;
        break;
    case Tiling::OCTILE:
        m_type = ABSTRACT_OCTILE;
        break;
    case Tiling::OCTILE_UNICOST:
        m_type = ABSTRACT_OCTILE_UNICOST;
        break;
    default:
        assert (false);
        break;
    }
}

StatisticsCollection AbsTiling::createStorageStatistics()
{
    StatisticsCollection collection;
    collection.create("nodes");
    collection.create("inter_edges");
    collection.create("intra_edges");
    return collection;
}

const StatisticsCollection& AbsTiling::getStorageStatistics(int level) const
{
    assert (0 <= level && level < MAX_LEVELS);
    return m_storageStatistics[level];
}

const StatisticsCollection& AbsTiling::getPreStatistics(int level) const
{
    assert (0 <= level && level < MAX_LEVELS);
    return m_preStatistics[level];
}

const StatisticsCollection& AbsTiling::getStStatistics(int level) const
{
    assert (0 <= level && level < MAX_LEVELS);
    return m_stStatistics[level];
}

const StatisticsCollection& AbsTiling::getTgStatistics(int level) const
{
    assert (0 <= level && level < MAX_LEVELS);
    return m_tgStatistics[level];
}

const StatisticsCollection& AbsTiling::getAbMainSearchStatistics(int level) const
{
    assert (0 <= level && level < MAX_LEVELS);
    return m_abMainSearchStatistics[level];
}

const StatisticsCollection& AbsTiling::getAbInterSearchStatistics(int level) const
{ 
    assert (0 <= level && level < MAX_LEVELS);
    return m_abInterSearchStatistics[level];
}

//-----------------------------------------------------------------------------
// methods for m_clusters and m_entrances

void AbsTiling::addCluster(const Cluster &cluster)
{
    assert((int)m_clusters.size() == cluster.getClusterId());
    m_clusters.push_back(cluster);
}

void AbsTiling::addEntrance(const Entrance &entrance)
{
    assert((int)m_entrances.size() == entrance.getEntranceId());
    m_entrances.push_back(entrance);
}

int AbsTiling::determineLevel(int row)
{
    int level = 1;
    if (row % m_clusterSize != 0)
        row++;
    assert(row % m_clusterSize == 0);
    int clusterRow = (row / m_clusterSize);
    while (clusterRow % 2 == 0 && level < m_maxLevel)
    {
        clusterRow /= 2;
        level++;
    }
    if (level > m_maxLevel)
        level = m_maxLevel;
    return level;
}

void AbsTiling::addAbsNodes()
{
    int nr_nodes[MAX_LEVELS];
    memset(nr_nodes, 0, sizeof(int)*MAX_LEVELS);
    int nodeId = 0;
    map<int,AbsNode> absNodes;
    absNodes.clear();
    m_nrAbsNodes = 0;
    for (unsigned int i = 0; i < m_entrances.size(); i++)
    {
        Entrance &entrance = m_entrances[i];
        Cluster &cluster1 = getCluster(entrance.getCluster1Id());
        Cluster &cluster2 = getCluster(entrance.getCluster2Id());

        int level;
        switch (entrance.getOrientation())
        {
        case HORIZONTAL:
            level = determineLevel(entrance.getCenter1Row());
            break;
        case VERTICAL:
            level = determineLevel(entrance.getCenter1Col());
            break;
        default:
            level = -1;
            assert (false);
            break;
        }

        // use absNodes as a local var to check quickly if a node with the same centerId
        // has been created before
        map<int,AbsNode>::iterator p = absNodes.find(entrance.getCenter1Id());
        if (p == absNodes.end())
        {
            m_absNodeIds[entrance.getCenter1Id()] = nodeId;
            AbsNode node(nodeId,
                         entrance.getCluster1Id(),
                         entrance.getCenter1Row(), entrance.getCenter1Col(),
                         entrance.getCenter1Id());
            node.setLevel(level);
            nr_nodes[level]++;
            absNodes[entrance.getCenter1Id()] = node;
            cluster1.addEntrance(LocalEntrance(entrance.getCenter1Id(),
                                               nodeId,
                                               -1, // real value set in addEntrance()
                                               entrance.getCenter1Row() - cluster1.getVertOrigin(),
                                               entrance.getCenter1Col() - cluster1.getHorizOrigin(),
                                               entrance.getLength()));
            absNodes[entrance.getCenter1Id()].setLocalIdxCluster(cluster1.getNrEntrances() - 1);
            nodeId++;
        }
        else
        {
            if (level > p->second.getLevel())
            {
                p->second.setLevel(level);
                nr_nodes[level]++;
                nr_nodes[p->second.getLevel()]--;
            }
        }
        p = absNodes.find(entrance.getCenter2Id());
        if (p == absNodes.end())
        {
            m_absNodeIds[entrance.getCenter2Id()] = nodeId;
            AbsNode node(nodeId,
                         entrance.getCluster2Id(),
                         entrance.getCenter2Row(), entrance.getCenter2Col(),
                         entrance.getCenter2Id());
            node.setLevel(level);
            nr_nodes[level]++;
            absNodes[entrance.getCenter2Id()] = node;
            cluster2.addEntrance(LocalEntrance(entrance.getCenter2Id(),
                                               nodeId,
                                               -1, // real value set in addEntrance()
                                               entrance.getCenter2Row() - cluster2.getVertOrigin(),
                                               entrance.getCenter2Col() - cluster2.getHorizOrigin(),
                                               entrance.getLength()));
            absNodes[entrance.getCenter2Id()].setLocalIdxCluster(cluster2.getNrEntrances() - 1);
            nodeId++;
        }
        else
        {
            if (level > p->second.getLevel())
            {
                p->second.setLevel(level);
                nr_nodes[level]++;
                nr_nodes[p->second.getLevel()]--;
            }
        }
    }
    // add nodes to the graph
    for (map<int,AbsNode>::const_iterator i = absNodes.begin();
         i != absNodes.end(); ++i)
    {
        AbsTilingNodeInfo node(i->second.getNodeId(), i->second.getLevel(), i->second.getClusterId(),
                               i->second.getRow(), i->second.getColumn(), i->second.getCenter(),
                               i->second.getLocalIdxCluster());
        m_graph.addNode(i->second.getNodeId(), node);
        m_nrAbsNodes++;
    }
    for (int i = 0; i < MAX_LEVELS; i++)
        m_storageStatistics[i].get("nodes").add(nr_nodes[i]);
}

void AbsTiling::linkEntrancesAndClusters()
{
    for (unsigned int i = 0; i < m_entrances.size(); i++)
    {
        Entrance &entrance = m_entrances[i];
        switch (entrance.getOrientation())
        {
        case HORIZONTAL:
        case HDIAG1:
        case HDIAG2:
            {
            int cluster1Id = getClusterId(entrance.getRow(), entrance.getCol());
            int cluster2Id = getClusterId(entrance.getRow() + 1, entrance.getCol());
            // update entrance
            entrance.setCluster1Id(cluster1Id);
            entrance.setCluster2Id(cluster2Id);
            }
            break;
        case VERTICAL:
        case VDIAG2:
        case VDIAG1:
            {
            int cluster1Id = getClusterId(entrance.getRow(), entrance.getCol());
            int cluster2Id = getClusterId(entrance.getRow(), entrance.getCol() + 1);
            entrance.setCluster1Id(cluster1Id);
            entrance.setCluster2Id(cluster2Id);
            }
            break;
        default:
            assert(false);
            break;
        }
    }
}

void AbsTiling::computeClusterPaths()
{
    cerr << "Computing internal cluster paths...\n";
    for (unsigned int i = 0; i < m_clusters.size(); i++)
    {
        Cluster &cluster = m_clusters[i];
        cluster.computePaths(m_preStatistics[0]);
    }
}

void AbsTiling::addOutEdge(int initNodeId, int destNodeId, int cost, int level, bool inter)
{
    m_graph.addOutEdge(initNodeId, destNodeId, AbsTilingEdgeInfo(cost, level, inter));
}

void AbsTiling::createEdges()
{
    // add cluster edges
    for (vector<Cluster>::const_iterator i = m_clusters.begin();
         i != m_clusters.end(); ++i)
    {
        for (int k = 0; k < i->getNrEntrances(); k++)
        for (int l = k + 1; l < i->getNrEntrances(); l++)
        {
            if (!i->areConnected(l, k))
                continue;
            addOutEdge(i->getGlobalAbsNodeId(k), i->getGlobalAbsNodeId(l), i->getDistance(l, k), 1, false);
            addOutEdge(i->getGlobalAbsNodeId(l), i->getGlobalAbsNodeId(k), i->getDistance(k, l), 1, false);
            m_storageStatistics[1].get("intra_edges").add(1);
        }
    }
    // add transition edges
    for (unsigned int i = 0; i < m_entrances.size(); i++)
    {
        Entrance &entrance = m_entrances[i];
        int level;
        switch (entrance.getOrientation())
        {
        case HORIZONTAL:
            level = determineLevel(entrance.getCenter1Row());
            break;
        case VERTICAL:
            level = determineLevel(entrance.getCenter1Col());
            break;
        default:
            level = -1;
            assert(false);
            break;
        }
        switch(m_type)
        {
        case ABSTRACT_TILE:
        case ABSTRACT_OCTILE_UNICOST:
            addOutEdge(m_absNodeIds[entrance.getCenter1Id()],
                       m_absNodeIds[entrance.getCenter2Id()], COST_ONE, level, true);
            addOutEdge(m_absNodeIds[entrance.getCenter2Id()],
                       m_absNodeIds[entrance.getCenter1Id()], COST_ONE, level, true);
            break;
            case ABSTRACT_OCTILE:
                {
                    int unit_cost;
                    switch (entrance.getOrientation())
                    {
                    case HORIZONTAL:
                    case VERTICAL:
                        unit_cost = COST_ONE;
                        break;
                    case HDIAG2:
                    case HDIAG1:
                    case VDIAG1:
                    case VDIAG2:
                        unit_cost = COST_SQRT2;
                        break;
                    default:
                        unit_cost = -1;
                        assert (false);
                        break;
                    }
                    addOutEdge(m_absNodeIds[entrance.getCenter1Id()],
                               m_absNodeIds[entrance.getCenter2Id()], unit_cost, level, true);
                    addOutEdge(m_absNodeIds[entrance.getCenter2Id()],
                               m_absNodeIds[entrance.getCenter1Id()], unit_cost, level, true);
                }
                break;
        default:
            break;
        }
        m_storageStatistics[level].get("inter_edges").add(1);
    }
}

void AbsTiling::createNodes()
{
//     for (map<int,AbsNode>::const_iterator i = m_absNodes.begin();
//          i != m_absNodes.end(); ++i)
//     {
//         AbsTilingNodeInfo node(i->second.getNodeId(), 1, i->second.getClusterId(),
//                                i->second.getRow(), i->second.getColumn(), i->second.getCenter(), -1);
//         m_graph.addNode(i->second.getNodeId(), node);
//     }
}

void AbsTiling::getNodeOutEdges(int nodeId, vector<AbsTilingEdge>& edges) const
{
        const AbsTilingNode& node = m_graph.getNode(m_absNodeIds[nodeId]);
        edges = node.getOutEdges();
}        

int AbsTiling::insertStal(int nodeId, int nodeRow, int nodeCol, int start)
{
    int clusterId = -1;
    int absNodeId = m_absNodeIds[nodeId];
    if (absNodeId != NO_NODE)
    {
        m_stalLevel[start] = m_graph.getNodeInfo(m_absNodeIds[nodeId]).getLevel();
        getNodeOutEdges(nodeId, m_stalEdges[start]);
        m_stalUsed[start] = true;
        return absNodeId;
    }
    m_stalUsed[start] = false;
    // identify the cluster
    for (vector<Cluster>::const_iterator i = m_clusters.begin();
         i != m_clusters.end(); ++i)
    {
        if (i->getVertOrigin() <= nodeRow && nodeRow < i->getVertOrigin() + i->getHeight() &&
            i->getHorizOrigin() <= nodeCol && nodeCol < i->getHorizOrigin() + i->getWidth())
        {
            clusterId = i->getClusterId();
            break;
        }
    }
    Cluster& cluster = m_clusters[clusterId];
    // create global entrance
    absNodeId = m_nrAbsNodes;
    // insert local entrance to cluster
    cluster.addEntrance(LocalEntrance(nodeId, absNodeId, -1,
                                      nodeRow - cluster.getVertOrigin(),
                                      nodeCol - cluster.getHorizOrigin(),
                                      1));
    // update paths
    cluster.updatePaths(cluster.getNrEntrances() - 1, m_stStatistics[0]);
    // create new node to the abstract graph
    m_graph.addNode(absNodeId,
                    AbsTilingNodeInfo(absNodeId, 1,
                                      clusterId,
                                      nodeRow,
                                      nodeCol, nodeId, 
                                      cluster.getNrEntrances() - 1));
    // add new edges to the abstract graph
    int l = cluster.getNrEntrances() - 1;
    for (int k = 0; k < cluster.getNrEntrances() - 1; k++)
    {
        if (!cluster.areConnected(l, k))
            continue;
        addOutEdge(cluster.getGlobalAbsNodeId(k), cluster.getGlobalAbsNodeId(l), 
                   cluster.getDistance(l, k), 1, false);
        addOutEdge(cluster.getGlobalAbsNodeId(l), cluster.getGlobalAbsNodeId(k), 
                   cluster.getDistance(k, l), 1, false);
        m_storageStatistics[1].get("intra_edges").add(1);
    }
    m_absNodeIds[nodeId] = m_nrAbsNodes;
    m_nrAbsNodes++;
    return absNodeId;
}

void AbsTiling::removeStal(int nodeId, int stal)
{
    if (m_stalUsed[stal])
    {
        AbsTilingNodeInfo nodeInfo = m_graph.getNodeInfo(nodeId);
        nodeInfo.setLevel(m_stalLevel[stal]);
        m_graph.removeNodeEdges(nodeId);
        m_graph.addNode(nodeId, nodeInfo);
        for (vector<AbsTilingEdge>::const_iterator i = m_stalEdges[stal].begin();
             i != m_stalEdges[stal].end(); ++i)
        {
            int targetNodeId = i->getTargetNodeId();
            assert(isValidNodeId(targetNodeId));
            addOutEdge(nodeId, targetNodeId, i->getInfo().getCost(),
                       i->getInfo().getLevel(), i->getInfo().getInter());
            addOutEdge(targetNodeId, nodeId, i->getInfo().getCost(),
                       i->getInfo().getLevel(), i->getInfo().getInter());
        }
    }
    else
    {
        assert(nodeId == m_nrAbsNodes - 1);
        const AbsTilingNodeInfo& currentNodeInfo = m_graph.getNodeInfo(nodeId);
        int clusterId = currentNodeInfo.getClusterId();
        Cluster& cluster = m_clusters[clusterId];
        cluster.removeLastEntranceRecord();
        m_absNodeIds[currentNodeInfo.getCenterId()] = NO_NODE;
        m_graph.removeNodeEdges(nodeId);
        m_graph.removeLastNode();
        m_nrAbsNodes--;
    }
}


void AbsTiling::createGraph()
{
    createNodes();
    createEdges();
}

int AbsTiling::localId2GlobalId(int localId, const Cluster& cluster, int cols) const
{
    int result;
    int localRow = localId/cluster.getWidth();
    int localCol = localId%cluster.getWidth();
    result = (localRow + cluster.getVertOrigin())*cols +
        (localCol + cluster.getHorizOrigin());
    return result;
}

int AbsTiling::globalId2LocalId(int globalId, const Cluster& cluster, int cols) const
{
    int globalRow = globalId/cols;
    int globalCol = globalId%cols;
    return (globalRow - cluster.getVertOrigin())*cluster.getWidth() +
        (globalCol - cluster.getHorizOrigin());
    return -1;
}

vector<char> AbsTiling::getCharVector() const
{
      vector<char> result;
      return result;
}

int AbsTiling::getHeuristic(int start, int target) const
{
    const AbsTilingNodeInfo& startNodeInfo = m_graph.getNodeInfo(start);
    const AbsTilingNodeInfo& targetNodeInfo = m_graph.getNodeInfo(target);
    int colStart = startNodeInfo.getCenterCol();
    int colTarget = targetNodeInfo.getCenterCol();
    int rowStart = startNodeInfo.getCenterRow();
    int rowTarget = targetNodeInfo.getCenterRow();
    int diffCol = abs(colTarget - colStart);
    int diffRow = abs(rowTarget - rowStart);

    switch (m_type)
    {
    case ABSTRACT_TILE:
        return (diffCol + diffRow)*COST_ONE;
        break;
    case ABSTRACT_OCTILE:
        {
        int diagonal = min(diffCol, diffRow);
        int straight = max(diffCol, diffRow) - diagonal;
        return straight*COST_ONE + diagonal*COST_SQRT2;
        }
        break;
    default:
        assert(false);
        return 0;
    }
}

int AbsTiling::getMaxCost() const
{
    return MAX_COST;
}

int AbsTiling::getMaxEdges()
{
    return MAX_EDGES;
}

int AbsTiling::getMinCost() const
{
    return 0;
    //  return COST_ONE;
}

int AbsTiling::getNumberNodes() const
{
    return m_nrAbsNodes;
}

void AbsTiling::getSuccessors(int nodeId, int lastNodeId,
                           vector<Successor>& result) const
{
//     assert(false);
//     result.reserve(getMaxEdges());
//     result.clear();
//     const AbsTilingNode& node = m_graph.getNode(nodeId);
//     const vector<AbsTilingEdge>& edges = node.getOutEdges();
//     for (vector<AbsTilingEdge>::const_iterator i = edges.begin();
//          i != edges.end(); ++i)
//     {
//         int targetNodeId = i->getTargetNodeId();
//         assert(isValidNodeId(targetNodeId));
//         if (lastNodeId != NO_NODE)
//             if (pruneNode(targetNodeId, nodeId, lastNodeId))
//                 continue;
//         result.push_back(Successor(targetNodeId, i->getInfo().getCost()));
//     }
}

void AbsTiling::init(AbsType type, int clusterSize, int rows, int columns)
{
    m_type = type;
    m_maxEdges = getMaxEdges();
    m_rows = rows;
    m_columns = columns;
    m_clusterSize = clusterSize;
    for (int i = 0; i < rows*columns; i++)
        m_absNodeIds[i] = NO_NODE;
}

bool AbsTiling::isValidNodeId(int nodeId) const
{
    return nodeId >= 0 && nodeId < getNumberNodes();
}

void AbsTiling::absPath2llPath(const vector<int> &absPath, vector<int>& result, int cols) const
{
}

void AbsTiling::absPath2llPath2(const vector<int> &absPath, vector<int>& result, int cols)
{
    result.resize(1);
    result.clear();
    if (absPath.size() > 0)
    {
        int lastAbsNodeId = -1;
        for (vector<int>::const_iterator i = absPath.begin();
             i != absPath.end(); ++i)
        {
            if (i == absPath.begin())
            {
                lastAbsNodeId = *i;
                continue;
            }
            int currentAbsNodeId = *i;
            const AbsTilingNodeInfo& currentNodeInfo = m_graph.getNodeInfo(currentAbsNodeId);
            const AbsTilingNodeInfo& lastNodeInfo = m_graph.getNodeInfo(lastAbsNodeId);

            int eClusterId = currentNodeInfo.getClusterId();
            int leClusterId = lastNodeInfo.getClusterId();
            int index2 = currentNodeInfo.getLocalIdxCluster();
            int index1 = lastNodeInfo.getLocalIdxCluster();
            if (eClusterId == leClusterId)
            {
                // insert the local solution into the global one
                Cluster &cluster = getCluster(eClusterId);
                if (cluster.getLocalCenter(index1) != cluster.getLocalCenter(index2))
                {
                vector<int> localPath = 
                    cluster.computePath(cluster.getLocalCenter(index1),
                                      cluster.getLocalCenter(index2),
                                      m_abInterSearchStatistics[0]);
                assert(localPath.size() > 1);
                for (vector<int>::const_iterator j = localPath.begin();
                     j != localPath.end(); ++j)
                {
                    int val = localId2GlobalId(*j, cluster, cols);
                    if (result[result.size() - 1] == val)
                    {
                        continue;
                    }
                    result.push_back(val);
                }
                }
            }
            else
            {
                int lastVal = lastNodeInfo.getCenterId();
                int currentVal = currentNodeInfo.getCenterId();
                if (result[result.size() - 1] != lastVal)
                    result.push_back(lastVal);
                result.push_back(currentVal);
            }
            lastAbsNodeId = currentAbsNodeId;
        }
    }
}


void AbsTiling::convertVisitedNodes(const vector<char> &absNodes, vector<char> &llVisitedNodes, int size)
{
    llVisitedNodes.resize(size);
    for (vector<char>::iterator i = llVisitedNodes.begin();
         i != llVisitedNodes.end(); ++i)
        *i = ' ';

    for (unsigned int i = 0; i < absNodes.size(); i++)
        if (absNodes[i] != ' ')
        {
            const AbsTilingNodeInfo& currentNodeInfo = m_graph.getNodeInfo(i);
            int currentAbsNodeId = currentNodeInfo.getCenterId();
            llVisitedNodes[currentAbsNodeId] = '+';
        }

}

void AbsTiling::printFormatted(ostream& o) const
{
    printFormatted(o, getCharVector());
}

void AbsTiling::printFormatted(ostream& o, const vector<char>& chars) const
{
}

void AbsTiling::printFormatted(ostream& o, int start, int target) const
{
    vector<char> chars = getCharVector();
    chars[start] = 'S';
    chars[target] = 'T';
    printFormatted(o, chars);
}

void AbsTiling::printFormatted(ostream& o, const vector<int>& path) const
{
    for (unsigned int i = 0; i < path.size(); i++)
        o << path[i] << " ";
    o << "\n";
}

void AbsTiling::printLabels(ostream& o, const vector<char>& labels) const
{
    assert(labels.size() == static_cast<unsigned int>(getNumberNodes()));
    vector<char> chars = getCharVector();
    vector<char>::iterator j = chars.begin();
    for (vector<char>::const_iterator i = labels.begin();
         i != labels.end(); ++i, ++j)
        if (*i != ' ')
            *j = *i;
    printFormatted(o, chars);
}


void AbsTiling::printPathAndLabels(ostream& o, const vector<int>& path,
                                const vector<char>& labels) const
{
    assert(labels.size() == static_cast<unsigned int>(getNumberNodes()));

    vector<char> chars = getCharVector();

    vector<char>::iterator j = chars.begin();
    for (vector<char>::const_iterator i = labels.begin();
         i != labels.end(); ++i, ++j)
        if (*i != ' ')
            *j = *i;

    if (path.size() > 0)
    {
        for (vector<int>::const_iterator i = path.begin();
             i != path.end(); ++i)
            chars[*i] = 'x';
        chars[*path.begin()] = 'T';
        chars[*(path.end() - 1)] = 'S';
    }

    printFormatted(o, chars);
}


bool AbsTiling::pruneNode(int targetNodeId, int nodeId, int lastNodeId) const
{
    const AbsTilingNodeInfo& targetNodeInfo = m_graph.getNodeInfo(targetNodeId);
    const AbsTilingNodeInfo& lastNodeInfo = m_graph.getNodeInfo(lastNodeId);
    int targetClId = targetNodeInfo.getClusterId();
    int lastClId = lastNodeInfo.getClusterId();
    // if target node is in the same cluster as last node
    if (targetClId == lastClId)
    {
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
