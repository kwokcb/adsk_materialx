#ifndef MATERIALX_TEXCOORDGLSL_H
#define MATERIALX_TEXCOORDGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'texcoord' node for GLSL
class TexCoordGlsl : public GlslImplementation
{
public:
    static GenImplementationPtr create();

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
