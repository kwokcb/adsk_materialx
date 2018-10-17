#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXGenShader/GenImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public GenImplementation
{
public:
    static GenImplementationPtr create();

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
