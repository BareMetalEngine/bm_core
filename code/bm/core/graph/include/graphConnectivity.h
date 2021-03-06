/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: graph #]
***/

BEGIN_INFERNO_NAMESPACE_EX(graph)

// graph connectivity info
class BM_CORE_GRAPH_API Connectivity : public MainPoolData<NoCopy>
{
public:
    Connectivity(Container& graph);
    ~Connectivity();

    // get list of blocks sorted by the depth
    void buildReachabilityList(const Array<Block*>& outputList, Array<Block*>& outList);

private:
    struct Node;

    struct Edge : public MainPoolData<NoCopy>
    {
    public:
        StringID m_sourceName;
        StringID m_targetName;
        Node* m_target;

        INLINE Edge()
            : m_target(nullptr)
        {}
    };

    struct Node : public MainPoolData<NoCopy>
    {
    public:
        int m_depth = -1;
        int m_island = -1;
        Block* m_block = nullptr;
        Array<Edge> m_inputEdges;
        Array<Edge> m_outputEdges;
        bool m_marker = false;

        INLINE Node(Block* block)
            : m_block(block)
        {}
    };

    //---

    HashMap<const Block*, Node*> m_nodeMap;
    Array<Node*> m_nodes;

    // create 1-1 graph nodes
    void createNodes(Container& graph);

    // create node connections
    void createConnections();

    // resolve connection
    void resolveConnection(const Connection* con, const Socket* source, Array<Edge>& outEdges);

    // reset depths at blocks
    void resetDepths();

    // follow the output links of the block
    int followOutputLinks(Node* node, int depth);
};

END_INFERNO_NAMESPACE_EX(graph)
