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

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
        cerr << argv[i] << " ";
    cerr << "\n";
    int readFromFile = atoi(argv[1]);
    int nrRuns = atoi(argv[2]);
    int clSize = atoi(argv[3]);
    int maxLevel = atoi(argv[4]);
    int llSearch = atoi(argv[5]);
    if (readFromFile == 0)
    {
        int rows = atoi(argv[6]);
        int cols = atoi(argv[7]);
        float obstacle = atof(argv[8]);
        try
        {
            //srand(time(0));
            Experiment experiment(nrRuns, 10000000L, rows, cols,
                                  obstacle, (bool)llSearch, true, true,
                                  clSize, maxLevel, AbsWizard::END_ENTRANCE,
                                  A_STAR, Tiling::OCTILE);
            experiment.setupExperiment();
            experiment.runExperiment();
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << '\n';
            return -1;
        }
    }
    else if (readFromFile == 1)
    {
        int start = atoi(argv[6]);
        int target = atoi(argv[7]);
        try
        {
            //srand(time(0));
            Experiment experiment(nrRuns, 10000000L, 0, 0,
                                  0, (bool)llSearch, true, true,
                                  clSize, maxLevel, AbsWizard::END_ENTRANCE,
                                  A_STAR, Tiling::OCTILE);
            experiment.setupExperiment();
            experiment.runExperiment(start, target, argv[8]);
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << '\n';
            return -1;
        }
    }
    else if (readFromFile == 2)
    {
        try
        {
            //srand(time(0));
            Experiment experiment(nrRuns, 10000000L, 0, 0,
                                  0, (bool)llSearch, true, true,
                                  clSize, maxLevel, AbsWizard::END_ENTRANCE,
                                  A_STAR, Tiling::OCTILE);
            experiment.setupExperiment();
            experiment.runExperiment(argv[6]);
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << '\n';
            return -1;
        }
    }
    else if (readFromFile == 3)
    {
        try
        {
            //srand(time(0));
            Experiment experiment(nrRuns, 10000000L, 0, 0,
                                  0, (bool)llSearch, true, true,
                                  clSize, maxLevel, AbsWizard::END_ENTRANCE,
                                  A_STAR, Tiling::OCTILE);
            experiment.setupExperiment();
            experiment.runStorageExperiment(argv[6]);
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << '\n';
            return -1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
