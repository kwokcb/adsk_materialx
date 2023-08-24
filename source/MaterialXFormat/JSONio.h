//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_JSON_H
#define MATERIALX_JSON_H

/// @file
/// Support for the MTLX file format in JSON

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/Export.h>
#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

class XmlReadOptions;

extern MX_FORMAT_API const string JSON_EXTENSION;


class MX_FORMAT_API JSONWriteOptions
{
public:
    JSONWriteOptions() = default;
    ~JSONWriteOptions() {}

    /// If provided, this function will be used to exclude specific elements
    /// (those returning false) from the write operation.  Defaults to nullptr.
    ElementPredicate elementPredicate;

    /// If true will add node definition information for each node encountered.
    bool addDefinitionInformation = true;

    /// Store node layout information
    bool storeLayoutInformation = true;

    /// Add nodegraph children
    bool addNodeGraphChildren = true;

    /// Indentation. 4 by default
    unsigned int indent = 4;
};

/// @name Write Functions
/// @{

MX_FORMAT_API void writeToJSONFile(DocumentPtr doc, const FilePath& filename, const JSONWriteOptions* writeOptions = nullptr);
MX_FORMAT_API string writeToJSONString(DocumentPtr doc, const JSONWriteOptions* writeOptions = nullptr);

/// @}

MATERIALX_NAMESPACE_END

#endif
