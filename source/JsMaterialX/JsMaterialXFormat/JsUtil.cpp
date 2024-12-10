#include <JsMaterialX/Helpers.h>
#include <MaterialXFormat/Util.h>

#include <emscripten/bind.h>
#include <iostream>

namespace ems = emscripten;
namespace mx = MaterialX;

mx::DocumentPtr jsLoadLibraries(const mx::StringVec& libraryFolders, const mx::StringVec searchPaths)
{
    mx::DocumentPtr result;
    mx::FilePathVec libraryFolderPaths;
    for (const std::string& folder : libraryFolders)
    {
        libraryFolderPaths.push_back(folder);
    }
    mx::FileSearchPath fileSearchPath;
    for (const std::string& path : searchPaths)
    {
        fileSearchPath.append(mx::FilePath(path));
    }

    try
    {
        result = mx::createDocument();
        mx::StringSet _xincludeFiles = mx::loadLibraries(libraryFolderPaths, fileSearchPath, result);
        if (_xincludeFiles.empty())
        {
            std::cerr << "Could not find libraries on the given search path: " << fileSearchPath.asString() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to load libraries: " << e.what() << std::endl;
        return nullptr;
    }

    return result;
}

std::string jsGetDefaultDataSearchPath() 
{
    return "/";
}

mx::StringVec jsGetDefaultDataLibraryFolders()
{
    mx::StringVec defaultLibraryFolders = { "libraries" };
    return defaultLibraryFolders;
}

EMSCRIPTEN_BINDINGS(xformatUtil)
{
  ems::constant("PATH_LIST_SEPARATOR", mx::PATH_LIST_SEPARATOR);
  ems::constant("MATERIALX_SEARCH_PATH_ENV_VAR", mx::MATERIALX_SEARCH_PATH_ENV_VAR);

  ems::function("getDefaultDataLibraryFolders", &jsGetDefaultDataLibraryFolders);
  ems::function("getDefaultDataSearchPath", &jsGetDefaultDataSearchPath);
  ems::function("loadLibraries", &jsLoadLibraries);
}

