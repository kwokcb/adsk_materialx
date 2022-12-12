//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_FILTER_H
#define MATERIALX_FILTER_H

/// @file
/// Support for the MaterialX file format filters

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/Export.h>
#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

/// @class Filter
/// <summary>
///     Interface defining classes which can either read or write a given non-MaterialX format
///     to or from MaterialX respectively.
/// 
///     The class indicates which formats are supported and may either support reading / writing or
///     both.
/// 
///     The formatted input is assumed to be readable from and / or writeable to a string buffer
/// </summary>
class MX_FORMAT_API Filter
{
  public:
    Filter() {};
    virtual ~Filter() {};

    /// Returns list of formats that the filter can read and convert to MaterialX
    /// @return List of supported formats</returns>
    const StringSet& readFormats() const
    {
        return _readFormats;
    }

    /// Returns a list of formats that the filter can convert from MaterialX to a given format
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
    /// @param roots Optional list of roots to filter what upstream elements to consider>
    /// @param writeCategoryNames Use names of categories versus instance names for nodes. Default is true.
    /// @returns Buffer result
    virtual string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames=true) = 0;

  protected:
   StringSet _readFormats;
   StringSet _writeFormats;
};

class MX_FORMAT_API MermaidFilter : public Filter
{
  public:
    MermaidFilter()
    {
        _writeFormats.insert("md");
        _writeFormats.insert("mmd");
    }
    virtual ~MermaidFilter() = default;

    const GraphElementPtr read(const string& inputBuffer) override;
    string write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames=true) override;
};


MATERIALX_NAMESPACE_END

#endif
