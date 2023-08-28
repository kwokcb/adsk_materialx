//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/JSONIo.h>

#include <MaterialXFormat/External/nlohmann/json.hpp>

#include <MaterialXCore/Types.h>

#include <cstring>
#include <fstream>
#include <sstream>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

const string JSON_EXTENSION = "json";

using json = nlohmann::json;

namespace
{
    const string JSON_MIME_TYPE = "application/mtlx+json";
    const StringSet JSON_NON_ELEMENTS = { "materialx", "mimetype" };
    const string JSON_CATEGORY_NAME_SEPARATOR = ":";
    const StringSet LAYOUT_ATTRIBUES = { "xpos", "ypos" };

    // Writing utility
    void elementToJSON(ConstElementPtr elem, json& jsonObject, const JSONWriteOptions* writeOptions)
    {
        ElementPredicate elementPredicate;
        bool addDefinitionInformation = true;
        bool storeLayoutInformation = true;
        bool addNodeGraphChildren = true;

        if (writeOptions)
        {
            elementPredicate = writeOptions->elementPredicate;
            addDefinitionInformation = writeOptions->addDefinitionInformation;
            storeLayoutInformation = writeOptions->storeLayoutInformation;
            addNodeGraphChildren = writeOptions->addNodeGraphChildren;
        }

        if (elementPredicate && !elementPredicate(elem))
        {
            return;
        }

        // Store attributes in JSON.
        json jsonElem;
        // These are set as the object key: <category>:<name>
        // Instead of embedding them here.
        //
        //if (!elem->getName().empty())
        //{
        //    jsonElem[Element::NAME_ATTRIBUTE] = elem->getName();
        //}
        //if (!elem->getCategory().empty())
        //{
        //    jsonElem["category"] = elem->getCategory();
        //}

        // Add in definition information if it is a node   
        if (addDefinitionInformation)
        {
            ConstNodePtr node = elem->asA<Node>();
            if (node)
            {
                ConstNodeDefPtr nodeDef = node->getNodeDef();
                if (nodeDef)
                {
                    jsonElem[InterfaceElement::NODE_DEF_ATTRIBUTE] = nodeDef->getName();
                    const string& version = nodeDef->getVersionString();
                    if (!version.empty())
                        jsonElem[InterfaceElement::VERSION_ATTRIBUTE] = nodeDef->getVersionString();
                }
            }
        }

        for (const string& attrName : elem->getAttributeNames())
        {
            if (!storeLayoutInformation && LAYOUT_ATTRIBUES.count(attrName))
                continue;

            jsonElem[attrName] = elem->getAttribute(attrName);
        }

        // Create child nodes and recurse.
        bool isGraph = elem->isA<NodeGraph>();
        bool skipGraphChildren = (!addNodeGraphChildren && isGraph);

        for (ConstElementPtr child : elem->getChildren())
        {
            if (skipGraphChildren && child->isA<Node>())
            {
                continue;
            }
            if (elementPredicate && !elementPredicate(child))
            {
                continue;
            }

            elementToJSON(child, jsonElem, writeOptions);
        }

        // Add new element to parent
        jsonObject[elem->getCategory() + JSON_CATEGORY_NAME_SEPARATOR + elem->getName()] = jsonElem;
    }

    // Reading utilities
    void elementFromJSON(const json& node, ElementPtr elem, const JSONReadOptions* readOptions,
                         const string& indent)
    {
        if (node.is_object()) 
        {
            for (const auto& entry : node.items()) 
            {
                const std::string& key = entry.key();
                const json& value = entry.value();

                // Handle attributes
                if (value.is_string())
                {
                    const string& attrName = key;
                    elem->setAttribute(attrName, string(value));
                }
                else if (!JSON_NON_ELEMENTS.count(key))
                {
                    StringVec categoryName = splitString(key, JSON_CATEGORY_NAME_SEPARATOR);
                    const string& category = categoryName[0];
                    const string& name = categoryName[1];

                    // Check for duplicate elements.
                    ConstElementPtr previous = elem->getChild(name);
                    if (!previous)
                    {
                        if (!category.empty())
                        {
                            //std::cout << indent << "Create Element <" << category << "> Name: " << name << std::endl;
                            ElementPtr child = elem->addChildOfCategory(category, name);
                            elementFromJSON(value, child, readOptions, indent + string("  "));
                        }
                    }
                }
            }
        }
    }

    void documentFromJSON(DocumentPtr doc,
        const json& jsonDoc,
        const JSONReadOptions* readOptions = nullptr)
    {
        const string indent = "   ";

        // Go through all top level items
        for (const auto& entry: jsonDoc.items())
        {
            // Handle top level attributes
            const string& key = entry.key();
            const json& value = entry.value();
            if (value.is_string())
            {
                string attrName = key;
                if (!JSON_NON_ELEMENTS.count(attrName))
                {
                    //std::cout << indent << "Set Doc Attribute: " << attrName << " to: " << value << std::endl;
                    doc->setAttribute(attrName, string(value));
                }
            }
            else
            {
                elementFromJSON(value, doc, readOptions, indent);
            }
        }
        elementFromJSON(jsonDoc, doc, readOptions, indent);
        if (!readOptions || readOptions->upgradeVersion)
        {
            doc->upgradeVersion();
        }
    }
}

//
// Reading
//
void readFromJSONString(DocumentPtr doc, const string& buffer, const JSONReadOptions* readOptions)
{
    std::stringstream inputStream;
    inputStream << buffer;

    readFromJSONStream(doc, inputStream, readOptions);
}

void readFromJSONStream(DocumentPtr doc, std::istream& stream, const JSONReadOptions* readOptions)
{
    json jsonDoc;    
    try
    {
        jsonDoc = json::parse(stream);
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    documentFromJSON(doc, jsonDoc, readOptions);
}

void readFromJSONFile(DocumentPtr doc,
    FilePath filename,
    FileSearchPath searchPath,
    const JSONReadOptions* readOptions)
{
    searchPath.append(getEnvironmentPath());
    filename = searchPath.find(filename);

    std::ifstream inputFile;
    inputFile.open(filename.asString());
    if (!inputFile.is_open()) {
        throw std::runtime_error(string("Unable to open JSON file:") + filename.asString());
    }

    readFromJSONStream(doc, inputFile, readOptions);
}

//
// Writing
//

void writeToJSONStream(DocumentPtr doc, std::ostream& stream, const JSONWriteOptions* writeOptions)
{
    json materialXRoot;
    materialXRoot["mimetype"] = "application/mtlx+json";

    json documentRoot = json::object();

    for (const string& attrName : doc->getAttributeNames())
    {
        documentRoot[attrName] = doc->getAttribute(attrName);
    }

    // Children are in an array
    for (ElementPtr elem : doc->getChildren())
    {
        elementToJSON(elem, documentRoot, writeOptions);
    }

    materialXRoot["materialx"] = documentRoot;

    // Set to stream to dump of JSON object.
    const string jsonString = materialXRoot.dump(writeOptions ? writeOptions->indent : 4);
    stream << jsonString << std::endl;
}

void writeToJSONFile(DocumentPtr doc, const FilePath& filename, const JSONWriteOptions* writeOptions)
{
    std::ofstream ofs(filename.asString());
    writeToJSONStream(doc, ofs, writeOptions);
}

string writeToJSONString(DocumentPtr doc, const JSONWriteOptions* writeOptions)
{
    std::ostringstream stream;
    writeToJSONStream(doc, stream, writeOptions);
    return stream.str();
}

MATERIALX_NAMESPACE_END
