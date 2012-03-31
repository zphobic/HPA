//-----------------------------------------------------------------------------
/** @file example.cpp
    Example program using the pathfinding library.

    $Id: example.cpp,v 1.24 2002/12/09 22:10:19 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/example.cpp,v $
*/
//-----------------------------------------------------------------------------

#include <memory>
#include "pathfind.h"
#include "abswizard.h"
#include "smoothwizard.h"
#include "experiment.h"

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

static const int s_numberRuns = 100;
// static const int s_columns = 80;
// static const int s_rows = 60;
// static const float s_obstaclePercentage = 0.3;
static const long long int s_nodesLimit = 100000000L;

//-----------------------------------------------------------------------------

typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

//-----------------------------------------------------------------------------

void printHeader(SearchAlgorithm searchAlgorithm, Tiling::Type type)
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

void runExperiment(SearchAlgorithm searchAlgorithm, Tiling::Type type, 
                   int s_rows, int s_columns,  float s_obstaclePercentage)
{
    printHeader(searchAlgorithm, type);
    SearchUtils searchUtils;
    auto_ptr<Search> llSearch, abSearch, fakeSearch;
    SmoothWizard fakeSmooth;
    switch (searchAlgorithm)
    {
    case A_STAR:
        llSearch.reset(new AStar());
        abSearch.reset(new AStar());
        fakeSearch.reset(new AStar());
        break;
    case IDA_STAR:
        llSearch.reset(new IDAStar());
        abSearch.reset(new IDAStar());
        fakeSearch.reset(new AStar());
        break;
    }
    llSearch->setNodesLimit(s_nodesLimit);
    abSearch->setNodesLimit(s_nodesLimit);
    StatisticsCollection llStatistics = llSearch->createStatistics();
    StatisticsCollection abStatistics = abSearch->createStatistics();
    StatisticsCollection ppStatistics = fakeSearch->createStatistics();
    StatisticsCollection stStatistics = fakeSearch->createStatistics();
    StatisticsCollection tgStatistics = fakeSearch->createStatistics();
    StatisticsCollection smStatistics = fakeSmooth.createStatistics();

    cout << "Rows: " << s_rows << "; Columns: " << s_columns << "; Obstacle%: " << s_obstaclePercentage;
    cout << "; Number of runs: " << s_numberRuns << "\n";

    bool solveLL = true, solveAB = true;
    for (int runIndex = 0; runIndex < s_numberRuns; ++runIndex)
    {
        // Create a tiling search environment
        Tiling tiling(type, s_rows, s_columns);
        tiling.setObstacles(s_obstaclePercentage);
        
        // Choose random start and target
        int start, target;
        searchUtils.findRandomStartTarget(tiling, start, target);
        //        if (s_rows != 50 || s_obstaclePercentage <= .34)
        //              continue;
        if (runIndex%5 == 0)
        {
            cout << "Run " << runIndex << ":\n";
        }

        // low-level search
        cerr << "\n" << runIndex;
        cerr << " (" << s_rows << "," << s_columns << "," << s_obstaclePercentage << ")\n";
        tiling.printFormatted(cerr, start, target);

        if (solveLL)
        {
            cerr << "LL...";
            llSearch->findPath(tiling, start, target);
            const vector<int>& path1 = llSearch->getPath();
            if (runIndex%5 == 0)
            {
                tiling.printPathAndLabels(cout, path1, llSearch->getVisitedNodes());
                cout << "\n";
            }
        }
        // abstract search
        if (solveAB)
        {
            cerr << " AB...";
            cerr << " pp...";
            AbsWizard wizard(tiling, 10);
            wizard.abstractMaze(ppStatistics);
            AbsTiling& absTiling = wizard.getAbsTiling();
            cerr << " st...";
            int absStart = absTiling.insertNode2AbstractGraph(start/s_columns,
                                                              start%s_columns,
                                                              stStatistics);
            int absTarget = absTiling.insertNode2AbstractGraph(target/s_columns,
                                                               target%s_columns,
                                                               tgStatistics);
            cerr << " abs...";
            abSearch->findPath(wizard.getAbsTiling(), absStart, absTarget);
            cerr << " done\n";
            const vector<int>& path2 = abSearch->getPath();
            //        absTiling.printPathAndLabels(cout, path2, abSearch->getVisitedNodes());
            vector<int> result;
            wizard.getAbsTiling().absPath2llPath(path2, result, wizard.getTiling().getWidth());
            // path smoothing
            SmoothWizard smooth(tiling, result);
            smooth.smoothPath();
            if (runIndex%5 == 0)
            {
                tiling.printFormatted(cout, smooth.getSmoothPath());
                cout << "\n";
            }
            // accumulate statistics
            smStatistics.add(smooth.getStatistics());
            const vector<int>& path1 = llSearch->getPath();
            if (path1.size() > smooth.getSmoothPath().size())
            {
                cout << "BUG ALERT!!!!!!!!!\n";
                tiling.printPathAndLabels(cout, path1, llSearch->getVisitedNodes());
                cout << "\n";
                tiling.printFormatted(cout, smooth.getSmoothPath());
                cout << "\n";
                assert(false);
            }
        }

        // Accumulate statistics
        if (solveLL)
        {
            const StatisticsCollection& searchLlStatistics = llSearch->getStatistics();
            llStatistics.add(searchLlStatistics);
        }
        if (solveAB)
        {
            const StatisticsCollection& searchAbStatistics = abSearch->getStatistics();
            abStatistics.add(searchAbStatistics);
        }
    }
    cout << "SEARCH SUMMARY AND STATISTICS:\n";
    cout << "Rows: " << s_rows << "; Columns: " << s_columns << "; Obstacle%: " << s_obstaclePercentage;
    cout << "; Number of runs: " << s_numberRuns << "\n";
    cout << "LL\tLow-level search:\n";
    llStatistics.print(cout);
    cout << "AB\tAbstract search:\n";
    abStatistics.print(cout);
    cout << "PP\tPre-processing search:\n";
    ppStatistics.print(cout);
    cout << "ST\tSearch for abstract start:\n";
    stStatistics.print(cout);
    cout << "TG\tSearch for abstract target:\n";
    tgStatistics.print(cout);
    cout << "SM\t Post-processing for path smoothing:\n";
    smStatistics.print(cout);
    cout << "HF\t Average path in human readable format:\n";
    cout << "abstract: " << (abStatistics.get("path_cost").getMean() - COST_ONE)/COST_ONE << "; ";
    cout << "abstract+smoothing: " << smStatistics.get("path_cost").getMean()/COST_ONE << "; ";
    cout << "optimal: " << llStatistics.get("path_cost").getMean()/COST_ONE << "; ";
//        " << abStatistics.get("path_cost").getMean() - 
//                      COST_ONE - 
//        smStatistics.get("improvement").getMean()*
//        smStatistics.get("improvement").getCount()/
//        abStatistics.get("path_cost").getCount();
    cout << "\n\n";
}

