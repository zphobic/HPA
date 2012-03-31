//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#include <memory>
#include <assert.h>
#include "experiment.h"

#include <ctype.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "smoothwizard.h"
#include "abswizard.h"
#include "abstiling.h"
#include "util.h"
#include <stdio.h>

using namespace std;
using namespace PathFind;

int llLength, abLength, heuristic, heurStatsIndex;
float heurReport;

//-----------------------------------------------------------------------------

typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

//-----------------------------------------------------------------------------

Experiment::Experiment(int nrRuns, long long int nodesLimit, int rows, int columns,
                       float obstaclePercentage, bool ll, bool ab, bool cl,
                       int clusterSize, int level, AbsWizard::EntranceStyle entrStyle,
                       SearchAlgorithm searchAlgorithm, Tiling::Type type)
    :m_nrRuns(nrRuns),
     m_nodesLimit(nodesLimit),
     m_rows(rows),
     m_columns(columns),
     m_obstaclePercentage(obstaclePercentage),
     m_ll(ll),
     m_ab(ab),
     m_bClStats(cl),
     m_clusterSize(clusterSize),
     m_maxLevel(level),
     m_entrStyle(entrStyle),
     m_searchAlgorithm(searchAlgorithm),
     m_tilingType(type)
{
}

void Experiment::runExperiment()
{
    printHeader(m_searchAlgorithm, m_tilingType);
    SearchUtils searchUtils;

    for (int k = 0; k < m_nrRuns; k++)
    {
        cerr << "Create tiling...\n";
        Tiling tiling(m_tilingType, m_rows, m_columns);
        cerr << "Set obstacles...\n";
        tiling.setObstacles(m_obstaclePercentage);

        AbsWizard wizard(tiling, m_clusterSize, m_maxLevel, m_entrStyle);
        if (m_ab || m_bClStats)
        {
            abstractMaze(tiling, wizard);
        }

        for (int i = 0; i < m_nrRuns; i++)
        {
            m_contor = i;
            int start, target;
            //        while (true)
            {
                searchUtils.findRandomStartTarget(tiling, start, target);
                heuristic = tiling.getHeuristic(start, target);
                //  if (heuristic >= 30000)
                //  break;
            }
            cerr << "\n" << m_contor;
            cerr << " (" << m_rows << "," << m_columns << "," << m_obstaclePercentage << ")\n";
            //        tiling.printFormatted(cerr, start, target);
            if (m_ll)
            {
                cerr << "Low-level search...";
                runLlSearch(tiling, start, target, false);
                cerr << "\n";
            }
            if (m_ab)
            {
                cerr << "Hierarchical ";
                wizard.getAbsTiling().clearStatistics();
                insertSTAL(wizard, start, target);
                for (int level = 1; level <=m_maxLevel; level++)
                {
                    cerr << " " << level << " ";
                    runAbSearch(tiling, wizard, level, false);
                }
                removeSTAL(wizard);
                cerr << "\n";
            }
        }
        char output[50];
        int obst = (int)(m_obstaclePercentage*100);
        sprintf(output, "Rnd/Results/newrnd%d_%d.txt", obst, k);
        ofstream file(output);
        printStatistics(file);
    }
}

void Experiment::runExperiment(string fileName)
{
    printHeader(m_searchAlgorithm, m_tilingType);
    ifstream file(fileName.c_str());
    LineReader reader(file);
    Tiling tiling(reader);
    SearchUtils searchUtils;
    m_columns = tiling.getWidth();
    m_rows = tiling.getHeight();
    string pathfileName;
    pathfileName = fileName + ".paths";
    ifstream pathfile(pathfileName.c_str());
    if ( !pathfile ) {
        cerr << "Could not open pathfile " << pathfileName << endl;
        exit(1);
    }
    AbsWizard wizard(tiling, m_clusterSize, m_maxLevel, m_entrStyle);
    if (m_ab || m_bClStats)
    {
        abstractMaze(tiling, wizard);
    }
    vector< pair<int,int> > paths(100);
    int idx, start, target;
    while ( pathfile >> idx >> start >> target )
    {
        if (idx >= m_nrRuns)
            break;
        wizard.getAbsTiling().clearStatistics();
        heuristic = tiling.getHeuristic(start, target);
        cerr << "\n" << idx;
        cerr << " (" << fileName << ")\n";
        if (m_ll)
        {
            cerr << "Low-level search...";
            runLlSearch(tiling, start, target, false);
            cerr << "\n";
        }
        if (m_ab)
        {
            cerr << "Abstract search...";
            insertSTAL(wizard, start, target);
            for (int level = m_maxLevel; level >= 1; level--)
            {
                cerr << " " << level << " ";
                runAbSearch(tiling, wizard, level, false);
            }
            removeSTAL(wizard);
        }
        //        m_nrRuns = idx + 1;
    }
    printStatistics(cout);
}

