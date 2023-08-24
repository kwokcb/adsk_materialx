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

    void elementToJSON(ConstElementPtr elem, json& jsonObject, const JSONWriteOptions* writeOptions)
    {
        ElementPredicate elementPredicate = nullptr;
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
        if (!elem->getName().empty())
        {
            jsonElem["@" + Element::NAME_ATTRIBUTE] = elem->getName();
        }
        if (!elem->getCategory().empty())
        {
            jsonElem["@category"] = elem->getCategory();
        }

        // Add in definition information if it is a node   
        if (addDefinitionInformation)
        {
            ConstNodePtr node = elem->asA<Node>();
            if (node)
            {
                ConstNodeDefPtr nodeDef = node->getNodeDef();
                if (nodeDef)
                {
                    jsonElem["@nodedef"] = nodeDef->getName();
                    const string& version = nodeDef->getVersionString();
                    if (!version.empty())
                        jsonElem["@version"] = nodeDef->getVersionString();
                }
            }
        }

        StringSet layoutAttribues = { "xpos", "ypos" };
    for (const string& attrName : elem->getAttributeNames())
    {
        if (!storeLayoutInformation && layoutAttribues.count(attrName))
            continue;

        jsonElem["@" + attrName] = elem->getAttribute(attrName);
    }

    // Create child nodes and recurse.
    bool isGraph = elem->isA<NodeGraph>();
    bool skipGraphChildren = (!addNodeGraphChildren && isGraph);

    for (auto child : elem->getChildren())
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
    jsonObject[elem->getName()] = jsonElem;
}
}

//
// Writing
//

void writeToJSONStream(DocumentPtr doc, std::ostream& stream, const JSONWriteOptions* writeOptions)
{
    json materialXRoot;
    json documentRoot = json::object();

    for (const string& attrName : doc->getAttributeNames())
    {
        documentRoot["@" + attrName] = doc->getAttribute(attrName);
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
