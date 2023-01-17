#!/usr/bin/env python
'''
Create a document which contains a material root which samples
an upstream instance  for each node definition found.
'''

import sys, os, argparse
import MaterialX as mx


# Given a node, add downstream material attachments
def addMaterialGraphs(node, doc, outdoc, nodedef, addExplicitOutputs):

    # Use more cumbersome method of grabbing from nodedef if not using
    # explicit outputs on node
    outputs = []
    if not addExplicitOutputs:
        outputs = nodedef.getActiveOutputs()
    else:
        outputs = node.getActiveOutputs()
    isMultiOutput = len(outputs) > 1

    for output in outputs:
        outputName = output.getName()
        outputType = output.getType()
        #outputElem = node.getOutput(outputName)
        #if (not outputElem):
        #    outputElem = node.addOutput(outputName, outputType)

        shaderNodeName = outdoc.createValidChildName('shader_' + node.getName() + '_' + outputName)                
        materialNodeName = outdoc.createValidChildName('material_' + node.getName() + '_' + outputName)                

        # EDF and BSDF can feed into surface shader
        if outputType in { 'EDF', 'BSDF' }:
            shaderNode = outdoc.addNode("surface", shaderNodeName, "surfaceshader")
            newInput = shaderNode.addInput(outputType.lower(), outputType)
            newInput.setNodeName(node.getName())
            if isMultiOutput:
                newInput.setAttribute('output', outputName)
            materialNode = outdoc.addMaterialNode(materialNodeName, shaderNode)

        # Shader can feed directly into a material
        elif outputType in { 'surfaceshader', 'volumeshader', 'displacementshader' }: 
            materialNode = outdoc.addMaterialNode(materialNodeName, node)

        # Other numeric and boolean can feed into utility "convert" nodes 
        elif outputType in { 'float', 'vector2', 'vector3', 'vector4', 'integer', 'boolean', 'color3', 'color4' }:

            convertDefinition = 'ND_convert_' + outputType + '_material'
            convertNode = doc.getNodeDef(convertDefinition)
            if not convertNode:
                print("> Failed to find conversion definition: %s" % convertDefinition)
            else:
                materialNode = outdoc.addNodeInstance(convertNode, materialNodeName)
                materialNode.removeAttribute('nodedef')
                newInput = materialNode.addInput('in', outputType)
                newInput.setNodeName(node.getName())                        
                if isMultiOutput:
                    newInput.setAttribute('output', outputName)    

# Create a node instance given a node definition with appropriate inputs and outputs
def createNodeInstance(nodedef, nodeName, outdoc, setEmptyValues, addExplicitOutputs):
    node = outdoc.addNodeInstance(nodedef, nodeName)
    node.removeAttribute('nodedef')
    version = nodedef.getVersionString()
    if len(version) > 0:
        node.setVersionString(version)

    nodeType = nodedef.getNodeString()
    isGeomProp = nodeType == 'geompropvalue' 
    isUsdPrimvarReader = nodeType == 'UsdPrimvarReader'

    for input in nodedef.getActiveInputs():
        inputName = input.getName()
        inputType = input.getType()
        valueElem = node.getInput(inputName)
        if (not valueElem):
            newElem = node.addInput(inputName, inputType)
            newElem.copyContentFrom(input)
            if not newElem.getValue():

                # Set input values here as default definition does not define these
                # values. This avoids error in node and code generation validation.
                # Note: For geometry routing, this is using stream / primvar names
                # generated for render tests internally (addAdditionalTestStreams())
                if setEmptyValues:
                    if isGeomProp and inputName == 'geomprop':
                        newElem.setValue('geompropvalue_' + nodedef.getType())
                    elif isUsdPrimvarReader and inputName == 'varname':
                        newElem.setValue('geompropvalue_' + nodedef.getType())
                    elif inputType == 'BSDF':
                        # Creat an arbitrary input node 
                        bsdfNodeName = outdoc.createValidChildName('oren_nayar_diffuse_bsdf')
                        bsdfNode = outdoc.addNode('oren_nayar_diffuse_bsdf', bsdfNodeName, inputType)
                        newElem.setNodeName(bsdfNode.getName())
                    elif inputType == "EDF":
                        # Create an input uniform node
                        edfNodeName = outdoc.createValidChildName('uniform_edf')
                        edfNode = outdoc.addNode('uniform_edf', edfNodeName, inputType)
                        newElem.setNodeName(edfNode.getName())
                    elif inputType == 'surfaceshader':
                        ssNodeName = outdoc.createValidChildName('standard_surface')
                        ssNode = outdoc.addNode('standard_surface', ssNodeName, inputType)
                        newElem.setNodeName(ssNode.getName())
                        

                    #elif inputType == 'displacementshader':
                    #elif inputType == 'volumeshader':



            for attr in [ 'doc', 'uimin', 'uimax', 'uifolder', 'uisoftmin', 'uisoftmax', 'uiadvanced' ]:
                newElem.removeAttribute(attr)

    if addExplicitOutputs:
        print('add explicit outputs ?', addExplicitOutputs)
        for output in nodedef.getActiveOutputs():
            outputName = output.getName()
            outputType = output.getType()
            outputElem = node.getOutput(outputName)
            if (not outputElem):
                outputElem = node.addOutput(outputName, outputType)
    
    return node