void Experiment::runStorageExperiment(string fileName)
{
    printHeader(m_searchAlgorithm, m_tilingType);
    ifstream file(fileName.c_str());
    LineReader reader(file);
    Tiling tiling(reader);
    SearchUtils searchUtils;
    m_columns = tiling.getWidth();
    m_rows = tiling.getHeight();
    string pathfileName;
    AbsWizard wizard(tiling, m_clusterSize, m_maxLevel, m_entrStyle);
    if (m_ab || m_bClStats)
    {
        abstractMaze(tiling, wizard);
    }
    const StatisticsCollection& storageLlStatistics = tiling.getStorageStatistics();
    m_llStorageStatistics.add(storageLlStatistics);
    printStorageStatistics(cout);
}


void Experiment::runExperiment(int start, int target, char *fileName)
{
//     printHeader(m_searchAlgorithm, m_tilingType);
//     ifstream file(fileName);
//     LineReader reader(file);
//     Tiling tiling(reader);
//     SearchUtils searchUtils;
//     m_columns = tiling.getWidth();
//     m_rows = tiling.getHeight();
//     for (int i = 0; i < m_nrRuns; i++)
//     {
//         m_contor = i;
//         if (start == -1 || target == -1)
//         {
//                 searchUtils.findRandomStartTarget(tiling, start, target);
//         }
//         // start = 193*m_columns + 168;
//         // target = 192*m_columns + 190;
//         tiling.printFormatted(cout, start, target);
//         heuristic = tiling.getHeuristic(start, target);
//         cerr << "\n" << m_contor;
//         cerr << " (" << fileName << ")\n";
//         if (m_ll)
//             runLlSearch(tiling, start, target, false);
//         AbsWizard wizard(tiling, m_clusterSize, m_maxLevel, m_entrStyle);
//         if (m_ab || m_bClStats)
//         {
//             abstractMaze(tiling, wizard);
//         }
//         if (m_ab)
//         {
//             cerr << "Running abstract search...";
//             runAbSearch(tiling, wizard, start, target, m_maxLevel, false);
//         }
//         cout << i << ": " << heuristic << ", " << abLength << "\n";
//     }    
//     printStatistics();
}

StatisticsCollection Experiment::createHeurStatistics()
{
    StatisticsCollection collection;
    collection.create("report");
    return collection;
}


