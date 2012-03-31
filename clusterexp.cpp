//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#include <memory>
#include <assert.h>
#include "experiment.h"

#include <ctype.h>
#include <iostream>
#include <sstream>
#include "smoothwizard.h"
#include "abswizard.h"
#include "abstiling.h"
#include "util.h"

using namespace std;
using namespace PathFind;

int llLength, abLength;

//-----------------------------------------------------------------------------

typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

//-----------------------------------------------------------------------------

Experiment::Experiment(int nrRuns, long long int nodesLimit, int rows, int columns,
                       float obstaclePercentage, bool ll, bool ab,
                       SearchAlgorithm searchAlgorithm, Tiling::Type type)
    :m_nrRuns(nrRuns),
     m_nodesLimit(nodesLimit),
     m_rows(rows),
     m_columns(columns),
     m_obstaclePercentage(obstaclePercentage),
     m_ll(ll),
     m_ab(ab),
     m_searchAlgorithm(searchAlgorithm),
     m_tilingType(type)
{

}

void Experiment::addStats(AbStatistics &statistics)
{
    auto_ptr<Search> fakeSearch, abFakeSearch;
    fakeSearch.reset(new AStar());
    switch (m_searchAlgorithm)
    {
    case A_STAR:
        abFakeSearch.reset(new AStar());
        break;
    case IDA_STAR:
        abFakeSearch.reset(new IDAStar());
        break;
    }
    SmoothWizard fakeSmooth;
    statistics["pp"] = fakeSearch->createStatistics();
    statistics["st"] = fakeSearch->createStatistics();
    statistics["tg"] = fakeSearch->createStatistics();
    statistics["sm"] = fakeSmooth.createStatistics();
    statistics["ab"] = abFakeSearch->createStatistics();
}

void Experiment::addClusteringStats(int clusterSize)
{
    assert(clusterSize < m_rows);
    assert(clusterSize < m_columns);
    m_ClusteringStatistics[clusterSize] = m_tiling.createStatistics();
}

void Experiment::addClusterSizeStats(int clusterSize)
{
    assert(clusterSize < m_rows);
    assert(clusterSize < m_columns);
    addStats(m_AbStatistics[clusterSize]);
}

void Experiment::runExperiment()
{
    printHeader(m_searchAlgorithm, m_tilingType);
    SearchUtils searchUtils;
    for (int i = 0; i < m_nrRuns; i++)
    {
        // init the tile
        m_contor = i;
        Tiling tiling(m_tilingType, m_rows, m_columns);
        tiling.setObstacles(m_obstaclePercentage);
        int start, target;
        searchUtils.findRandomStartTarget(tiling, start, target);
        cerr << "\n" << m_contor;
        cerr << " (" << m_rows << "," << m_columns << "," << m_obstaclePercentage << ")\n";
        tiling.printFormatted(cerr, start, target);
        if (m_ll)
            runLlSearch(tiling, start, target, true);
        if (m_ab)
        {
            map<int,AbStatistics>::const_iterator p;
            for (p = m_AbStatistics.begin(); p != m_AbStatistics.end(); ++p)
            {
                int clusterSize = p->first;
                runAbSearch(tiling, start, target, true, clusterSize);
            }
        }
    }
    printStatistics();
}

void Experiment::runClusteringExperiment()
{
    SearchUtils searchUtils;
    for (int i = 0; i < m_nrRuns; i++)
    {
        // init the tile
        m_contor = i;
        Tiling tiling(m_tilingType, m_rows, m_columns);
        tiling.setObstacles(m_obstaclePercentage);
        cerr << "\n" << m_contor;
        cerr << " (" << m_rows << "," << m_columns << "," << m_obstaclePercentage << ")\n";
        map<int,AbStatistics>::const_iterator p;
        for (p = m_AbStatistics.begin(); p != m_AbStatistics.end(); ++p)
        {
            int clusterSize = p->first;
            AbsWizard wizard(tiling, clusterSize, MIDDLE_ENTRANCE);
            wizard.abstractMaze((StatisticsCollection&)p->second.find("pp")->second);
        }
    }
    printClusteringStatistics();
}

void Experiment::setupExperiment()
{
    auto_ptr<Search> fakeSearch;
    fakeSearch.reset(new AStar());
    m_llStatistics = fakeSearch->createStatistics();
    addClusterSizeStats(20);
//     addClusterSizeStats(15);
//     addClusterSizeStats(30);
//     addClusterSizeStats(45);
}

