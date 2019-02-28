#ifndef MATERIALX_COMPARENODE_H
#define MATERIALX_COMPARENODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of compare node
class CompareNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