void Experiment::setupExperiment()
{
    if (m_ll)
    {
        Tiling fakeTiling(Tiling::OCTILE, 1, 1);
        auto_ptr<Search> fakeSearch;
        fakeSearch.reset(new AStar(false));
        for (int k = 0; k < REFINEMENT_LEVELS; k++)
            m_llStatistics[k] = fakeSearch->createStatistics();
        m_llStorageStatistics = fakeTiling.createStorageStatistics();
    }
    if (m_ab || m_bClStats)
    {
        auto_ptr<Search> fakeSearch, abFakeSearch;
        fakeSearch.reset(new AStar(false));
        switch (m_searchAlgorithm)
        {
        case A_STAR:
            abFakeSearch.reset(new AStar(false));
            break;
        case IDA_STAR:
            abFakeSearch.reset(new IDAStar());
            break;
        }
        SmoothWizard fakeSmooth;
        for (int k = 0; k < REFINEMENT_LEVELS; k++)
        {
            m_smoothStatistics[k] = fakeSmooth.createStatistics();
            for (int i = 0; i <= m_maxLevel; i++)
            {
                m_preStatistics[k][i] = fakeSearch->createStatistics();
                m_tgStatistics[k][i] = fakeSearch->createStatistics();
                m_stStatistics[k][i] = fakeSearch->createStatistics();
                m_abMainSearchStatistics[k][i] = fakeSearch->createStatistics();
                m_abInterSearchStatistics[k][i] = fakeSearch->createStatistics();
            }
        }
    }
    if (m_ab || m_bClStats)
    {
        AbsTiling fakeAbsTiling;
        for (int k = 0; k < REFINEMENT_LEVELS; k++)
        for (int i = 0; i <= m_maxLevel; i++)
            m_storageStatistics[k][i] = fakeAbsTiling.createStorageStatistics();
    }
    for (int k = 0; k < REFINEMENT_LEVELS; k++)
        m_heurReportStatistics[k] = createHeurStatistics();
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

void Experiment::printStatistics(ostream& o)
{
    o << "SEARCH SUMMARY AND STATISTICS:\n";
    o << "Rows: " << m_rows << "; Columns: " << m_columns << "; Obstacle%: " << m_obstaclePercentage;
    o << "; Number of runs: " << m_nrRuns << "\n";
    int k = 0;
    //    for (int k = 0; k < REFINEMENT_LEVELS; k++)
    {
        //        if (m_ll && m_heurReportStatistics[k].get("report").getCount() == 0)
        //    continue;

        m_heurReportStatistics[k].print(o);
    // low-level search
    if (m_ll)
    {
        o << "LL\tLow-level search:\n";
        m_llStatistics[k].print(o);
    }
    // abstract search
    if (m_ab)
        printAbStatistics(o, k);
    if (m_ll)
    {
        o << "optimal: " << m_llStatistics[k].get("path_cost").getMean()/COST_ONE << "; ";
        o << "\n";
    }
    else
        o << "\n";
    o << "\n";
    }
}

void Experiment::printStorageStatistics(ostream& o)
{
    // low-level search
    if (m_ll)
    {
        o << "level 0:\n";
        m_llStorageStatistics.print(o);
    }
    // abstract search
    if (m_ab)
    {
        for (int i = 1; i <= m_maxLevel; i++)
        {
          o << "level " << i << ":\n";
          m_storageStatistics[0][i].print(o);
        }
    }
    o << "\n\n";
}


void Experiment::printClusteringStatistics()
{
}

void Experiment::printAbStatistics(ostream& o, int k)
{
    o << "AB\tcluster size " << m_clusterSize << "\t hierarchic abstract levels " << m_maxLevel << "\n";
    {
    o << "ABM\tAbstract main search:\n";
    for (int i = 1; i <= m_maxLevel; i++)
    {
        o << "at level " << i << ":\n";
        m_abMainSearchStatistics[k][i].print(o);
    }
    o << "ABI\tAbstract intermediate search:\n";
    for (int i = 0; i < m_maxLevel; i++)
    {
        o << "at level " << i << ":\n";
        m_abInterSearchStatistics[k][i].print(o);
    }

    o << "PP\tPre-processing search:\n";
    for (int i = 0; i < m_maxLevel; i++)
    {
        o << "at level " << i << " (building level " << i+1 << "):\n";
        m_preStatistics[0][i].print(o);
    }
    o << "ST\tSearch for abstract start:\n";
    for (int i = 0; i < m_maxLevel; i++)
    {
        o << "at level " << i << " (building level " << i+1 << "):\n";
        m_stStatistics[k][i].print(o);
    }
    o << "GR\t Graph storage statistics:\n";
    for (int i = 1; i <= m_maxLevel; i++)
    {
        o << "level " << i << ":\n";
        m_storageStatistics[0][i].print(o);
    }
    o << "SM\t Post-processing for path smoothing:\n";
    m_smoothStatistics[k].print(o);
    o << "HF\t Average path in human readable format:\n";
    o << "abstract: " << m_abMainSearchStatistics[k][m_maxLevel].get("path_cost").getMean()/COST_ONE << "; ";
    o << "abstract+smoothing: " << m_smoothStatistics[k].get("path_cost").getMean()/COST_ONE << "; ";
    }
}

void Experiment::runLlSearch(Tiling &tiling, int start, int target, bool print)
{
    auto_ptr<Search> llSearch;
    switch (m_searchAlgorithm)
    {
    case A_STAR:
        llSearch.reset(new AStar(true));
        break;
    case IDA_STAR:
        llSearch.reset(new IDAStar());
        break;
    }
    llSearch->setNodesLimit(m_nodesLimit);
    llSearch->findPath(tiling, start, target);
    const vector<int>& path1 = llSearch->getPath();
    llLength = tiling.getPathCost(path1);
    if (print && m_contor % 10 == 0)
    {
        cout << "\n" << m_contor << ":\n";
        tiling.printPathAndLabels(cout, path1, llSearch->getVisitedNodes());
        cout << "\n";
    }
    //    heurReport = (double)llLength/heuristic;
    heurStatsIndex = 0;
    //    double rep = 1.1;
//     int aValue = 2500;
//     while (aValue < llLength)
//     {
//         aValue += 2500;
//         heurStatsIndex++;
//         if (heurStatsIndex >= REFINEMENT_LEVELS - 1) 
//             break;
//     }
    const StatisticsCollection& searchLlStatistics = llSearch->getStatistics();
    m_llStatistics[heurStatsIndex].add(searchLlStatistics);
    const StatisticsCollection& storageLlStatistics = tiling.getStorageStatistics();
    m_llStorageStatistics.add(storageLlStatistics);
    //    m_heurReportStatistics[heurStatsIndex].get("report").add(aValue);
}

void Experiment::abstractMaze(Tiling &tiling, AbsWizard &wizard)
{
    cerr << " AB...";
    cerr << " pp...";
    wizard.abstractMaze();
    const HTiling& absTiling = wizard.getAbsTiling();
    for (int i = 0; i <= m_maxLevel; i++)
    {
        m_storageStatistics[0][i].add(absTiling.getStorageStatistics(i));
        m_preStatistics[0][i].add(absTiling.getPreStatistics(i));
    }
}

void Experiment::insertSTAL(AbsWizard& wizard, int start, int target)
{
    cerr << " STAL ";
    HTiling& absTiling = wizard.getAbsTiling();
    m_absStart = absTiling.insertSTAL(start,
                                      start/m_columns,
                                      start%m_columns, 0);
    m_absTarget = absTiling.insertSTAL(target,
                                       target/m_columns,
                                       target%m_columns, 1);
    cerr << "(" << m_absStart << ", " << m_absTarget << ")...";
}

void Experiment::removeSTAL(AbsWizard& wizard)
{
    cerr << " !STAL...";
    HTiling& absTiling = wizard.getAbsTiling();
    absTiling.removeStal(m_absTarget, 1);
    absTiling.removeStal(m_absStart, 0);
    cerr << "\n";
}

void Experiment::runAbSearch(Tiling &tiling, AbsWizard& wizard, int level, bool print)
{
    auto_ptr<Search> abSearch;
    switch (m_searchAlgorithm)
    {
    case A_STAR:
        abSearch.reset(new AStar(true));
        break;
    case IDA_STAR:
        abSearch.reset(new IDAStar());
        break;
    }
    abSearch->setNodesLimit(m_nodesLimit);
    HTiling& absTiling = wizard.getAbsTiling();
    //    cerr << " searching...";
    vector<int> path2;
    absTiling.doHierarchicalSearch(m_absStart, m_absTarget, path2, level);
    //    cerr << " path conversion...";
    vector<int> result;
    absTiling.absPath2llPath2(path2, result, wizard.getTiling().getWidth());
    vector<char> llVisitedNodes;
    absTiling.convertVisitedNodes(abSearch->getVisitedNodes(),
                                  llVisitedNodes, tiling.getNumberNodes());
    // path smoothing
    //    cerr << "path smoothing...";
    SmoothWizard smooth(tiling, result);
    smooth.smoothPath();
    if (print && m_contor % 10 == 0)
    {
        tiling.printPathAndLabels(cout, smooth.getSmoothPath(), llVisitedNodes);
        cout << "\n";
    }
    abLength = tiling.getPathCost(smooth.getSmoothPath());
    //    tiling.printFormatted(cout, smooth.getSmoothPath());
    if (abLength < llLength)
    {
        cout << "Bug alert: solution shorter than optimal! (" << abLength << " < " << llLength << ")\n";
        exit(0);
    }
    // accumulate statistics
    if (level == m_maxLevel)
    {
        m_smoothStatistics[heurStatsIndex].add(smooth.getStatistics());
        m_stStatistics[heurStatsIndex][level].add(absTiling.getStStatistics(level));
    }
    m_abMainSearchStatistics[heurStatsIndex][level].add(absTiling.getAbMainSearchStatistics(level));
    m_abInterSearchStatistics[heurStatsIndex][level-1].add(absTiling.getAbInterSearchStatistics(level-1));
    m_stStatistics[heurStatsIndex][level-1].add(absTiling.getStStatistics(level-1));
}
//-----------------------------------------------------------------------------
