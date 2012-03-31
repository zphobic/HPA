//-----------------------------------------------------------------------------
/** @file astar.cpp
    @see astar.h

    $Id: astar.cpp,v 1.27 2003/05/22 18:50:08 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/astar.cpp,v $
*/
//-----------------------------------------------------------------------------

#include "astar.h"

#include <iostream>
#include <math.h>
#include <memory>
#include <assert.h>
#include "util.h"

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

static const int NO_COST = -1;

AStar::AStarNode::Compare::~Compare()
{
    ;
}

bool AStar::AStarNode::Compare::operator()(const AStarNode& node1,
                                           const AStarNode& node2)
{
    int f1 = node1.m_f;
    int f2 = node2.m_f;
    if (f1 != f2)
        return (f1 < f2);
    int g1 = node1.m_g;
    int g2 = node2.m_g;
    return (g1 > g2);
}

//-----------------------------------------------------------------------------

void AStar::AStarNode::print(ostream& ostrm) const
{
    ostrm << "id:" << m_nodeId << '(' << m_g << ',' << m_h << ',' << m_f 
          << ',' << m_parent << ')';
}

//-----------------------------------------------------------------------------

AStar::ClosedListBase::~ClosedListBase()
{
    ;
}

vector<int> AStar::ClosedListBase::constructPath(int start, int target)
{
    vector<int> result;
    int nodeId = target;
    while (true)
    {
        result.push_back(nodeId);
        if (nodeId == start)
            break;
        const AStarNode* node = search(nodeId);
        assert(node != 0);
        nodeId = node->m_parent;
    }
    assert(result.size() > 0);
    assert(*result.begin() == target);
    assert(*(result.end() - 1) == start);
    return result;
}

//-----------------------------------------------------------------------------

void AStar::ClosedList::add(const AStarNode& node)
{
    m_list.push_back(node);
}

void AStar::ClosedList::init(int maxNodeId)
{
    m_list.clear();
}

void AStar::ClosedList::print(ostream& ostrm) const
{
   ostrm << "Closed {\n";
   for (list<AStarNode>::const_iterator i = m_list.begin();
        i != m_list.end(); ++i) 
   {
       i->print(ostrm);
       ostrm << '\n';
   }   
   ostrm << "}\n";
}

void AStar::ClosedList::remove(int nodeId)
{
    for (list<AStarNode>::iterator i = m_list.begin();
         i != m_list.end(); ++i)
        if (i->m_nodeId == nodeId)
        {
            m_list.erase(i);
            return;
        }
}

const AStar::AStarNode* AStar::ClosedList::search(int nodeId) const
{
    for (list<AStarNode>::const_iterator i = m_list.begin();
         i != m_list.end(); ++i)
        if (i->m_nodeId == nodeId)
            return &(*i);
    return 0;
}

//-----------------------------------------------------------------------------

AStar::ClosedListPerfectHash::ClosedListPerfectHash()
    : m_maxNodeId(0)
{
    init(0);
}

void AStar::ClosedListPerfectHash::add(const AStarNode& node)
{
    int nodeId = node.m_nodeId;
    assert(nodeId >= 0);
    assert(nodeId < m_maxNodeId);
    assert(m_marker[nodeId] == false);
    m_marker[nodeId] = true;
    m_nodes[nodeId] = node;
}

void AStar::ClosedListPerfectHash::init(int maxNodeId)
{
    assert(maxNodeId >= 0);
    m_nodes.resize(maxNodeId);
    m_marker.clear();
    m_marker.resize(maxNodeId, false);
    m_maxNodeId = maxNodeId;
}

void AStar::ClosedListPerfectHash::remove(int nodeId)
{
    assert(nodeId >= 0);
    assert(nodeId < m_maxNodeId);
    assert(m_marker[nodeId] == true);
    m_marker[nodeId] = false;
}

const AStar::AStarNode* AStar::ClosedListPerfectHash::search(int nodeId) const
{
    assert(nodeId >= 0);
    assert(nodeId < m_maxNodeId);
    if (! m_marker[nodeId])
        return 0;
    const AStarNode* result = &m_nodes[nodeId];
    return result;
}

//-----------------------------------------------------------------------------

AStar::OpenQueue::OpenQueue()
{
    init(0);
}

void AStar::OpenQueue::init(int numberNodes)
{
    m_marker.clear();
    m_marker.resize(numberNodes, false);
    m_lookupTable.resize(numberNodes);
    m_numberNodes = numberNodes;
    m_nodes.clear();
}

void AStar::OpenQueue::insert(const AStarNode& node)
{
    int nodeId = node.m_nodeId;
    assert(nodeId < m_numberNodes);
    assert(m_marker[nodeId] == false);
    m_marker[nodeId] = true;
    NodeSetIterator pos = m_nodes.insert(node);
    m_lookupTable[nodeId] = pos;
}

AStar::AStarNode AStar::OpenQueue::pop()
{
    assert(! isEmpty());
    AStarNode result = *(m_nodes.begin());
    m_nodes.erase(m_nodes.begin());
    int nodeId = result.m_nodeId;
    assert(nodeId < m_numberNodes);
    assert(m_marker[nodeId] == true);
    m_marker[nodeId] = false;
    return result;
}

