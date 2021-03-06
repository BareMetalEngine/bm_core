/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: graph #]
***/

#pragma once

#include "bm/core/object/include/object.h"

BEGIN_INFERNO_NAMESPACE_EX(graph)

//---

/// persistent connection data
/// used to save/load the connections, the actual connection objects are not saved (neither are the sockets)
struct BM_CORE_GRAPH_API PersistentConnection
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(PersistentConnection);

public:
    uint32_t firstBlockIndex;
    StringID firstSocketName;
    uint32_t secondBlockIndex;
    StringID secondSocketName;
};

//--

/// persistent block state
/// extract from the block layout before saving, applied after loading
/// NOTE: this struct allows us NOT to save the block layout as full object
struct BM_CORE_GRAPH_API PersistentBlockState
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(PersistentBlockState);

public:
    Point placement;

    PersistentBlockState();
};

//---

/// supported connection information
struct BM_CORE_GRAPH_API SupprotedBlockConnectionInfo
{
    RTTI_DECLARE_NONVIRTUAL_CLASS(SupprotedBlockConnectionInfo);

public:
    SpecificClassType<Block> blockClass;
    Array<StringID> socketNames; // compatible sockets

    SupprotedBlockConnectionInfo(SpecificClassType<Block> cls = nullptr);
};

//---

/// graph container, contains graph blocks
/// does not have to be saved to allow graph to be edited
class BM_CORE_GRAPH_API Container : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(Container, IObject);

public:
    Container();
    virtual ~Container();

    /// get graph blocks (read only)
    typedef Array< BlockPtr > TBlocks;
    INLINE const TBlocks& blocks() const { return m_blocks; }

    ///--
            
    /// attach a graph observer
    void attachObserver(IGraphObserver* observer);

    /// detach graph observer
    void dettachObserver(IGraphObserver* observer);

    ///--

    /// remove block from graph
    void removeBlock(const BlockPtr& block, bool modify=true);

    /// add block to graph
    void addBlock(const BlockPtr& block, bool modify = true);

    ///--

    /// test if adding a new block of this class is supported
    virtual bool canAddBlockOfClass(ClassType blockClass) const;

    /// can we delete given block
    virtual bool canDeleteBlock(const BlockPtr& block) const;

    /// get list of supported block classes
    virtual void supportedBlockClasses(Array<SpecificClassType<Block>>& outBlockClasses) const = 0;

    /// get list of supported blocks (and sockets) that can be connected to given block and socket
    virtual void supportedBlockConnections(const Socket* sourceSocket, Array<SupprotedBlockConnectionInfo>& outSupportedBlocks) const;

    /// invalidate cached data
    virtual void notifyStructureChanged();

    ///---

    /// extract given blocks into separate graph, internal links are preserved
    /// NOTE: the class of the graph container is preserved
    ContainerPtr extractSubGraph(const Array<BlockPtr>& fromBlocks) const;

private:
    // structure
    TBlocks m_blocks;

    // persistent data - filled in only when saving
    Array<PersistentConnection> m_persistentConnections;
    //mutable Array<PersistentBlockState> m_persistentBlockState;

    void applyConnections(const Array<PersistentConnection>& con);
    void writePersistentConnections(SerializationWriter& writer) const;
    void readPersistentConnections(SerializationReader& reader);


    //--

    // observers
    Array<IGraphObserver*> m_observers;

    // notify that block was added
    void notifyBlockAdded(Block* block);
    void notifyBlockRemoved(Block* block);
    void notifyBlockStyleChanged(Block* block);
    void notifyBlockLayoutChanged(Block* block);
    void notifyBlockConnectionsChanged(Block* block);

protected:
    virtual void onPostLoad() override;

    virtual void onReadBinary(SerializationReader& reader) override;
    virtual void onWriteBinary(SerializationWriter& writer) const override;

    friend class Block;
    friend class Socket;
};

//--

END_INFERNO_NAMESPACE_EX(graph)
