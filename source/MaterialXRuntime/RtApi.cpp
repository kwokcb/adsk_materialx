//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/Codegen/RtSubGraphImpl.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // Syntactic sugar
    inline PvtApi* _cast(void* ptr)
    {
        return static_cast<PvtApi*>(ptr);
    }
}

RtApi::RtApi() :
    _ptr(new PvtApi())
{
}

RtApi::~RtApi()
{
    delete _cast(_ptr);
}

void RtApi::initialize()
{
    _cast(_ptr)->reset();

    // Register built in schemas
    registerTypedSchema<RtGeneric>();
    registerTypedSchema<RtNode>();
    registerTypedSchema<RtNodeDef>();
    registerTypedSchema<RtNodeGraph>();
    registerTypedSchema<RtNodeImpl>();
    registerTypedSchema<RtTargetDef>();
    registerTypedSchema<RtSourceCodeImpl>();
    registerTypedSchema<RtSubGraphImpl>();
    registerTypedSchema<RtBackdrop>();
    registerTypedSchema<RtBindElement>();
    registerTypedSchema<RtLookGroup, RtLookGroupConnectableApi>();
    registerTypedSchema<RtLook, RtLookConnectableApi>();
    registerTypedSchema<RtMaterialAssign, RtMaterialAssignConnectableApi>();
    registerTypedSchema<RtCollection, RtCollectionConnectableApi>();
}

void RtApi::shutdown()
{
    _cast(_ptr)->reset();

    // Unregister built in schemas
    unregisterTypedSchema<RtGeneric>();
    unregisterTypedSchema<RtNode>();
    unregisterTypedSchema<RtNodeDef>();
    unregisterTypedSchema<RtNodeGraph>();
    unregisterTypedSchema<RtNodeImpl>();
    unregisterTypedSchema<RtTargetDef>();
    unregisterTypedSchema<RtSourceCodeImpl>();
    unregisterTypedSchema<RtSubGraphImpl>();
    unregisterTypedSchema<RtBackdrop>();
    unregisterTypedSchema<RtBindElement>();
    unregisterTypedSchema<RtLookGroup>();
    unregisterTypedSchema<RtLook>();
    unregisterTypedSchema<RtMaterialAssign>();
    unregisterTypedSchema<RtCollection>();
}

void RtApi::registerLogger(RtLoggerPtr logger)
{
    _cast(_ptr)->registerLogger(logger);
}

void RtApi::unregisterLogger(RtLoggerPtr logger)
{
    _cast(_ptr)->unregisterLogger(logger);
}

void RtApi::log(RtLogger::MessageType type, const string& msg)
{
    _cast(_ptr)->log(type, msg);
}

void RtApi::registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func)
{
    _cast(_ptr)->registerCreateFunction(typeName, func);
}

void RtApi::unregisterCreateFunction(const RtToken& typeName)
{
    _cast(_ptr)->unregisterCreateFunction(typeName);
}

bool RtApi::hasCreateFunction(const RtToken& typeName) const
{
    return _cast(_ptr)->hasCreateFunction(typeName);
}

RtPrimCreateFunc RtApi::getCreateFunction(const RtToken& typeName) const
{
    return _cast(_ptr)->getCreateFunction(typeName);
}

void RtApi::registerNodeDef(const RtPrim& prim)
{
    _cast(_ptr)->registerNodeDef(prim);
}

void RtApi::unregisterNodeDef(const RtToken& name)
{
    _cast(_ptr)->unregisterNodeDef(name);
}

bool RtApi::hasNodeDef(const RtToken& name) const
{
    return _cast(_ptr)->hasNodeDef(name);
}

size_t RtApi::numNodeDefs() const
{
    return _cast(_ptr)->numNodeDefs();
}

RtPrim RtApi::getNodeDef(size_t index) const
{
    return _cast(_ptr)->getNodeDef(index);
}

RtPrim RtApi::getNodeDef(const RtToken& name) const
{
    return _cast(_ptr)->getNodeDef(name);
}

