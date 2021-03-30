//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEDEF_H
#define MATERIALX_RTNODEDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @struct RtNodeLayout
/// Container for node layout information.
struct RtNodeLayout
{
    RtIdentifierVec order;
    RtIdentifierMap<string> uifolder;
};

/// @class RtNodeDef
/// Schema for nodedef prims.
class RtNodeDef : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNodeDef)

public:
    /// Constructor.
    RtNodeDef(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Set the node for this nodedef.
    void setNode(const RtIdentifier& node);

    /// Return the node for this nodedef.
    const RtIdentifier& getNode() const;

    /// Return the namespaced node for this nodedef.
    RtIdentifier getNamespacedNode() const;

    /// Set the nodegroup for this nodedef.
    void setNodeGroup(const RtIdentifier& nodegroup);

    /// Return the node group for this nodedef.
    const RtIdentifier& getNodeGroup() const;

    /// Set the target for this nodedef.
    void setTarget(const RtIdentifier& nodegroup);

    /// Return the target for this nodedef.
    const RtIdentifier& getTarget() const;

    /// Set the inheritance for this nodedef.
    void setIneritance(const RtIdentifier& inherit);

    /// Return the inheritance for this nodedef.
    const RtIdentifier& getIneritance() const;

    /// Set the version for this nodedef.
    void setVersion(const RtIdentifier& version);

    /// Return the version for this nodedef.
    const RtIdentifier& getVersion() const;

    /// Is the version for this definition compatible with the version passed in
    bool isVersionCompatible(const RtIdentifier& version) const;

    /// Set if this nodedef is the default version.
    void setIsDefaultVersion(bool isDefault);

    /// Return if this nodedef is the default version.
    bool getIsDefaultVersion() const;

    /// Set the namespace for this nodedef.
    void setNamespace(const RtIdentifier& space);

    /// Return the namespace for this nodedef.
    const RtIdentifier& getNamespace() const;

    /// Add an input port to the interface.
    /// Shorthand for calling getPrim().createInput().
    RtInput createInput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags = 0)
    {
        return getPrim().createInput(name, type, flags);
    }

    /// Remove an input port from the interface.
    /// Shorthand for calling getPrim().removeInput().
    void removeInput(const RtIdentifier& name)
    {
        return getPrim().removeInput(name);
    }

    /// Return the number of inputs on the node.
    /// Shorthand for calling getPrim().numInputs().
    size_t numInputs() const
    {
        return getPrim().numInputs();
    }

    /// Return an input by index.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(size_t index) const
    {
        return getPrim().getInput(index);
    }

    /// Return an input by name.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(const RtIdentifier& name) const
    {
        return getPrim().getInput(name);
    }

    /// Return an iterator over all inputs.
    /// Shorthand for calling getPrim().getInputs().
    RtInputIterator getInputs() const
    {
        return getPrim().getInputs();
    }

    /// Add an output port to the interface.
    /// Shorthand for calling getPrim().numInputs().
    RtOutput createOutput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags = 0)
    {
        return getPrim().createOutput(name, type, flags);
    }

    /// Remove an output port from the interface.
    /// Shorthand for calling getPrim().removeOutput().
    void removeOutput(const RtIdentifier& name)
    {
        return getPrim().removeOutput(name);
    }

    /// Return the number of outputs on the node.
    /// Shorthand for calling getPrim().numOutputs().
    size_t numOutputs() const
    {
        return getPrim().numOutputs();
    }

    /// Return an output by index.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(size_t index = 0) const
    {
        return getPrim().getOutput(index);
    }

    /// Return an output by name.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(const RtIdentifier& name) const
    {
        return getPrim().getOutput(name);
    }

    /// Return an iterator over all outputs.
    /// Shorthand for calling getPrim().getOutputs().
    RtOutputIterator getOutputs() const
    {
        return getPrim().getOutputs();
    }

    /// Return the relationship maintaining all node implementations registered for this nodedef.
    RtRelationship getNodeImpls() const;

    /// Return the node implementation prim for this nodedef matching the given target.
    /// If no such implementation can be found a null prim is returned.
    RtPrim getNodeImpl(const RtIdentifier& target) const;

    /// Return a node layout struct for this nodedef.
    /// Containing its input ordering and uifolder hierarchy.
    RtNodeLayout getNodeLayout();
};

}

#endif