void Experiment::printHeader(SearchAlgorithm searchAlgorithm, Tiling::Type type)
{
    cerr << "==============================================================\n";
    switch (searchAlgorithm)
    {
    case A_STAR:
        cerr << "A_STAR";
        break;
    case IDA_STAR:
        cerr << "IDA_STAR";
        break;
    }
    cerr << "  ";
    switch (type)
    {
    case Tiling::TILE:
        cerr << "TILE";
        break;
    case Tiling::OCTILE:
        cerr << "OCTILE";
        break;
    case Tiling::OCTILE_UNICOST:
        cerr << "OCTILE_UNICOST";
        break;
    case Tiling::HEX:
        cerr << "HEX";
        break;
    }
    cerr << '\n';
    cerr << "==============================================================\n";
}

void Experiment::printStatistics()
{
    cout << "SEARCH SUMMARY AND STATISTICS:\n";
    cout << "Rows: " << m_rows << "; Columns: " << m_columns << "; Obstacle%: " << m_obstaclePercentage;
    cout << "; Number of runs: " << m_nrRuns << "\n";
    // low-level search
    cout << "LL\tLow-level search:\n";
    m_llStatistics.print(cout);
    // abstract search
    map<int,AbStatistics>::const_iterator p;
    for (p = m_AbStatistics.begin(); p != m_AbStatistics.end(); ++p)
    {
        int clusterSize = p->first;
        printAbStatistics(clusterSize);
        cout << "optimal: " << m_llStatistics.get("path_cost").getMean()/COST_ONE << "; ";
        cout << "\n";
    }
    cout << "\n";
}

void Experiment::printClusteringStatistics()
{
    cout << "CLUSTERING STATISTICS:\n";
    cout << "Rows: " << m_rows << "; Columns: " << m_columns << "; Obstacle%: " << m_obstaclePercentage;
    cout << "; Number of runs: " << m_nrRuns << "\n";
    cout << "Cluster size: " << "\n";
    cout << "Gates per cluster:\n";

    cout << "Paths per cluster: \n";

}

void Experiment::printAbStatistics(int clusterSize)
{
    cout << "AB\tAbstract framework with clusters of size " << clusterSize << "\n";
    AbStatistics& stats = getAbStats(clusterSize);
    cout << "AB\tAbstract search:\n";
    getAbSubStats(stats, "ab").print(cout);
    cout << "PP\tPre-processing search:\n";
    getAbSubStats(stats, "pp").print(cout);
    cout << "ST\tSearch for abstract start:\n";
    getAbSubStats(stats, "st").print(cout);
    cout << "TG\tSearch for abstract target:\n";
    getAbSubStats(stats, "tg").print(cout);
    cout << "SM\t Post-processing for path smoothing:\n";
    getAbSubStats(stats, "sm").print(cout);
    cout << "HF\t Average path in human readable format:\n";
    cout << "abstract: " << (getAbSubStats(stats, "ab").get("path_cost").getMean())/COST_ONE << "; ";
    cout << "abstract+smoothing: " << getAbSubStats(stats, "sm").get("path_cost").getMean()/COST_ONE << "; ";
}

AbStatistics& Experiment::getAbStats(const int clusterSize)
{
    map<int,AbStatistics>::iterator p = m_AbStatistics.find(clusterSize);
    if (p == m_AbStatistics.end())
    {
        ostringstream o;
        o << "Unknown statistics index " << clusterSize << '.';
        throw Error(o.str());
    }
    return p->second;
}

StatisticsCollection& Experiment::getAbSubStats(AbStatistics& abStats, const string& name)
{
    map<string,StatisticsCollection>::iterator p = abStats.find(name);
    if (p == abStats.end())
    {
        ostringstream o;
        o << "Unknown statistics name " << name << '.';
        throw Error(o.str());
    }
    return p->second;
}

StatisticsCollection& Experiment::getAbSubStats(const string &name, const int clusterSize)
{
    return getAbSubStats(getAbStats(clusterSize), name);
}

