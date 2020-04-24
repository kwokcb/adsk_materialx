//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/Util.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace MaterialX
{

string removeExtension(const string& filename)
{
    size_t lastDot = filename.find_last_of('.');
    if (lastDot == string::npos) return filename;
    return filename.substr(0, lastDot);
}

string readFile(const string& filename)
{
    std::ifstream file(filename, std::ios::in);
    if (file)
    {
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        if (stream)
        {
            return stream.str();
        }
    }
    return EMPTY_STRING;
}

void loadDocuments(const FilePath& rootPath, const FileSearchPath& searchPath, const StringSet& skipFiles,
                   const StringSet& includeFiles, vector<DocumentPtr>& documents, StringVec& documentsPaths,
                   const XmlReadOptions& readOptions, StringVec& errors)
{
    for (const FilePath& dir : rootPath.getSubDirectories())
    {
        for (const FilePath& file : dir.getFilesInDirectory(MTLX_EXTENSION))
        {
            if (!skipFiles.count(file) &&
                (includeFiles.empty() || includeFiles.count(file)))
            {
                DocumentPtr doc = createDocument();
                const FilePath filePath = dir / file;
                try
                {
                    FileSearchPath readSearchPath(searchPath);
                    readSearchPath.append(dir);
                    readFromXmlFile(doc, filePath, readSearchPath, &readOptions);
                    documents.push_back(doc);
                    documentsPaths.push_back(filePath.asString());
                }
                catch (Exception& e)
                {
                    errors.push_back("Failed to load: " + filePath.asString() + ". Error: " + e.what());
                }
            }
        }
    }
}

void loadLibrary(const FilePath& file, DocumentPtr doc, const FileSearchPath* searchPath, XmlReadOptions* readOptions)
{
    DocumentPtr libDoc = createDocument();
    XmlReadOptions localOptions;
    localOptions.skipConflictingElements = true;
    if (!readOptions)
    {
        readOptions = &localOptions;
    }
    else
    {
        readOptions->skipConflictingElements = true;
    }
    readFromXmlFile(libDoc, file, searchPath ? *searchPath : FileSearchPath(), readOptions);
    CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

StringVec loadLibraries(const StringVec& libraryNames,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles,
                        XmlReadOptions* readOptions)
{
    StringVec loadedLibraries;
    for (const std::string& libraryName : libraryNames)
    {
        FilePath libraryPath = searchPath.find(libraryName);
        for (const FilePath& path : libraryPath.getSubDirectories())
        {
            for (const FilePath& filename : path.getFilesInDirectory(MTLX_EXTENSION))
            {
                if (!excludeFiles || !excludeFiles->count(filename))
                {
                    const FilePath& file = path / filename;
                    loadLibrary(file, doc, &searchPath, readOptions);
                    loadedLibraries.push_back(file.asString());
                }
            }
        }
    }
    return loadedLibraries;
}

StringVec loadLibraries(const StringVec& libraryNames,
                        const FilePath& filePath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles)
{
    FileSearchPath searchPath;
    searchPath.append(filePath);
    return loadLibraries(libraryNames, searchPath, doc, excludeFiles);
}

}
