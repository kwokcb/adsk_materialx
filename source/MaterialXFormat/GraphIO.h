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

/// @class GraphIO
/// <summary>
///     Interface defining classes which can either read or write a given non-MaterialX GraphElement
///     to or from MaterialX respectively.
///
///     The class indicates which formats are supported and may either support reading / writing or
///     both.
///
///     The formatted input is assumed to be readable from and / or writeable to a string buffer
/// </summary>
class MX_FORMAT_API GraphIO
{
  public:
    GraphIO(){};
    virtual ~GraphIO(){};

    /// Returns list of formats that the GraphIO can read and convert to MaterialX
    /// @return List of supported formats</returns>
    const StringSet& readFormats() const
    {
        return _readFormats;
    }

    /// Returns a list of formats that the GraphIO can convert from MaterialX to a given format
    const StringSet& writeFormats() const
    {
        return _writeFormats;
    }

    /// Parse the input buffer and return a GraphElement
    /// Derived classes must implement this method
    /// @param inputBuffer Buffer to read from
    /// @return GraphElement result from converting the input
    virtual const GraphElementPtr read(const string& inputBuffer) = 0;

    /// Traverse a graph and return a string
    /// Derived classes must implement this method
    /// @param graph GraphElement to write
    /// @param roots Optional list of roots to GraphIO what upstream elements to consider>
    /// @param writeCategoryNames Use names of categories versus instance names for nodes. Default is true.
    /// @returns Buffer result
    virtual string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) = 0;

  protected:
    StringSet _readFormats;
    StringSet _writeFormats;
};

class MX_FORMAT_API DotGraphIO : public GraphIO
{
  public:
    DotGraphIO()
    {
        _writeFormats.insert("dot");
    }
    virtual ~DotGraphIO() = default;

    const GraphElementPtr read(const string& inputBuffer) override 
    {
      return nullptr;
    }
    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) override
    {
      return EMPTY_STRING;
    }
};

class MX_FORMAT_API MermaidGraphIO : public GraphIO
{
  public:
    MermaidGraphIO()
    {
        _writeFormats.insert("md");
        _writeFormats.insert("mmd");
    }
    virtual ~MermaidGraphIO() = default;

    const GraphElementPtr read(const string& inputBuffer) override;
    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames = true) override;

  protected:
    /// Add a Element label to a subgraph list. Given a node and label, the label will to bused to add an identifier to the subgraph list.
    /// @param subGraphs Structure to maintain a list of unique node names per subgraph name. The subgraph name is the full path name.
    /// @param node The parent of this node if it exists is the subgraph to add the label to
    /// @param label The string used to create a uniquely labelled element in the subgraph. The subgraph path will be prepended to the lable
    /// @return Derived label name
    /// </summary>
    string addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, const ElementPtr node, const string& label) const;
};

MATERIALX_NAMESPACE_END

#endif
