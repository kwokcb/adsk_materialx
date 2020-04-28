//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtFileIo.h>

namespace MaterialX
{

void PvtApi::reset()
{
    static const RtTypeInfo masterPrimRootType("api_masterprimroot");
    static const RtToken libRootName("api_libroot");

    _masterPrimRoot.reset(new PvtPrim(&masterPrimRootType, masterPrimRootType.getShortTypeName(), nullptr));
    _createFunctions.clear();
    _stages.clear();

    _libraryRoot.reset();
    _libraries.clear();
    _libraryRoot = RtStage::createNew(libRootName);

    _unitDefinitions = UnitConverterRegistry::create();
}

void PvtApi::loadLibrary(const RtToken& name)
{
    // If already loaded unload the old first,
    // to support reloading of updated libraries.
    if (getLibrary(name))
    {
        unloadLibrary(name);
    }

    RtStagePtr lib = RtStage::createNew(name);
    _libraries[name] = lib;

    RtFileIo file(lib);
    file.readLibraries({ name }, _searchPaths);

    _libraryRoot->addReference(lib);
}

void PvtApi::unloadLibrary(const RtToken& name)
{
    RtStagePtr lib = getLibrary(name);
    if (lib)
    {
        // Unregister any nodedefs from this library.
        RtSchemaPredicate<RtNodeDef> nodedefFilter;
        for (RtPrim nodedef : lib->getRootPrim().getChildren(nodedefFilter))
        {
            unregisterMasterPrim(nodedef.getName());
        }

        // Delete the library.
        _libraries.erase(name);
    }
}

RtToken PvtApi::makeUniqueName(const RtToken& name) const
{
    RtToken newName = name;

    // Check if there is another stage with this name.
    RtStagePtr otherStage = getStage(name);
    if (otherStage)
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of("0123456789") + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other stage with the resulting name.
        do {
            newName = baseName + std::to_string(i++);
            otherStage = getStage(newName);
        } while (otherStage);
    }

    return newName;
}


}
