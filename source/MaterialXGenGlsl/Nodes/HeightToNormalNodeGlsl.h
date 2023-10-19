//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HEIGHTTONORMALNODEGLSL_H
#define MATERIALX_HEIGHTTONORMALNODEGLSL_H

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

MATERIALX_NAMESPACE_BEGIN

/// HeightToNormal node implementation for GLSL
class MX_GENGLSL_API HeightToNormalNodeGlsl : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode&, GenContext&, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getTarget() const override;

  protected:
    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;
};

MATERIALX_NAMESPACE_END

#endif
