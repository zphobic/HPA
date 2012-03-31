//-----------------------------------------------------------------------------
/** @file search.h
    Search algorithms.

    $Id: searchutils.h,v 1.2 2002/11/26 23:39:05 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/searchutils.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_SEARCHUTILS_H
#define PATHFIND_SEARCHUTILS_H

#include "environment.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    class SearchUtils
    {
    public:
        bool checkPathExists(const Environment& env, int start, int target);

        void findRandomStartTarget(const Environment& env,
                                   int& start, int &target);

    private:
        int m_target;

        const Environment* m_env;

        vector<bool> m_mark;

        vector<vector<Environment::Successor> > m_successorStack;

        bool searchPathExists(int node, int depth);
    };
}

//-----------------------------------------------------------------------------

#endif
