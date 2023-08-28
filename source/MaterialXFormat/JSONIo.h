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

class JSONReadOptions;

extern MX_FORMAT_API const string JSON_EXTENSION;
extern MX_FORMAT_API const string JSON_MIME_TYPE;

/// @class JSONReadOptions
/// A set of options for controlling the behavior of JSON read functions.
class MX_FORMAT_API JSONReadOptions
{
public:
    JSONReadOptions() {};
    ~JSONReadOptions() { }

    /// If true, then documents from earlier versions of MaterialX will be upgraded
    /// to the current version.  Defaults to true.
    bool upgradeVersion = true;
};

/// @class JSONWriteOptions
/// A set of options for controlling the behavior of JSON write functions.
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

    /// Indentation. 2 by default
    unsigned int indent = 2;
};

/// @name Read Functions
/// @{

/// Read a Document as JSON from the given character buffer.
/// @param doc The Document into which data is read.
/// @param buffer The string buffer from which data is read.
/// @param readOptions An optional pointer to an JSONReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
MX_FORMAT_API void readFromJSONString(DocumentPtr doc, const string& buffer, const JSONReadOptions* readOptions = nullptr);

/// Read a Document as JSON from the given input stream.
/// @param doc The Document into which data is read.
/// @param stream The input stream from which data is read.
/// @param readOptions An optional pointer to an JSONReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
MX_FORMAT_API void readFromJSONStream(DocumentPtr doc, std::istream& stream, const JSONReadOptions* readOptions = nullptr);

/// Read a Document as JSON from the given filename.
/// @param doc The Document into which data is read.
/// @param filename The filename from which data is read.  This argument can
///    be supplied either as a FilePath or a standard string.
/// @param readOptions An optional pointer to an JSONReadOptions object.
///    If provided, then the given options will affect the behavior of the read
///    function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
/// @throws ExceptionFileMissing if the file cannot be opened.
MX_FORMAT_API void readFromJSONFile(DocumentPtr doc,
    FilePath filename,
    FileSearchPath searchPath = FileSearchPath(),
    const JSONReadOptions* readOptions = nullptr);

/// @}
/// @name Write Functions

/// Write a Document as JSON to the given filename.
/// @param doc The Document to be written.
/// @param filename The filename to which data is written.  This argument can
///    be supplied either as a FilePath or a standard string.
/// @param writeOptions An optional pointer to an JSONWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
MX_FORMAT_API void writeToJSONFile(DocumentPtr doc, const FilePath& filename, const JSONWriteOptions* writeOptions = nullptr);

/// Write a Document as JSON to a new string, returned by value.
/// @param doc The Document to be written.
/// @param writeOptions An optional pointer to an JSONWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
/// @return The output string, returned by value
MX_FORMAT_API string writeToJSONString(DocumentPtr doc, const JSONWriteOptions* writeOptions = nullptr);

/// Write a Document as JSON to the given output stream.
/// @param doc The Document to be written.
/// @param stream The output stream to which data is written
/// @param writeOptions An optional pointer to an JSONWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
MX_FORMAT_API void writeToJSONStream(DocumentPtr doc, std::ostream& stream, const JSONWriteOptions* writeOptions);

/// @}

/// @}


MATERIALX_NAMESPACE_END

#endif