void RtApi::registerNodeImpl(const RtPrim& prim)
{
    _cast(_ptr)->registerNodeImpl(prim);
}

void RtApi::unregisterNodeImpl(const RtToken& name)
{
    _cast(_ptr)->unregisterNodeImpl(name);
}

bool RtApi::hasNodeImpl(const RtToken& name) const
{
    return _cast(_ptr)->hasNodeImpl(name);
}

size_t RtApi::numNodeImpls() const
{
    return _cast(_ptr)->numNodeImpls();
}

RtPrim RtApi::getNodeImpl(size_t index) const
{
    return _cast(_ptr)->getNodeImpl(index);
}

RtPrim RtApi::getNodeImpl(const RtToken& name) const
{
    return _cast(_ptr)->getNodeImpl(name);
}

void RtApi::registerTargetDef(const RtPrim& prim)
{
    _cast(_ptr)->registerTargetDef(prim);
}

void RtApi::unregisterTargetDef(const RtToken& name)
{
    _cast(_ptr)->unregisterTargetDef(name);
}

bool RtApi::hasTargetDef(const RtToken& name) const
{
    return _cast(_ptr)->hasTargetDef(name);
}

void RtApi::clearSearchPath()
{
    _cast(_ptr)->clearSearchPath();
}

void RtApi::clearTextureSearchPath()
{
    _cast(_ptr)->clearTextureSearchPath();
}

void RtApi::clearImplementationSearchPath()
{
    _cast(_ptr)->clearImplementationSearchPath();
}

void RtApi::setSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setSearchPath(searchPath);
}

void RtApi::setTextureSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setTextureSearchPath(searchPath);
}

void RtApi::setImplementationSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setImplementationSearchPath(searchPath);
}

const FileSearchPath& RtApi::getSearchPath() const
{
    return _cast(_ptr)->getSearchPath();
}

const FileSearchPath& RtApi::getTextureSearchPath() const
{
    return _cast(_ptr)->getTextureSearchPath();
}

const FileSearchPath& RtApi::getImplementationSearchPath() const
{
    return _cast(_ptr)->getImplementationSearchPath();
}

void RtApi::createLibrary(const RtToken& name)
{
    _cast(_ptr)->createLibrary(name);
}

void RtApi::loadLibrary(const RtToken& name, const RtReadOptions& options)
{
    _cast(_ptr)->loadLibrary(name, options);
}

void RtApi::unloadLibrary(const RtToken& name)
{
    _cast(_ptr)->unloadLibrary(name);
}

RtTokenVec RtApi::getLibraryNames() const
{
    return _cast(_ptr)->getLibraryNames();
}

const FilePath& RtApi::getUserDefinitionPath() const
{
    return _cast(_ptr)->getUserDefinitionPath();
}

void RtApi::setUserDefinitionPath(const FilePath& path)
{
    return _cast(_ptr)->setUserDefinitionPath(path);
}

RtStagePtr RtApi::getLibrary(const RtToken& name)
{
    return _cast(_ptr)->getLibrary(name);
}

RtStagePtr RtApi::getLibrary()
{
    return _cast(_ptr)->getLibraryRoot();
}

RtStagePtr RtApi::createStage(const RtToken& name)
{
    return _cast(_ptr)->createStage(name);
}

void RtApi::deleteStage(const RtToken& name)
{
    _cast(_ptr)->deleteStage(name);
}

RtStagePtr RtApi::getStage(const RtToken& name) const
{
    return _cast(_ptr)->getStage(name);
}

RtToken RtApi::renameStage(const RtToken& name, const RtToken& newName)
{
    return _cast(_ptr)->renameStage(name, newName);
}

RtTokenVec RtApi::getStageNames() const
{
    return _cast(_ptr)->getStageNames();
}

UnitConverterRegistryPtr RtApi::getUnitDefinitions()
{
    return _cast(_ptr)->getUnitDefinitions();
}

RtApi& RtApi::get()
{
    static RtApi _instance;
    return _instance;
}

}
