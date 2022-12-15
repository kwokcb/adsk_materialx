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


/// @class GraphIO
/// <summary>
///     Interface defining classes which can write a given non-MaterialX GraphElement
///     to another format.
///     The class indicates which formats are supported by string. 
///     The formatted input is assumed to be writeable to a string buffer
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
    /// Add a Element label to a subgraph list. Given a node and label, the label will to bused to add an identifier to the subgraph list.
    /// @param subGraphs Structure to maintain a list of unique node names per subgraph name. The subgraph name is the full path name.
    /// @param node The parent of this node if it exists is the subgraph to add the label to
    /// @param label The string used to create a uniquely labelled element in the subgraph. The subgraph path will be prepended to the lable
    /// @return Derived label name
    /// </summary>
    string addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, const ElementPtr node, const string& label) const;

    virtual string writeGraph(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true);
      
    virtual string writeRootNode(const string& /*rootName*/,
                                 const string& /*rootLabel*/)
    {
        return EMPTY_STRING;
    }

    /// Write upstream node and label 
    /// @param 
    /// @param
    /// @return
    virtual string writeUpstreamNode(const string& /*nodeName*/,
        const string& /*nodeLabel*/)
    {
        return EMPTY_STRING;
    }

    // Write the connection from an upstream node to a
    // downstream node. Include upstream output and downstream
    // inputs if specified
    /// @param 
    /// @param
    /// @return
    virtual string writeConnectingElement(const string& /*upstreamPortLabel*/,
        const string& /*upstreamPort*/, const string& /*connectingElementString*/) 
    {
        return EMPTY_STRING;
    }

    /// Write interface connection
    /// @param interafaceId Identifier for interface
    /// @param interfaceInputName Identifier for interface input
    /// @param inputName Name of input on interior node
    /// @param inputNodeName Name of interior node
    virtual string writeInterfaceConnection(
        const string& /*interfaceId*/,
        const string& /*interfaceInputName*/,
        const string& /*inputName*/,
        const string& /*inputNodeName*/)
    {
        return EMPTY_STRING;
    }

    /// Write downstream node and label
    /// @param 
    /// @param
    /// @return
    virtual string writeDownstreamNode(const string& /*nodeName*/,
        const string& /*nodeLabel*/, const string& /*category*/) 
    {
        return EMPTY_STRING;
    };

    virtual string writeSubgraphs(std::unordered_map<string, StringSet> /*subGraphs*/)
    {
        return EMPTY_STRING;
    }

    virtual string writeGraphString(const string& /*graphString*/,
        const string& /*orientation*/)
    {
        return EMPTY_STRING;
    }

    StringSet _formats;
};

class MX_FORMAT_API DotGraphIO : public GraphIO
{
  public:
    DotGraphIO()
    {
        _formats.insert("dot");
    }
    virtual ~DotGraphIO() = default;

    static DotGraphIOPtr create();

    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) override;
};

class MX_FORMAT_API MermaidGraphIO : public GraphIO
{
  public:
    MermaidGraphIO()
    {
        _formats.insert("md");
        _formats.insert("mmd");
    }
    virtual ~MermaidGraphIO() = default;

    /// Creator
    static MermaidGraphIOPtr create();

    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) override;

  protected:
    string writeRootNode(const string& rootName, 
                         const string&rootLabel) override;

    string writeUpstreamNode(
        const string& nodeName, 
        const string& nodeLabel) override;
    string writeConnectingElement(
        const string& outputName,
        const string& outputLabel, 
        const string& inputLabel) override;
    string writeInterfaceConnection(
        const string& /*interfaceId*/,
        const string& /*interfaceInputName*/,
        const string& /*inputName*/,
        const string& /*inputNodeName*/) override;
    string writeDownstreamNode(
        const string& nodeName,
        const string& nodeLabel, 
        const string& category) override;

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
/// A registry for graph IO.
class MX_FORMAT_API GraphIORegistry
{
  public:
    virtual ~GraphIORegistry() { }

    /// Creator
    static GraphIORegistryPtr create();

    /// Add a graph IO 
    void addGraphIO(GraphIOPtr graphIO);

    /// 
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