# Given a node definition create a material node graph in a new document
def createMaterialFromNodedef(nodedef, doc, outdoc, addExplicitOutputs):

    nodeName = nodedef.getName()
    functionName = nodeName.removeprefix('ND_')
    functionName = outdoc.createValidChildName(functionName)

    node = createNodeInstance(nodedef, functionName, outdoc, True, addExplicitOutputs)

    addMaterialGraphs(node, doc, outdoc, nodedef, addExplicitOutputs)

    return node

# Print the document for node definitions in a file
def createMaterials(doc, opts):

    # thin_film_bsdf code generation produces undefined variable names for OSL and GLSL
    ignoreNodeList = [ "thin_film_bsdf", "surfacematerial", "volumematerial", "arrayappend", "dot_filename" ]
    ignoreTypeList = [ "lightshader" ]
    ignoreNodeDefList = [ "ND_convert_BSDF_material", "ND_convert_EDF_material" ]

    nodedefs = doc.getNodeDefs()
    nodedefCount = str(len(nodedefs))
    if nodedefCount == 0:
        print('No definitions to create materials for')

    count = 0
    ignoreList = []
    for nodedef in nodedefs:

        nodeinterface = nodedef.getImplementation(opts.target)
        if not nodeinterface: 
            continue

        sourceUri = nodedef.getSourceUri()
        if opts.libName and sourceUri.find(opts.libName) == -1:
            continue            

        skip = False
        for i in ignoreNodeList:
            if nodedef.getNodeString() == i:
                skip = True
                continue
        for i in ignoreTypeList:
            if nodedef.getType() == i:
                skip = True
                continue
        for i in ignoreNodeDefList:
            if nodedef.getName() == i:
                skip = True
                continue

        if skip:
            ignoreList.append( nodedef.getName() )
            continue

        outdoc = mx.createDocument()

        node = createMaterialFromNodedef(nodedef, doc, outdoc, opts.addExplicitOutputs)

        filename = opts.outputPath + '/' + node.getName() + ".mtlx"
        #print("Write defintion file: %s" % filename)
        mx.writeToXmlFile(outdoc, filename)

        count = count + 1

    print('Create materials for %d definitions' % count)
    print('SKipped nodedefs: ', ignoreList)


def main():
    parser = argparse.ArgumentParser(description="Create a Materialx document for each output on an instance per nodedef. This instance is sampled by a downstream material node.")
    parser.add_argument(dest="libraryPath", help="Path for MaterialX libraries.")
    parser.add_argument('--libName', dest='libName', help='Name of library to generate for. Does a match against the filename')
    parser.add_argument('--outputPath', dest='outputPath', help='File path to output material files to.')
    parser.add_argument('--target', dest='target', default='genglsl', help='Shading language target. Default is genglsl')
    parser.add_argument('--addExplicitOutputs', dest='addExplicitOutputs', default=False, type=bool, help='Add explicit outputs to node instances created from nodedefs')

    opts = parser.parse_args()

    # Read library
    rootPath = opts.libraryPath;
    stdlib = mx.createDocument()
    searchPath = rootPath
    mx.loadLibraries([ rootPath ], searchPath, stdlib)
    doc = mx.createDocument()
    doc.importLibrary(stdlib)

    # Create output directory
    if opts.outputPath:
        if not os.path.exists(opts.outputPath):
            os.makedirs(opts.outputPath)

    # Create material files
    createMaterials(doc, opts) 

if __name__ == '__main__':
    main()
