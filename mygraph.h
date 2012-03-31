//-----------------------------------------------------------------------------
/** @file graph.h
    Graph class.

    $Id: graph.h,v 1.8 2002/11/26 21:12:33 yngvi Exp $
    $Source: /usr/cvsroot/project_pathfind/graph.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_MYGRAPH_H
#define PATHFIND_MYGRAPH_H
#include <assert.h>
#include <iostream>

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    template<class NODEINFO, class EDGEINFO>
    class MyGraph : public Graph
    {
    public:
        class MyEdge : public Edge
        {
        public:
            MyEdge();

            MyEdge(int targetNodeId, const EDGEINFO& info);

            void print(ostream& o) const;

        };

        class MyNode : public Node
        {
        public:
            MyNode();

            MyNode(int nodeId, const NODEINFO& info);

            void print(ostream& o) const;

        };
    public:
        void print(ostream& o);

    };
}

#endif
