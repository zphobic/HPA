//-----------------------------------------------------------------------------
/** @file statistics.h
    Search statistics.

    $Id: statistics.h,v 1.4 2002/12/09 22:10:19 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/statistics.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_STATISTICS_H
#define PATHFIND_STATISTICS_H

#include <iostream>
#include <math.h>
#include <map>
#include <string>

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    /** Keeps track of the mean and variance of a variable. */
    class Statistics
    {
    public:
        Statistics();

        void add(double value);

        void add(const Statistics& statistics);

        void clear();

        double getCount() const
        {
            return m_count;
        }

        double getMean() const
        {
            return m_sum / m_count;
        }

        double getDeviation() const
        {
            return sqrt(getVariance());
        }

        double getVariance() const;

        void print(ostream& o) const;

    private:
        double m_count;

        double m_sum;

        double m_sumSq;
    };

    /** Set of statistics variables. */
    class StatisticsCollection
    {
    public:
        /** Add the statistics of another collection.
            The collections must contain the same entries.
        */
        void add(const StatisticsCollection& collection);

        void clear();

        void create(const string& name);

        Statistics& get(const string& name);

        void print(ostream& o) const;

    private:
        map<string, Statistics> m_map;
    };
}

//-----------------------------------------------------------------------------

#endif
