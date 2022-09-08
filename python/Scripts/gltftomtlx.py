#!/usr/bin/env python
'''
Utility to extract materials from a gltf file and write out a mtlx file
Options include:
  - Writing material assignments
  - Writing out all inputs on the definition for a given node
  - Writing output file name
'''

import sys, os, argparse, subprocess
import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
import MaterialX.PyMaterialXGenGlsl as mx_gen_glsl
import MaterialX.PyMaterialXGenOsl as mx_gen_osl
import MaterialX.PyMaterialXGenMdl as mx_gen_mdl
import MaterialX.PyMaterialXglTF as mx_gltf

def hasNoURI(elem):
    if elem.hasSourceUri():
        return False
    return True        

def main():
    parser = argparse.ArgumentParser(description='Generate shader code for each material / shader in a document.')
    parser.add_argument('--path', dest='paths', action='append', nargs='+', help='An additional absolute search path location (e.g. "/projects/MaterialX")')
    parser.add_argument('--library', dest='libraries', action='append', nargs='+', help='An additional relative path to a custom data library folder (e.g. "libraries/custom")')
    parser.add_argument('--assignments', action='store_true', help='If argument specified, handle material assignments')
    parser.add_argument('--full', action='store_true', help='If argument specified, generate full descriptions for load')
    parser.add_argument('--outputFile', dest='outputFile', help='Name of the output file. If not specified, the original file name with a ".mtlx" extension is written to.')
    parser.add_argument(dest='inputFile', help='Filename of the input document.')
    opts = parser.parse_args()

    stdlib = mx.createDocument()
    searchPath = mx.FileSearchPath(os.path.dirname(opts.inputFile))
    libraryFolders = []
    if opts.paths:
        for pathList in opts.paths:
            for path in pathList:
                searchPath.append(path)
    if opts.libraries:
        for libraryList in opts.libraries:
            for library in libraryList:
                libraryFolders.append(library)
    libraryFolders.append("libraries")
    try:
        mx.loadLibraries(libraryFolders, searchPath, stdlib)
    except err:
        print('Generation failed: "', err, '"')
        sys.exit(-1)

    doc = mx.createDocument()
    materials = None
    try:
        # Create an output converter
        handler = mx_gltf.CgltfMaterialHandler.create()
        handler.setDefinitions(stdlib)

        if opts.assignments:
            handler.setGenerateAssignments(True)
        if opts.full:
            handler.setGenerateFullDefinitions(True)

        print("Converting: " + opts.inputFile)
        if handler.load(opts.inputFile):
            materials = handler.getMaterials()

    except mx.ExceptionFileMissing as err:
        print('Conversion failed: "', err, '"')
        sys.exit(-1)

    if not materials:
        print('Materials could not be converted for input document')
        sys.exit(-1)

    # Perform validation check
    valid, msg = materials.validate()
    if not valid:
        print('Invalid document created.')
        print(msg)
        sys.exit(-1)

    # Write to disk
    outFileName = opts.inputFile + ".mtlx"
    if opts.outputFile:
        outFileName = opts.outputFile
    print("Write materials to: " + outFileName)
    writeOptions = mx.XmlWriteOptions()
    # This does not work currently. 
    #writeOptions.elementPredicate = hasNoURI
    mx.writeToXmlFile(materials, outFileName, writeOptions)

    #mx.writeToXmlFile(stdlib, 'defs.mtlx')        

if __name__ == '__main__':
    main()
