//-----------------------------------------------------------------------------
/** @file astar.h
    A* search algorithm.

    Pseudo algorithm for basic A* algorithm:
    <pre>
    priorityqueue Open
    list Closed

    AStarSearch
        s.g = 0 // s is the start node
        s.h = GoalDistEstimate(s)
        s.f = s.g + s.h
        s.parent = null
        push s on Open
        while Open is not empty
            pop node n from Open // n has the lowest f
            if n is a goal node
                construct path
                return success
            for each successor n' of n
                newg = n.g + cost(n,n')
                if n' is in Open or Closed
                   if n'.g <= newg skip
                   remove n' from Open or Closed
                n'.parent = n
                n'.g = newg
                n'.h = GoalDistEstimate(n')
                n'.f = n'.g + n'.h
                push n' on Open
            push n onto Closed
        return failure // if no path found
    </pre>

    $Id: astar.h,v 1.16 2003/05/22 18:50:08 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/astar.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ASTAR_H
#define PATHFIND_ASTAR_H

#include <list>
#include <memory>
#include <queue>
#include <set>
#include "search.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    /** A* search engine. */
    class AStar
        : public Search
    {
    public:
        explicit AStar(bool usePerfectHashClosedList);

        StatisticsCollection createStatistics();

        bool findPath(const Environment& env, int start, int target);

        const vector<int>& getPath() const
        {
            return m_path;
        }

        const StatisticsCollection& getStatistics() const;

        /** Get a vector with 'x' char labels for each visited node. */
        const vector<char>& getVisitedNodes() const
        {
            return m_visitedNodes;
        }

        void setNodesLimit(long long int nodesLimit)
        {
            m_nodesLimit = nodesLimit;
        }

        int getPathCost() const
        {
            return m_pathCost;
        }

    private:
        class AStarNode
        {
        public:
            class Compare
            {
            public:
                virtual ~Compare();

                bool operator()(const AStarNode& node1,
                                const AStarNode& node2);
            };

            int m_nodeId;

            int m_parent;

            int m_g;

            int m_h;

            int m_f;

            AStarNode()
            {
            }

            AStarNode(int nodeId, int parent, int g, int h)
                : m_nodeId(nodeId),
                  m_parent(parent),
                  m_g(g),
                  m_h(h),
                  m_f(g + h)
            {
            }

            void print(ostream& ostrm) const;
        };

        class ClosedListBase
        {
        public:
            virtual ~ClosedListBase();

            virtual void add(const AStarNode& node) = 0;

            vector<int> constructPath(int start, int target);

            /** Initialize for search.
                Must be called before using the closed list.
            */
            virtual void init(int maxNodeId) = 0;

            virtual void remove(int nodeId) = 0;

            virtual const AStarNode* search(int nodeId) const = 0;
        };

        /** Closed list implement using a list. */
        class ClosedList
            : public ClosedListBase
        {
        public:
            void add(const AStarNode& node);

            void init(int maxNodeId);

            void print(ostream& ostrm) const;

            void remove(int nodeId);

            const AStarNode* search(int nodeId) const;

        private:
            list<AStarNode> m_list;            
        };

        /** Closed list implement using perferct hash table. */
        class ClosedListPerfectHash
            : public ClosedListBase
        {
        public:
            ClosedListPerfectHash();

            void add(const AStarNode& node);

            void init(int maxNodeId);

            void print(ostream& ostrm) const;

            void remove(int nodeId);

            const AStarNode* search(int nodeId) const;

        private:
            int m_maxNodeId;

            vector<bool> m_marker;

            vector<AStarNode> m_nodes;            
        };

        class OpenQueue
        {
        public:
            OpenQueue();

            void init(int numberNodes);

            void insert(const AStarNode& node);

            bool isEmpty() const
            {
                return (m_nodes.size() == 0);
            }

            AStarNode pop();

            void print(ostream& ostrm) const;

            bool  remove(int nodeId);

            const AStarNode* search(int nodeId) const;

            int getSize() const
            {
                return m_nodes.size();
            }

        private:
            typedef multiset<AStarNode, AStarNode::Compare> NodeSet;

            typedef NodeSet::const_iterator NodeSetConstIterator;

            typedef NodeSet::iterator NodeSetIterator;

            int m_numberNodes;

            vector<bool> m_marker;

            /** Iterator in m_nodes by nodeId. */
            vector<NodeSetIterator> m_lookupTable;

            NodeSet m_nodes;
        };

        int m_pathCost;

        int m_target;

        const Environment* m_env;

        long long int m_nodesExpanded;

        long long int m_nodesVisited;

        auto_ptr<ClosedListBase> m_closed;

        vector<char> m_visitedNodes;

        vector<int> m_path;

        OpenQueue m_open;

        StatisticsCollection m_statistics;

        Statistics& m_branchingFactor;

        /** Find a node in open or closed lists. */
        const AStarNode* findNode(int nodeId);

        void findPathAStar(int start);

        /** Construct path and set statistics after target node was found. */
        void finishSearch(int start, const AStarNode& node);

        AStarNode getBestNodeFromOpen();
    };
}

//-----------------------------------------------------------------------------

#endif
