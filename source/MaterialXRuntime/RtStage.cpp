//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

namespace
{
    // Syntactic sugar
    inline PvtStage* _cast(void* ptr)
    {
        return static_cast<PvtStage*>(ptr);
    }
}

RtStage::RtStage() :
    _ptr(nullptr)
{
}

RtStage::~RtStage()
{
    delete _cast(_ptr);
}

RtStagePtr RtStage::createNew(const RtToken& name)
{
    // Create the shared stage object.
    RtStagePtr stage(new RtStage());

    // Create the private stage implementation.
    stage->_ptr = new PvtStage(name, RtStageWeakPtr(stage));

    // Return the shared wrapper object.
    return stage;
}

const RtToken& RtStage::getName() const
{
    return _cast(_ptr)->getName();
}

void RtStage::addSourceUri(const RtToken& uri)
{
    return _cast(_ptr)->addSourceUri(uri);
}

const RtTokenVec& RtStage::getSourceUri() const
{
    return _cast(_ptr)->getSourceUri();
}

RtPrim RtStage::createPrim(const RtToken& typeName)
{
    return createPrim(RtPath("/"), EMPTY_TOKEN, typeName);
}

RtPrim RtStage::createPrim(const RtPath& path, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(path._ptr), typeName);
    return prim->hnd();
}

RtPrim RtStage::createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(parentPath._ptr), name, typeName);
    return prim->hnd();
}

void RtStage::removePrim(const RtPath& path)
{
    _cast(_ptr)->removePrim(*static_cast<PvtPath*>(path._ptr));
}

RtToken RtStage::renamePrim(const RtPath& path, const RtToken& newName)
{
    return _cast(_ptr)->renamePrim(*static_cast<PvtPath*>(path._ptr), newName);
}

RtToken RtStage::reparentPrim(const RtPath& path, const RtPath& newParentPath)
{
    return _cast(_ptr)->reparentPrim(
        *static_cast<PvtPath*>(path._ptr),
        *static_cast<PvtPath*>(newParentPath._ptr)
    );
}

RtPrim RtStage::getPrimAtPath(const RtPath& path)
{
    PvtPrim* prim = _cast(_ptr)->getPrimAtPath(*static_cast<PvtPath*>(path._ptr));
    return prim ? prim->hnd() : RtPrim();
}

RtPrim RtStage::getRootPrim()
{
    return _cast(_ptr)->getRootPrim()->hnd();
}

RtStageIterator RtStage::traverse(RtObjectPredicate predicate)
{
    return RtStageIterator(shared_from_this(), predicate);
}

void RtStage::addReference(RtStagePtr stage)
{
    _cast(_ptr)->addReference(stage);
}

RtStagePtr RtStage::getReference(const RtToken& name) const
{
    return _cast(_ptr)->getReference(name);
}

void RtStage::removeReference(const RtToken& name)
{
    _cast(_ptr)->removeReference(name);
}

void RtStage::removeReferences()
{
    _cast(_ptr)->removeReferences();
}

void RtStage::setName(const RtToken& name)
{
    _cast(_ptr)->setName(name);
}

void RtStage::disposePrim(const RtPath& path)
{
    _cast(_ptr)->disposePrim(*static_cast<PvtPath*>(path._ptr));
}

void RtStage::restorePrim(const RtPath& parentPath, const RtPrim& prim)
{
    _cast(_ptr)->restorePrim(*static_cast<PvtPath*>(parentPath._ptr), prim);
}

RtPrim RtStage::getImplementation(const RtNodeDef& definition) const
{
    const RtToken& nodeDefName = definition.getName();

    RtSchemaPredicate<RtNodeGraph> filter;
    for (RtPrim child : _cast(_ptr)->getRootPrim()->getChildren(filter))
    {
        RtNodeGraph nodeGraph(child);
        // Check if there is a definition name match 
        if (nodeGraph.getDefinition() == nodeDefName)
        {
            PvtPrim* graphPrim = PvtObject::ptr<PvtPrim>(child);
            return RtPrim(graphPrim->hnd());
        }
    }

    // TODO: Return an empty prim for now. When support is added in to be able to
    // access non-nodegraph implementations, this method should throw an exception if not found.
    return RtPrim();
}

RtPrim RtStage::createNodeDef(RtNodeGraph& nodeGraph, 
                              const RtToken& nodeDefName, 
                              const RtToken& nodeName, 
                              const RtToken& version,
                              bool isDefaultVersion,
                              const RtToken& nodeGroup)
{
    // Must have a nodedef name and a node name
    if (nodeDefName == EMPTY_TOKEN || nodeName == EMPTY_TOKEN)
    {
        throw ExceptionRuntimeError("Cannot create nodedef '" + nodeDefName.str() + "', with node name: '" + nodeName.str() + "'");
    }

    // Make sure a nodedef with this name in not already registered.
    if (RtApi::get().hasNodeDef(nodeDefName))
    {
        throw ExceptionRuntimeError("A nodedef named '" + nodeDefName.str() + "' is already registered");
    }

    PvtStage* stage = _cast(_ptr);

    // Make sure the nodedef name is unique among all prims.
    PvtPath path(PvtPath::ROOT_NAME.str());
    path.push(nodeDefName);
    if (stage->getPrimAtPath(path))
    {
        throw ExceptionRuntimeError("The nodedef named '" + nodeDefName.str() + "' is not unique");
    }

    PvtPrim* prim = stage->createPrim(stage->getPath(), nodeDefName, RtNodeDef::typeName());
    RtNodeDef nodedef(prim->hnd());

    // Set node, version and optional node group
    nodedef.setNode(nodeName);
    if (version != EMPTY_TOKEN)
    {
        nodedef.setVersion(version);

        // If a version is specified, set if it is the default version
        if (isDefaultVersion)
        {
            nodedef.setIsDefaultVersion(true);
        }
    }
    if (nodeGroup != EMPTY_TOKEN)
    {
        nodedef.setNodeGroup(nodeGroup);
    }

    // Add an input per nodegraph input
    for (RtInput input : nodeGraph.getInputs())
    {
        RtInput nodedefInput = nodedef.createInput(input.getName(), input.getType());
        nodedefInput.setUniform(input.isUniform());
        RtValue::copy(input.getType(), input.getValue(), nodedefInput.getValue());
    }

    // Add an output per nodegraph output
    for (RtOutput output : nodeGraph.getOutputs())
    {
        RtOutput nodedefOutput = nodedef.createOutput(output.getName(), output.getType());
        RtValue::copy(output.getType(), output.getValue(), nodedefOutput.getValue());
    }

    // Set the definition on the nodegraph
    nodeGraph.setDefinition(nodeDefName);

    // Register this nodedef.
    RtApi::get().registerNodeDef(nodedef.getPrim());

    return nodedef.getPrim();
}

}
