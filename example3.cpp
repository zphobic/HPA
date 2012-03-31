//-----------------------------------------------------------------------------
/** @file example.cpp
    Program using the pathfinding library to test different pathfinding alg/top.

    $Id: example.cpp,v 1.25 2003/04/01 23:00:47 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/example.cpp,v $
*/
//-----------------------------------------------------------------------------

#include <memory>
#include "pathfind.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

static const int s_numberRuns = 400;
static const long long int s_nodesLimit = 100000000L;

//-----------------------------------------------------------------------------

typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

//-----------------------------------------------------------------------------

void printHeader(SearchAlgorithm searchAlgorithm, Tiling::Type type, const char* name)
{
    cout << "==============================================================\n";
    switch (searchAlgorithm)
    {
    case A_STAR:
        cout << "A_STAR";
        break;
    case IDA_STAR:
        cout << "IDA_STAR";
        break;
    }
    cout << "  ";
    switch (type)
    {
    case Tiling::TILE:
        cout << "TILE";
        break;
    case Tiling::OCTILE:
        cout << "OCTILE";
        break;
    case Tiling::OCTILE_UNICOST:
        cout << "OCTILE_UNICOST";
        break;
    case Tiling::HEX:
        cout << "HEX";
        break;
    }
    cout << "  ";
    cout << name;
    cout << '\n';
    cout << "==============================================================\n";
}

StatisticsCollection runExperiment(SearchAlgorithm searchAlgorithm, 
                                   const Tiling& tiling,
                                   const int fileIndex, const char *fileName)
{
    printHeader(searchAlgorithm, tiling.getType(), fileName);
    cout << tiling.getWidth() << "x" << tiling.getHeight() << '\n';

    auto_ptr<Search> search;
    switch (searchAlgorithm)
    {
    case A_STAR:
        search.reset(new AStar(false));
        break;
    case IDA_STAR:
        search.reset(new IDAStar());
        break;
    }
    search->setNodesLimit(s_nodesLimit);
    SearchUtils searchUtils;
    StatisticsCollection statistics = search->createStatistics();
    for (int runIndex = 0; runIndex < s_numberRuns; ++runIndex)
    {
        // Choose random start and target
        int start, target;
        searchUtils.findRandomStartTarget(tiling, start, target);
        //cout << "Run " << runIndex << ":\n";
        //tiling.printFormatted(cout, start, target);
        //cout << '\n';
        
        // Do the search
        search->findPath(tiling, start, target);
        //cout << '\n';
        //const vector<int>& path = search->getPath();
        //tiling.printFormatted(cout, path);
        //cout << '\n';
        //tiling.printPathAndLabels(cout, path, search->getVisitedNodes());
        //cout << '\n';
        
        // Accumulate statistics
        const StatisticsCollection& searchStatistics = search->getStatistics();
        //searchStatistics.print(cout);
        //cout << '\n';

        cout << "  " << fileIndex;
        cout << "  " << runIndex;
        cout << "  " << start; 
        cout << "  " << target;
        //     << " " << tiling.getRow(start) << " " << tiling.getCol(start);
        //     << " " << tiling.getRow(target) << " " << tiling.getCol(target);
        cout << "  " << tiling.getHeuristic(start,target);
        cout << "  " << searchStatistics.get("path_cost").getMean();
        cout << "  " << search->getPath().size();
        cout << "  " << searchStatistics.get("nodes_visited").getMean();
        cout << "  " << searchStatistics.get("cpu_time").getMean();
        cout << '\n';

        statistics.add(searchStatistics);
        //statistics.print(cout);
        //cout << '\n';
    }
    statistics.print(cout);

    return statistics;
}

int main(int argc, char *argv[])
{
    try
    {
        //srand(time(0));
        StatisticsCollection *statisticsSum = 0;
        for ( int i=1 ; i<argc ; i++ ) {
            ifstream file(argv[i]);
            LineReader reader(file);
            Tiling tiling(reader);
            StatisticsCollection statistics =
                runExperiment(IDA_STAR, tiling, i, argv[i]);
            if ( statisticsSum == 0 )
                statisticsSum = new StatisticsCollection(statistics);
            else
                statisticsSum->add(statistics);
        }
        cout << '\n';
        statisticsSum->print(cout);
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << '\n';
        return -1;
    }
    return 0;
}

//-----------------------------------------------------------------------------
