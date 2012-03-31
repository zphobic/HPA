//-----------------------------------------------------------------------------
/** @file graph.h
    Graph class.

    $Id: graph.h,v 1.9 2003/05/20 17:24:50 adib Exp $
    $Source: /usr/cvsroot/project_pathfind/graph.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_GRAPH_H
#define PATHFIND_GRAPH_H
#include <assert.h>

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    template<class NODEINFO, class EDGEINFO>
    class Graph
    {
    public:
        class Edge
        {
        public:
            Edge();

            Edge(int targetNodeId, const EDGEINFO& info);

            int getTargetNodeId() const
            {
                return m_targetNodeId;
            }

            EDGEINFO& getInfo()
            {
                return m_info;
            }

            const EDGEINFO& getInfo() const
            {
                return m_info;
            }

        private:
            int m_targetNodeId;
            EDGEINFO m_info;
        };

        class Node
        {
        public:
            Node();

            Node(int nodeId, const NODEINFO& info);

            void addOutEdge(const Edge& edge);

            void removeOutEdge(int targetNodeId);

            void clearOutEdges();

            NODEINFO& getInfo()
            {
                return m_info;
            }

            const NODEINFO& getInfo() const
            {
                return m_info;
            }

            const vector<Edge>& getOutEdges() const;

        private:
            int m_nodeId;
            vector<Edge> m_outEdges;
            NODEINFO m_info;
        };

        void addNode(int nodeId, const NODEINFO& info);

        void addOutEdge(int sourceNodeId, int targetNodeId,
                        const EDGEINFO& info);

        void removeNodeEdges(int nodeId);
        void removeLastNode();
        void removeOutEdge(int sourceNodeId, int targetNodeId);

        void clear();

        const Node& getNode(int nodeId) const
        {
            assert(isValidNodeId(nodeId));
            return m_nodes[nodeId];
        }

        NODEINFO& getNodeInfo(int nodeId)
        {
            return getNode(nodeId).getInfo();
        }

        const NODEINFO& getNodeInfo(int nodeId) const
        {
            return getNode(nodeId).getInfo();
        }

        const vector<Edge>& getOutEdges(int nodeId) const;

    private:
        vector<Node> m_nodes;

        Node& getNode(int nodeId)
        {
            assert(isValidNodeId(nodeId));
            return m_nodes[nodeId];
        }
        bool isValidNodeId(int nodeId) const;
    };
}

//-----------------------------------------------------------------------------
// Inline/template methods.
//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    template<class NODEINFO, class EDGEINFO>
    Graph<NODEINFO, EDGEINFO>::Edge::Edge(int targetNodeId,
                                          const EDGEINFO& info)
        : m_targetNodeId(targetNodeId),
          m_info(info)
    {
    }

    template<class NODEINFO, class EDGEINFO>
    Graph<NODEINFO, EDGEINFO>::Node::Node()
        : m_nodeId(-1)
    {
    }

    template<class NODEINFO, class EDGEINFO>
    Graph<NODEINFO, EDGEINFO>::Node::Node(int nodeId, const NODEINFO& info)
        : m_nodeId(nodeId),
          m_info(info)
    {
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::Node::addOutEdge(const Edge& edge)
    {
        m_outEdges.push_back(edge);
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::Node::removeOutEdge(int targetNodeId)
    {
		vector<Edge> ve;
        typename vector<Edge>::iterator i;
        for (i = m_outEdges.begin();
             i != m_outEdges.end(); ++i)
        {
            if (i->getTargetNodeId() == targetNodeId)
            {
                m_outEdges.erase(i);
                break;
            }
        }
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::Node::clearOutEdges()
    {
        m_outEdges.clear();
    }

    template<class NODEINFO, class EDGEINFO>
    const vector<typename Graph<NODEINFO, EDGEINFO>::Edge>&
    Graph<NODEINFO, EDGEINFO>::Node::getOutEdges() const
    {
        return m_outEdges;
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::addNode(int nodeId, const NODEINFO& info)
    {
        assert(nodeId >= 0);
        unsigned int size = static_cast<unsigned int>(nodeId + 1);
        if (m_nodes.size() < size)
            m_nodes.resize(size);
        m_nodes[nodeId] = Node(nodeId, info);
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::removeNodeEdges(int nodeId)
    {
        const vector<Edge>& edges = m_nodes[nodeId].getOutEdges();
        for (typename vector<Edge>::const_iterator i = edges.begin();
             i != edges.end(); ++i)
        {
            m_nodes[i->getTargetNodeId()].removeOutEdge(nodeId);
        }
        m_nodes[nodeId].clearOutEdges();
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::removeLastNode()
    {
        m_nodes.erase(m_nodes.end());
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::addOutEdge(int sourceNodeId,
                                               int targetNodeId,
                                               const EDGEINFO& info)
    {
        assert(isValidNodeId(sourceNodeId));
        assert(isValidNodeId(targetNodeId));
        m_nodes[sourceNodeId].addOutEdge(Edge(targetNodeId, info));
    }

    template<class NODEINFO, class EDGEINFO>
    void Graph<NODEINFO, EDGEINFO>::clear()
    {
        m_nodes.clear();
    }

    template<class NODEINFO, class EDGEINFO>
    const vector<typename Graph<NODEINFO, EDGEINFO>::Edge>&
    Graph<NODEINFO, EDGEINFO>::getOutEdges(int nodeId) const
    {
        assert(isValidNodeId(nodeId));
        return m_nodes[nodeId].getOutEdges();
    }

    template<class NODEINFO, class EDGEINFO>
    bool Graph<NODEINFO, EDGEINFO>::isValidNodeId(int nodeId) const
    {
        return nodeId >= 0
            && static_cast<unsigned int>(nodeId) < m_nodes.size();
    }
}

#endif
