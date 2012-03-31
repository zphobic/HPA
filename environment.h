//-----------------------------------------------------------------------------
/** @file environment.h
    Search environment.

    $Id: environment.h,v 1.5 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/environment.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ENVIRONMENT_H
#define PATHFIND_ENVIRONMENT_H

#include <exception>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    /** Interface to search environment. */
    class Environment
    {
    public:
        class Successor
        {
        public:
            Successor();
            
            Successor(int target, int cost)
                : m_target(target), m_cost(cost)
            { }
            
            int m_target;
            
            int m_cost;
        };
        
        virtual ~Environment();
        
        virtual int getHeuristic(int start, int target) const = 0;
        
        virtual int getMaxCost() const = 0;
        
        virtual int getMinCost() const = 0;
        
        virtual int getNumberNodes() const = 0;
        
        /** Generate successor nodes for the search.
            @param lastNodeId
            Can be used to prune nodes,
            (is set to NO_NODE in Search::checkPathExists).
        */
        virtual void getSuccessors(int nodeId, int lastNodeId,
                                   vector<Successor>& result) const = 0;
        
        virtual bool isValidNodeId(int nodeId) const = 0;
    };        
}

#endif
