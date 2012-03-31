//-----------------------------------------------------------------------------
// $Id: statistics.cpp,v 1.4 2002/12/09 22:10:19 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/statistics.cpp,v $
//-----------------------------------------------------------------------------

#include "statistics.h"

#include <sstream>
#include <time.h>
#include "error.h"

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

Statistics::Statistics()
{
    clear();
}

void Statistics::add(double value)
{
    m_count += 1.0;
    m_sum += value;
    m_sumSq += (value * value);
}

void Statistics::add(const Statistics& statistics)
{
    m_count += statistics.m_count;
    m_sum += statistics.m_sum;
    m_sumSq += statistics.m_sumSq;
}

void Statistics::clear()
{
    m_count = 0;
    m_sum = 0;
    m_sumSq = 0;
}

double Statistics::getVariance() const
{
    double mean = getMean();
    return m_sumSq / m_count - mean * mean;
}

void Statistics::print(ostream& o) const
{
    // AdiB - print more info
    o << m_sum << ' ' << getCount() << ' ' << getMean() << ' ' << getDeviation();
}

//-----------------------------------------------------------------------------

void StatisticsCollection::add(const StatisticsCollection& collection)
{
    if (m_map.size() != collection.m_map.size())
        throw Error("Trying to add incompatible statistics collections");
    map<string,Statistics>::iterator p;
    for (p = m_map.begin(); p != m_map.end(); ++p)
    {
        map<string,Statistics>::const_iterator k
            = collection.m_map.find(p->first);
        if (k == collection.m_map.end())
            throw Error("Trying to add incompatible statistics collections");
        p->second.add(k->second);
    }
}

void StatisticsCollection::clear()
{
    map<string,Statistics>::iterator p;
    for (p = m_map.begin(); p != m_map.end(); ++p)
        p->second.clear();
}

void StatisticsCollection::create(const string& name)
{
    m_map[name] = Statistics();
}

Statistics& StatisticsCollection::get(const string& name)
{
    map<string,Statistics>::iterator p = m_map.find(name);
    if (p == m_map.end())
    {
        ostringstream o;
        o << "Unknown statistics name " << name << '.';
        throw Error(o.str());
    }
    return p->second;
}

void StatisticsCollection::print(ostream& o) const
{
    map<string,Statistics>::const_iterator p;
    for (p = m_map.begin(); p != m_map.end(); ++p)
    {
        string name = p->first;
        const Statistics& statistics = p->second;
        o << name << ' ';
        statistics.print(o);
        o << '\n';
    }
}

//-----------------------------------------------------------------------------

#if 0

void check(bool expression, int line)
{
    if (! expression)
        cerr << "Error line " << line << '\n';
}

int main()
{
    Statistics statistics;
    statistics.add(1.0);
    statistics.add(2.0);
    statistics.add(3.0);
    check(fabs(statistics.getMean() - 2.000) < 0.001, __LINE__);
    check(fabs(statistics.getDeviation() - 0.816) < 0.001, __LINE__);
    
    Statistics statistics2;
    statistics2.add(-1.0);
    statistics2.add(2.5);
    statistics2.add(2.5);
    statistics2.add(2.7);
    check(fabs(statistics2.getMean() - 1.675) < 0.001, __LINE__);
    check(fabs(statistics2.getDeviation() - 1.547) < 0.001, __LINE__);
    
    statistics.add(statistics2);
    check(fabs(statistics.getMean() - 1.814) < 0.001, __LINE__);
    check(fabs(statistics.getDeviation() - 1.296) < 0.001, __LINE__);
    return 0;
}

#endif

//-----------------------------------------------------------------------------