void Experiment::addStatistics(const string& name, const int clusterSize, StatisticsCollection stats)
{
    map<int,AbStatistics>::iterator p = m_AbStatistics.find(clusterSize);
    if (p == m_AbStatistics.end())
    {
        ostringstream o;
        o << "Unknown statistics index " << clusterSize << '.';
        throw Error(o.str());
    }
    map<string,StatisticsCollection>::iterator q = p->second.find(name);
    if (q == p->second.end())
    {
        ostringstream o;
        o << "Unknown statistics name " << name << '.';
        throw Error(o.str());
    }
    q->second.add(stats);
}

void Experiment::runLlSearch(Tiling &tiling, int start, int target, bool print)
{
    auto_ptr<Search> llSearch;
    switch (m_searchAlgorithm)
    {
    case A_STAR:
        llSearch.reset(new AStar());
        break;
    case IDA_STAR:
        llSearch.reset(new IDAStar());
        break;
    }
    llSearch->setNodesLimit(m_nodesLimit);
    llSearch->findPath(tiling, start, target);
    const vector<int>& path1 = llSearch->getPath();
    llLength = tiling.getPathCost(path1);
    if (print && m_contor % 5 == 0)
    {
        cout << "\n" << m_contor << ":\n";
        tiling.printPathAndLabels(cout, path1, llSearch->getVisitedNodes());
        cout << "\n";
    }
    const StatisticsCollection& searchLlStatistics = llSearch->getStatistics();
    m_llStatistics.add(searchLlStatistics);
}


void Experiment::runAbSearch(Tiling &tiling, 
                             int start, int target, 
                             bool print, int clusterSize)
{
    auto_ptr<Search> abSearch;
    switch (m_searchAlgorithm)
    {
    case A_STAR:
        abSearch.reset(new AStar());
        break;
    case IDA_STAR:
        abSearch.reset(new IDAStar());
        break;
    }
    abSearch->setNodesLimit(m_nodesLimit);

    map<int,AbStatistics>::iterator p = m_AbStatistics.find(clusterSize);
    map<string,StatisticsCollection>::iterator q = p->second.find("pp");
    map<string,StatisticsCollection>::iterator r = p->second.find("st");
    map<string,StatisticsCollection>::iterator s = p->second.find("tg");

    cerr << " AB...";
    cerr << " pp...";
    AbsWizard wizard(tiling, clusterSize, MIDDLE_ENTRANCE);
    wizard.abstractMaze(q->second);
    AbsTiling& absTiling = wizard.getAbsTiling();
    cerr << " st...";
    int absStart = absTiling.insertNode2AbstractGraph(start, 
                                                      start/tiling.getWidth(),
                                                      start%tiling.getWidth(),
                                                      r->second);
    int absTarget = absTiling.insertNode2AbstractGraph(target,
                                                       target/tiling.getWidth(),
                                                       target%tiling.getWidth(),
                                                       s->second);
    cerr << " abs...";
    abSearch->findPath(wizard.getAbsTiling(), absStart, absTarget);
    cerr << " done\n";
    const vector<int>& path2 = abSearch->getPath();
    cerr << "abstract path: ";
    for (unsigned int i = 0; i < path2.size(); i++)
    {
        cerr << path2[i] << " ";
    }
    cerr << "\n";
    vector<int> result;
    wizard.getAbsTiling().absPath2llPath(path2, result, wizard.getTiling().getWidth());
    vector<char> llVisitedNodes;
    wizard.getAbsTiling().convertVisitedNodes(abSearch->getVisitedNodes(), 
                                              llVisitedNodes, tiling.getNumberNodes());
    // path smoothing
    SmoothWizard smooth(tiling, result);
    smooth.smoothPath();
    if (print && m_contor % 5 == 0)
    {
        tiling.printPathAndLabels(cout, smooth.getSmoothPath(), llVisitedNodes);
        //        tiling.printFormatted(cout, result);
        cout << "\n";
    }
    abLength = tiling.getPathCost(smooth.getSmoothPath());
    if (abLength < llLength)
    {
        cout << abLength << " " << llLength << "\n";
        exit(0);
    }
    // accumulate statistics
    //    getAbSubStats(abStats, "sm").add(smooth.getStatistics());
    addStatistics("sm", clusterSize, smooth.getStatistics());
    const StatisticsCollection& searchAbStatistics = abSearch->getStatistics();
    //    getAbSubStats(abStats, "ab").add(searchAbStatistics);
    addStatistics("ab", clusterSize, searchAbStatistics);
}
//-----------------------------------------------------------------------------
