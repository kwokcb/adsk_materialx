//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GRAPHIO_H
#define MATERIALX_GRAPHIO_H

/// @file
/// Support for the MaterialX GraphElement interchange classes

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/Export.h>
#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

class GraphIO;
class DotGraphIO;
class MermaidGraphIO;

/// A shared pointer to a GraphIO
using GraphIOPtr = shared_ptr<GraphIO>;
/// A shared pointer to a const GraphIO
using ConstGraphIOPtr = shared_ptr<const GraphIO>;

/// A shared pointer to a DotGraphIO
using DotGraphIOPtr = shared_ptr<DotGraphIO>;
/// A shared pointer to a const GraphIO
using ConstDotGraphIOPtr = shared_ptr<const DotGraphIO>;

/// A shared pointer to a MermaidGraphIO
using MermaidGraphIOPtr = shared_ptr<MermaidGraphIO>;
/// A shared pointer to a const MermaidGraphIO
using ConstMermaidGraphIOPtr = shared_ptr<const MermaidGraphIO>;

/// @class NodeIO
///     Node information extracted during graph traversal and 
///     provided to utility writer methods. This includes user
///     interface information hints such as UI label and shape.
/// 
class MX_FORMAT_API NodeIO
{
  public:
    /// UI node shapes
    enum NodeShape
    {
        Box = 0,        /// Box shape. Used for non interface nodes
        RoundedBox = 1, /// Rounded box shape. Used to indicate interface input and output nodes
        Diamond = 1,    /// Diamond shape. Used to indicate conditionals.
    };

    /// Uniique Node identifier. This identifier is unique per MaterialX Document.
    string identifier;

    /// Node UI label for the identifier
    string uilabel;

    /// MaterialX node category string
    string category;

    /// MaterialX node group string
    string group;

    /// Node UI shape. Default is box.
    NodeShape uishape = Box;
};

/// @class GraphIO
/// <summary>
///     Interface defining classes which can write a given non-MaterialX GraphElement
///     to another format The formatted input is assumed to be writeable to 
///     a string.
/// 
///     The class indicates which formats are supported by a list of strings.
///     This is used to register the interface with a GraphIORegistry.
/// 
///     Default traversal logic is provided which calls into a set of utities
///     which are responsdible for producing the appropriate string output.
///     A defived class may chose to implement these methods or write their
///     own traversal logic.
///     
/// </summary>
class MX_FORMAT_API GraphIO
{
  public:
    GraphIO(){};
    virtual ~GraphIO(){};

    /// Returns a list of formats that the GraphIO can convert from MaterialX to a given format
    const StringSet& supportsFormats() const
    {
        return _formats;
    }

    /// Traverse a graph and return a string
    /// Derived classes must implement this method
    /// @param graph GraphElement to write
    /// @param roots Optional list of roots to GraphIO what upstream elements to consider>
    /// @param writeCategoryNames Use names of categories versus instance names for nodes. Default is true.
    /// @returns Buffer result
    virtual string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) = 0;

  protected:
    /// @name Graph Writing Utilties
    /// @{

    /// Traverse a graph and return a string. If used the additional utility
    /// methods must be implemented with the exception of writeSubgraphs()
    /// which is used to create groupings of nodes.
    /// 
    /// @param graph GraphElement to write
    /// @param roots Optional list of roots to GraphIO what upstream elements to consider>
    /// @param writeCategoryNames Use names of categories versus instance names for nodes. Default is true.
    /// @returns Buffer result
    virtual string writeGraph(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames);
      
    /// Write root node only. Called when there are no downstream.
    /// connections.
    /// @param root Root node
    /// @return Written string result
    virtual string writeRootNode(const NodeIO& root)
    {
        (void)(root);
        return EMPTY_STRING;
    }

    /// Write upstream node and label 
    /// @param node Write upstream node
    /// @return Written string result
    virtual string writeUpstreamNode(const NodeIO& node)
    {
        (void)(node);
        return EMPTY_STRING;
    }

    /// Write the connection from an upstream node to a
    /// downstream node. Include upstream portand downstream
    /// input if specified.
    /// @param upstreamPortLabel ui label for output on upstream node
    /// @param upstreamPort name of output on upstream node
    /// @param inputName name of input on downstream node
    /// @return Written string result
    virtual string writeConnection(
        const string& upstreamPortLabel,
        const string& upstreamPort, 
        const string& inputName) 
    {
        (void)(upstreamPortLabel);
        (void)(upstreamPort);
        (void)(inputName);
        return EMPTY_STRING;
    }

    /// Write interface connection
    /// @param interfaceId Identifier for interface
    /// @param interfaceInputName Identifier for interface input
    /// @param inputName Name of input on interior node
    /// @param interiorNode Interior node information
    /// @return Written string result
    virtual string writeInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode)
    {
        (void)(interfaceId);
        (void)(interfaceInputName);
        (void)(inputName);
        (void)(interiorNode);
        return EMPTY_STRING;
    }

    /// Write downstream node and label
    /// @param node Node information to write
    /// @param inputLabel input on node
    /// @return Written string result
    virtual string writeDownstreamNode(const NodeIO& node,
                                       const string& inputLabel)
    {
        (void)(node);
        (void)(inputLabel);
        return EMPTY_STRING;
    }

    /// Write sub-graph groupings. For now the only subgraphs supported
    /// are NodeGraphs
    /// @param  subGraphs List of sub-graphs to write
    /// @return Written string result
    virtual string writeSubgraphs(std::unordered_map<string, StringSet> subGraphs)
    {
        (void)(subGraphs);
        return EMPTY_STRING;
    }

    /// Write GraphElement
    /// @param graphString Name to use for the graph 
    /// @param orientation Orientation of the graph
    /// @return Written string result
    virtual string writeGraphString(const string& graphString,
                                    const string& orientation)
    {
        (void)(graphString);
        (void)(orientation);
        return EMPTY_STRING;
    }

    /// @}
    /// @name Subgraph Handling Utilities
    /// @{

    /// Add an Element label subgraph list. 
    /// Given a node and label, the label will to be used to add an identifier to the subgraph list.
    /// @param subGraphs Structure to maintain a list of unique node identifiers per subgraph name. The subgraph name is a unique Document name.
    /// @param node The parent of this node if it exists is the subgraph to add the label to
    /// @param label The string used to create a uniquely labelled element in the subgraph. The subgraph path will be prepended to the lable
    /// @return Derived label name
    /// </summary>
    string addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, 
                             const ElementPtr node, 
                             const string& label) const;

    /// @}

    /// Storage for formats supported
    StringSet _formats;

    /// Map containing restricted keywords and their replacement for identifiers
    StringMap _restrictedMap;
};

