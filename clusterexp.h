//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#ifndef PATHFIND_EXPERIMENT_H
#define PATHFIND_EXPERIMENT_H

#include "pathfind.h"
#include "util.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    typedef map<string, StatisticsCollection> AbStatistics;

    typedef enum {A_STAR, IDA_STAR} SearchAlgorithm;

    class Experiment
    {

    public:
        Experiment(int nrRuns, long long int nodesLimit, int rows, int columns,
                   float obstaclePercentage, bool ll, bool ab,
                   SearchAlgorithm searchAlgorithm, Tiling::Type type);

        void runExperiment();
        void runClusteringExperiment();
        void setupExperiment();

    private:
        void addStats(AbStatistics &statistics);

        void addClusterSizeStats(int clusterSize);

        void printHeader(SearchAlgorithm searchAlgorithm, Tiling::Type type);

        void printStatistics();

        void printAbStatistics(int clusterSize);

        void printClusteringStatistics();

        void addStatistics(const string& name, const int clusterSize, StatisticsCollection stats);

        AbStatistics& getAbStats(const int clusterSize);

        StatisticsCollection& getAbSubStats(AbStatistics& abStats, const string& name);

        StatisticsCollection& getAbSubStats(const string &name, const int clusterSize);

        void runLlSearch(Tiling &tiling, 
                         int start, int target, 
                         bool print);

        void runAbSearch(Tiling &tiling, 
                         int start, int target, 
                         bool print, int clusterSize);

//-----------------------------------------------------------------------------

    private:
        int m_nrRuns;
        long long int m_nodesLimit;
        int m_rows;
        int m_columns;
        float m_obstaclePercentage;
        bool m_ll;
        bool m_ab;
        int m_contor;
        SearchAlgorithm m_searchAlgorithm;
        Tiling::Type m_tilingType;
        map<int, AbStatistics> m_AbStatistics;
        StatisticsCollection m_llStatistics;
    };

}

//-----------------------------------------------------------------------------

#endif
