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

class JsonWriteOptions;

extern MX_FORMAT_API const string JSON_EXTENSION;
extern MX_FORMAT_API const string JSON_MIME_TYPE;
extern MX_FORMAT_API const string JSON_CATEGORY_NAME_SEPARATOR;

/// @class JsonWriteOptions
/// A set of options for controlling the behavior of JSON read functions.
class MX_FORMAT_API JsonReadOptions
{
public:
    JsonReadOptions() {};
    ~JsonReadOptions() { }

    /// If true, then documents from earlier versions of MaterialX will be upgraded
    /// to the current version.  Defaults to true.
    bool upgradeVersion = true;
};

/// @class JsonWriteOptions
/// A set of options for controlling the behavior of JSON write functions.
class MX_FORMAT_API JsonWriteOptions
{
public:
    JsonWriteOptions() = default;
    ~JsonWriteOptions() {}

    /// If provided, this function will be used to exclude specific elements
    /// (those returning false) from the write operation.  Defaults to nullptr.
    ElementPredicate elementPredicate;

    /// Indentation. 2 spaces by default
    unsigned int indent = 2;
    char indentCharacter = ' ';
};

/// @name Read Functions
/// @{

/// Read a Document as JSON from the given character buffer.
/// @param doc The Document into which data is read.
/// @param buffer The string buffer from which data is read.
/// @param readOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
MX_FORMAT_API void readFromJsonString(DocumentPtr doc, const string& buffer, const JsonReadOptions* readOptions = nullptr);

/// Read a Document as JSON from the given input stream.
/// @param doc The Document into which data is read.
/// @param stream The input stream from which data is read.
/// @param readOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
MX_FORMAT_API void readFromJsonStream(DocumentPtr doc, std::istream& stream, const JsonReadOptions* readOptions = nullptr);

/// Read a Document as JSON from the given filename.
/// @param doc The Document into which data is read.
/// @param filename The filename from which data is read.  This argument can
///    be supplied either as a FilePath or a standard string.
/// @param readOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the read
///    function.  Defaults to a null pointer.
/// @param searchPath An optional sequence of file paths that will be applied
///    in order when searching for the given file and its includes.  This
///    argument can be supplied either as a FileSearchPath, or as a standard
///    string with paths separated by the PATH_SEPARATOR character.
/// @throws ExceptionParseError if the document cannot be parsed.
/// @throws ExceptionFileMissing if the file cannot be opened.
MX_FORMAT_API void readFromJsonFile(DocumentPtr doc,
                   FilePath filename,
                   FileSearchPath searchPath = FileSearchPath(),
                   const JsonReadOptions* readOptions = nullptr);

/// @}
/// @name Write Functions
/// @{

/// Write a Document as JSON to the given filename.
/// @param doc The Document to be written.
/// @param filename The filename to which data is written.  This argument can
///    be supplied either as a FilePath or a standard string.
/// @param writeOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
MX_FORMAT_API void writeToJsonFile(DocumentPtr doc, const FilePath& filename, const JsonWriteOptions* writeOptions = nullptr);

/// Write a Document as JSON to a new string, returned by value.
/// @param doc The Document to be written.
/// @param writeOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
/// @return The output string, returned by value
MX_FORMAT_API string writeToJsonString(DocumentPtr doc, const JsonWriteOptions* writeOptions = nullptr);

/// Write a Document as JSON to the given output stream.
/// @param doc The Document to be written.
/// @param stream The output stream to which data is written
/// @param writeOptions An optional pointer to an JsonWriteOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
MX_FORMAT_API void writeToJsonStream(DocumentPtr doc, std::ostream& stream, const JsonWriteOptions* writeOptions);

/// @}

MATERIALX_NAMESPACE_END

#endif