void runLlSearch(SearchAlgorithm searchAlgorithm, Tiling &tiling, 
                 int start, int target, StatisticsCollection &llStatistics, 
                 bool print)
{
    auto_ptr<Search> llSearch;
    switch (searchAlgorithm)
    {
    case A_STAR:
        llSearch.reset(new AStar());
        break;
    case IDA_STAR:
        llSearch.reset(new IDAStar());
        break;
    }
    llSearch->setNodesLimit(s_nodesLimit);
    llSearch->findPath(tiling, start, target);
    const vector<int>& path1 = llSearch->getPath();
    if (print)
    {
        tiling.printPathAndLabels(cout, path1, llSearch->getVisitedNodes());
        cout << "\n";
    }
    const StatisticsCollection& searchLlStatistics = llSearch->getStatistics();
    llStatistics.add(searchLlStatistics);
}

void runAbSearch(SearchAlgorithm searchAlgorithm, Tiling &tiling, 
                 int start, int target, 
                 StatisticsCollection &abStatistics, 
                 StatisticsCollection &ppStatistics,
                 StatisticsCollection &stStatistics,
                 StatisticsCollection &tgStatistics,
                 StatisticsCollection &smStatistics,
                 bool print, int clusterSize)
{
    auto_ptr<Search> abSearch;
    switch (searchAlgorithm)
    {
    case A_STAR:
        abSearch.reset(new AStar());
        break;
    case IDA_STAR:
        abSearch.reset(new IDAStar());
        break;
    }
    abSearch->setNodesLimit(s_nodesLimit);

    cerr << " AB...";
    cerr << " pp...";
    AbsWizard wizard(tiling, clusterSize);
    wizard.abstractMaze(ppStatistics);
    AbsTiling& absTiling = wizard.getAbsTiling();
    cerr << " st...";
    int absStart = absTiling.insertNode2AbstractGraph(start/tiling.getWidth(), 
                                                      start%tiling.getWidth(), 
                                                      stStatistics);
    int absTarget = absTiling.insertNode2AbstractGraph(target/tiling.getWidth(), 
                                                       target%tiling.getWidth(), 
                                                       tgStatistics);
    cerr << " abs...";
    abSearch->findPath(wizard.getAbsTiling(), absStart, absTarget);
    cerr << " done\n";
    const vector<int>& path2 = abSearch->getPath();
    vector<int> result;
    wizard.getAbsTiling().absPath2llPath(path2, result, wizard.getTiling().getWidth());
    // path smoothing
    SmoothWizard smooth(tiling, result);
    smooth.smoothPath();
    if (print)
    {
        tiling.printFormatted(cout, smooth.getSmoothPath());
        cout << "\n";
    }
    // accumulate statistics
    smStatistics.add(smooth.getStatistics());
    const StatisticsCollection& searchAbStatistics = abSearch->getStatistics();
    abStatistics.add(searchAbStatistics);
}

void runExperiment2()
{
    auto_ptr<Search> fakeSearch;
    fakeSearch.reset(new AStar());
    SmoothWizard fakeSmooth;
    StatisticsCollection ppStatistics = fakeSearch->createStatistics();
    StatisticsCollection stStatistics = fakeSearch->createStatistics();
    StatisticsCollection tgStatistics = fakeSearch->createStatistics();
    StatisticsCollection smStatistics = fakeSmooth.createStatistics();
    map<int, StatisticsCollection> abStats;
    map<int, StatisticsCollection> stStats;
    map<int, StatisticsCollection> tgStats;
    map<int, StatisticsCollection> smStats;

    // init statistics
    stStats[10] = fakeSearch->createStatistics();

}

int main()
{
    for (int rows = 20; rows <= 100; rows+= 10)
    {
        for (float obstacle = .2; obstacle <= .41; obstacle += .05)
        {
            try
            {
                //srand(time(0));
                //runExperiment(A_STAR, Tiling::TILE, rows, rows, obstacle);
                //runExperiment(IDA_STAR, Tiling::TILE);
                Experiment experiment(100, 10000000, rows, rows,
                                      obstacle, true, true,
                                      IDA_STAR, Tiling::TILE);
                experiment.runExperiment();
            }
            catch (const exception& e)
            {
                cerr << "Error: " << e.what() << '\n';
                return -1;
            }
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
