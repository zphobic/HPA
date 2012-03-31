//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#ifndef PATHFIND_EXPERIMENT_H
#define PATHFIND_EXPERIMENT_H

#include "pathfind.h"
#include "util.h"
#include "abswizard.h"
#include <sstream>

//-----------------------------------------------------------------------------

namespace PathFind
{
    typedef map<string, StatisticsCollection> AbStatistics;

    typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

    static const int REFINEMENT_LEVELS = 40;

    class Experiment
    {

    public:
        Experiment(int nrRuns, long long int nodesLimit, int rows, int columns,
                   float obstaclePercentage, bool ll, bool ab, bool cl,
                   int clusterSize, int level, AbsWizard::EntranceStyle entrStyle,
                   SearchAlgorithm searchAlgorithm, Tiling::Type type);

        void runExperiment();
        void runExperiment(int start, int target, char *fileName);
        void runExperiment(string fileName);
        void runClusteringExperiment();
        void runStorageExperiment(string fileName);
        void setupExperiment();

    private:
        void printHeader(SearchAlgorithm searchAlgorithm, Tiling::Type type);

        void printStatistics(ostream& o);

        void printAbStatistics(ostream& o, int k);

        void printClusteringStatistics();

        void accumulateStatistics(const string& name, StatisticsCollection stats);

        AbStatistics& getAbStats(const int clusterSize);

        StatisticsCollection& getAbSubStats(const string &name);

        void runLlSearch(Tiling &tiling, 
                         int start, int target, 
                         bool print);

        void runAbSearch(Tiling &tiling, AbsWizard& wizard, int level, bool print);

        void abstractMaze(Tiling &tiling, AbsWizard& wizard);

        void insertSTAL(AbsWizard& wizard, int start, int target);

        void removeSTAL(AbsWizard& wizard);

        StatisticsCollection createHeurStatistics();

        void printStorageStatistics(ostream& o);


//-----------------------------------------------------------------------------

    private:
        int m_absStart, m_absTarget;
        int m_nrRuns;
        long long int m_nodesLimit;
        int m_rows;
        int m_columns;
        float m_obstaclePercentage;
        bool m_ll;
        bool m_ab;
        bool m_bClStats;
        int m_contor;
        int m_clusterSize;
        int m_maxLevel;
        AbsWizard::EntranceStyle m_entrStyle;
        SearchAlgorithm m_searchAlgorithm;
        Tiling::Type m_tilingType;
        StatisticsCollection m_smoothStatistics[REFINEMENT_LEVELS];
        StatisticsCollection m_storageStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_preStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_stStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_tgStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_abMainSearchStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_abInterSearchStatistics[REFINEMENT_LEVELS][MAX_LEVELS];
        StatisticsCollection m_llStatistics[REFINEMENT_LEVELS];
        StatisticsCollection m_heurDiffStatistics[REFINEMENT_LEVELS];
        StatisticsCollection m_heurReportStatistics[REFINEMENT_LEVELS];
        StatisticsCollection m_llStorageStatistics;
    };
}

//-----------------------------------------------------------------------------

#endif
