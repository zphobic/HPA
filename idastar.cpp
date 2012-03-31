//-----------------------------------------------------------------------------
/** @file idastar.cpp
    @see idastar.h

    $Id: idastar.cpp,v 1.6 2002/12/09 22:10:19 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/idastar.cpp,v $
*/
//-----------------------------------------------------------------------------

#include "idastar.h"

#include <iostream>
#include <math.h>
#include <assert.h>
#include "util.h"

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

IDAStar::IDAStar()
    : m_statistics(createStatistics()),
      m_branchingFactor(m_statistics.get("branching_factor"))
{
}

StatisticsCollection IDAStar::createStatistics()
{
    StatisticsCollection collection;
    collection.create("aborted");
    collection.create("cpu_time");
    collection.create("path_cost");
    collection.create("branching_factor");
    collection.create("nodes_expanded");
    collection.create("nodes_visited");
    return collection;
}

bool IDAStar::findPath(const Environment& env, int start, int target)
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
    m_abortSearch = false;
    m_visitedNodes.resize(env.getNumberNodes());
    for (vector<char>::iterator i = m_visitedNodes.begin();
         i != m_visitedNodes.end(); ++i)
        *i = ' ';
    findPathIdaStar(start);
    m_statistics.get("aborted").add(m_abortSearch ? 1 : 0);
    double timeDiff =
        static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC;
    m_statistics.get("cpu_time").add(timeDiff);
    m_statistics.get("nodes_expanded").add(m_nodesExpanded);
    m_statistics.get("nodes_visited").add(m_nodesVisited);
    return true;
}

void IDAStar::findPathIdaStar(int start)
{
    const int maxFLimit = m_env->getNumberNodes() * m_env->getMaxCost();
    int heuristic = m_env->getHeuristic(start, m_target);
    m_fLimit = heuristic;
    int minCost = m_env->getMinCost();
    unsigned int expectedMaxDepth =
        static_cast<unsigned int>(heuristic / minCost);
    if (m_successorStack.size() < expectedMaxDepth)
        m_successorStack.resize(expectedMaxDepth);
    int iteration = 0;
    while (true)
    {
        cerr << "limit = " << m_fLimit << '\n';
        m_nextFLimit = INT_MAX;
        searchPathIdaStar(iteration, start, NO_NODE, 0, 0);
        if (m_path.size() > 0)
            break;
        if (m_nextFLimit > maxFLimit)
            break;
        m_fLimit = m_nextFLimit;
        ++iteration;
    }
}

const StatisticsCollection& IDAStar::getStatistics() const
{
    return m_statistics;
}

bool IDAStar::searchPathIdaStar(int iteration, int node, int lastNode,
                                int depth, int g)
{
    ++m_nodesVisited;
    char label;
    if (m_visitedNodes[node] == ' ')
    {
        if (iteration < 10)
            label = '0' + iteration;
        else if (iteration <= 36)
            label = 'a' + iteration - 10;
        else
            label = '+';
        m_visitedNodes[node] = label;
    }
    if (m_nodesLimit >= 0 && m_nodesVisited > m_nodesLimit)
    {
        m_abortSearch = true;
        return false;
    }
    int f = g + m_env->getHeuristic(node, m_target);
    if (f > m_fLimit)
    {
        if (f < m_nextFLimit)
        {
            m_nextFLimit = f;
        }
        return false;
    }
    if (node == m_target)
    {
        m_statistics.get("path_cost").add(f);
        m_path.push_back(node);
        return true;
    }
    ++m_nodesExpanded;
    assert(depth >= 0);
    if (m_successorStack.size() < static_cast<unsigned int>(depth + 1))
        m_successorStack.resize(depth + 1);
    vector<Environment::Successor>& successors = m_successorStack[depth];
    m_env->getSuccessors(node, lastNode, successors);
    int numberSuccessors = successors.size();
    m_branchingFactor.add(numberSuccessors);
    //    cout << "Generated " << numberSuccessors << " successors\n";
    for (int i = 0; i < numberSuccessors; ++i)
    {
        // Get reference on edge again, because resize could have changed it.
        const Environment::Successor& successor = m_successorStack[depth][i];
        int targetNodeId = successor.m_target;
        if (targetNodeId == lastNode)
            continue;
        if (searchPathIdaStar(iteration, targetNodeId, node, depth + 1,
                              g + successor.m_cost))
        {
            m_path.push_back(node);
            return true;
        }
        if (m_abortSearch)
            return false;
    }
    return false;
}

//-----------------------------------------------------------------------------