class MX_FORMAT_API DotGraphIO : public GraphIO
{
public:
    DotGraphIO()
    {
        // Dot files
        _formats.insert("dot");
    }
    virtual ~DotGraphIO() = default;

    static DotGraphIOPtr create();

    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames) override;

  protected:
    string writeRootNode(const NodeIO& root) override;
    string writeUpstreamNode(const NodeIO& node) override;
    string writeConnection(
      const string& upstreamPortLabel,
        const string& upstreamPort, 
        const string& inputName) override;
    string writeInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode) override;
    string writeDownstreamNode(const NodeIO& node, const string& inputName) override;
    string writeSubgraphs(
        std::unordered_map<string, StringSet> subGraphs) override;
    string writeGraphString(const string& graphString, const string& orientation) override;
};
    

class MX_FORMAT_API MermaidGraphIO : public GraphIO
{
  public:
    MermaidGraphIO()
    {
        // Markdown and Markdown diagrams
        _formats.insert("md");
        _formats.insert("mmd");

        // Add restricted keywords
        _restrictedMap["default"] = "dfault";
    }
    virtual ~MermaidGraphIO() = default;

    /// Creator
    static MermaidGraphIOPtr create();

    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames) override;

  protected:
    string writeRootNode(const NodeIO& root) override;
    string writeUpstreamNode(const NodeIO& node) override;
    string writeConnection(
      const string& /*upstreamPortLabel*/,
        const string& /*upstreamPort*/, 
        const string& /*connectingElementString*/) override;
    string writeInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode) override;
    string writeDownstreamNode(const NodeIO& node, const string& inputName) override;
    string writeSubgraphs(
        std::unordered_map<string, StringSet> subGraphs) override;
    string writeGraphString(const string& graphString, const string& orientation) override;
};

/// Map of graph IO
using GraphIOPtrMap = std::unordered_map<string, std::vector<GraphIOPtr>>;

/// GraphIO registry
class GraphIORegistry;
using GraphIORegistryPtr = std::shared_ptr<GraphIORegistry>;

/// @class GraphIORegistry
/// A registry for graph IO interfaces. 
/// * GraphUI classes can register for for one or more formats. 
/// * Latter registrations will override previous ones.
/// 
class MX_FORMAT_API GraphIORegistry
{
  public:
    virtual ~GraphIORegistry() { }

    /// Creator
    static GraphIORegistryPtr create();

    /// Add a graph IO 
    void addGraphIO(GraphIOPtr graphIO);

    /// Write a GraphElement to a given format
    /// @param format Target format
    /// @param graph GraphElement to write
    /// @param roots List of possible roots
    /// @param writeCategoryNames Write node labels using the category of the node as the label as
    ///     opposed to the unique name of the Element. Default is to write category names.
    string write(const string& format, GraphElementPtr graph, const std::vector<OutputPtr> roots, 
                bool writeCategoryNames = true);

  private:
    GraphIORegistry(const GraphIORegistry&) = delete;
    GraphIORegistry() { }

    GraphIORegistry& operator=(const GraphIORegistry&) = delete;

  private:
    GraphIOPtrMap _graphIOs;
};

MATERIALX_NAMESPACE_END

#endif