void AStar::OpenQueue::print(ostream& ostrm) const
{
   ostrm << "Open {\n"; 
   for (NodeSetConstIterator i = m_nodes.begin(); i != m_nodes.end(); ++i) 
   {
       i->print(ostrm);
       ostrm << '\n';
   }   
   ostrm << "}\n";
}

bool AStar::OpenQueue::remove(int nodeId)
{
    assert(nodeId >= 0);
    assert(nodeId < m_numberNodes);
    if (! m_marker[nodeId])
        return false;
    NodeSetIterator pos = m_lookupTable[nodeId];
    m_nodes.erase(pos);
    m_marker[nodeId] = false;
    return true;
}

const AStar::AStarNode* AStar::OpenQueue::search(int nodeId) const
{
    assert(nodeId >= 0);
    assert(nodeId < m_numberNodes);
    if (m_marker[nodeId])
        return &(*m_lookupTable[nodeId]);
    return 0;
}

//-----------------------------------------------------------------------------

AStar::AStar(bool usePerfectHashClosedList)
    : m_statistics(createStatistics()),
      m_branchingFactor(m_statistics.get("branching_factor"))
{
    if (usePerfectHashClosedList)
        m_closed.reset(new ClosedListPerfectHash());
    else
        m_closed.reset(new ClosedList());
}

StatisticsCollection AStar::createStatistics()
{
    StatisticsCollection collection;
    collection.create("cpu_time");
    collection.create("path_cost");
    collection.create("path_length");
    collection.create("branching_factor");
    collection.create("nodes_expanded");
    collection.create("nodes_visited");
    collection.create("open_length");
    //    collection.create("closed_length");
    collection.create("open_max");
    return collection;
}

const AStar::AStarNode* AStar::findNode(int nodeId)
{
    const AStarNode* result = 0;
    result = m_open.search(nodeId);
    if (result != 0)
        return result;
    result = m_closed->search(nodeId);
    return result;
}

bool AStar::findPath(const Environment& env, int start, int target)
{
    assert(env.isValidNodeId(start));
    assert(env.isValidNodeId(target));
    clock_t startTime = clock();
    m_statistics.clear();
    m_nodesExpanded = 0;
    m_nodesVisited = 0;
    m_env = &env;
    m_target = target;
    m_path.clear();
    m_visitedNodes.resize(env.getNumberNodes());
    for (vector<char>::iterator i = m_visitedNodes.begin();
         i != m_visitedNodes.end(); ++i)
        *i = ' ';
    findPathAStar(start);
    double timeDiff =
        static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    m_statistics.get("cpu_time").add(timeDiff);
    m_statistics.get("nodes_expanded").add(m_nodesExpanded);
    m_statistics.get("nodes_visited").add(m_nodesVisited);
    m_statistics.get("path_length").add(m_path.size());
    return true;
}

void AStar::findPathAStar(int start)
{
    int maxopen = 0;
    //    int closedsize = 0;
    int numberNodes = m_env->getNumberNodes();
    m_closed->init(numberNodes);
    m_open.init(numberNodes);
    int heuristic = m_env->getHeuristic(start, m_target);
    m_pathCost = NO_COST;
    AStarNode startNode(start, NO_NODE, 0, heuristic);
    m_open.insert(startNode);
    vector<Environment::Successor> successors;
    while (! m_open.isEmpty())
    {
        m_statistics.get("open_length").add(m_open.getSize());
        if (m_open.getSize() > maxopen)
             maxopen = m_open.getSize();
        //m_open.print(cout);
        AStarNode node = getBestNodeFromOpen();
        //cout << '[';  node.print(cout); cout << ']' << endl;
        if (node.m_nodeId == m_target)
        {
            finishSearch(start, node);
            return;
        }
        ++m_nodesExpanded;
        m_env->getSuccessors(node.m_nodeId, NO_NODE, successors);
        m_branchingFactor.add(successors.size());
        for (vector<Environment::Successor>::const_iterator i
                 = successors.begin(); i != successors.end(); ++i)
        {
            int newg = node.m_g + i->m_cost;
            int target = i->m_target;
            const AStarNode* targetAStarNode = findNode(target);
            if (targetAStarNode != 0)
            {
                if (newg >= targetAStarNode->m_g)
                    continue;
                if (! m_open.remove(target))
                    m_closed->remove(target);
            }
            int newHeuristic = m_env->getHeuristic(target, m_target);
            AStarNode newAStarNode(target, node.m_nodeId, newg, newHeuristic);
            m_open.insert(newAStarNode);
        }
        //        closedsize++;
        m_closed->add(node);
        //        m_statistics.get("closed_length").add(m_closed.size());
        //closed->print(cout);
    }
    m_statistics.get("open_max").add(maxopen);
}

void AStar::finishSearch(int start, const AStarNode& node)
{
    m_closed->add(node);
    m_path = m_closed->constructPath(start, m_target);
    m_pathCost = node.m_f;
    m_statistics.get("path_cost").add(m_pathCost);
}

AStar::AStarNode AStar::getBestNodeFromOpen()
{
    assert(! m_open.isEmpty());
    AStarNode result = m_open.pop();
    int nodeId = result.m_nodeId;
    assert(nodeId >= 0);
    ++m_nodesVisited;
    m_visitedNodes[nodeId] = '+';
    return result;
}

const StatisticsCollection& AStar::getStatistics() const
{
    return m_statistics;
}

//-----------------------------------------------------------------------------
