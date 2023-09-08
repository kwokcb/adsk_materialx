//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/JsonIo.h>

#include <MaterialXFormat/External/nlohmann/json.hpp>

#include <MaterialXCore/Types.h>

#include <cstring>
#include <fstream>
#include <sstream>

MATERIALX_NAMESPACE_BEGIN

const string JSON_EXTENSION = "json";

using json = nlohmann::json;

const string JSON_MIME_TYPE = "application/mtlx+json";
const string JSON_CATEGORY_NAME_SEPARATOR = ":";

namespace
{
    const string JSON_MIME_TYPE_KEY = "mimetype";
    const StringSet JSON_NON_ELEMENTS = { "materialx", "mimetype" };

    // Writing utility
    void elementToJson(ConstElementPtr elem, json& jsonObject, const JsonWriteOptions* writeOptions)
    {
        ElementPredicate elementPredicate = nullptr;
        if (writeOptions)
        {
            elementPredicate = writeOptions->elementPredicate;
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
        for (const string& attrName : elem->getAttributeNames())
        {
            jsonElem[attrName] = elem->getAttribute(attrName);
        }

        // Create child nodes and recurse.
        for (ConstElementPtr child : elem->getChildren())
        {
            if (elementPredicate && !elementPredicate(child))
            {
                continue;
            }
            elementToJson(child, jsonElem, writeOptions);
        }

        // Add new element to parent
        jsonObject[elem->getCategory() + JSON_CATEGORY_NAME_SEPARATOR + elem->getName()] = jsonElem;
    }

    // Reading utilities
    void elementFromJson(const json& node, ElementPtr elem, const JsonReadOptions* readOptions,
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
                else if (value.is_object())
                {
                    if (!JSON_NON_ELEMENTS.count(key))
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
                                ElementPtr child = elem->addChildOfCategory(category, name);
                                elementFromJson(value, child, readOptions, indent + string("  "));
                            }
                        }
                    }
                }
                else
                {
                    throw(Exception("JSON parsing error: Invalid value type found."));
                }
            }
        }
    }

    void documentFromJson(DocumentPtr doc,
        const json& jsonDoc,
        const JsonReadOptions* readOptions = nullptr)
    {
        const string indent = "   ";

        // Check for correct mimetype first
        bool correctMimeType = false;
        string mimeTypeFound = EMPTY_STRING;
        for (const auto& entry : jsonDoc.items())
        {
            const string& key = entry.key();
            const json& value = entry.value();
            if (value.is_string())
            {
                if (key == JSON_MIME_TYPE_KEY)
                {
                    mimeTypeFound = string(value);
                    if (mimeTypeFound == JSON_MIME_TYPE)
                    {
                        correctMimeType = true;
                    }
                }
            }
        }
        if (!correctMimeType)
        {
            throw(Exception("JSON parsing error: Invalid mimetype: '" + mimeTypeFound + "'"));
        }

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
                    doc->setAttribute(attrName, string(value));
                }
            }
            else if (value.is_object())
            {
                elementFromJson(value, doc, readOptions, indent);
            }
            else
            {
                throw(Exception("JSON parsing error: Invalid value type found."));
            }
        }
        elementFromJson(jsonDoc, doc, readOptions, indent);
        if (!readOptions || readOptions->upgradeVersion)
        {
            doc->upgradeVersion();
        }
    }
}

//
// Reading
//
void readFromJsonString(DocumentPtr doc, const string& buffer, const JsonReadOptions* readOptions)
{
    std::stringstream inputStream;
    inputStream << buffer;

    readFromJsonStream(doc, inputStream, readOptions);
}

void readFromJsonStream(DocumentPtr doc, std::istream& stream, const JsonReadOptions* readOptions)
{
    json jsonDoc;    
    try
    {
        jsonDoc = json::parse(stream);
    }
    catch (const json::parse_error& e) 
    {
        throw(Exception("JSON parsing error: " + string(e.what())));
    }

    documentFromJson(doc, jsonDoc, readOptions);
}

void readFromJsonFile(DocumentPtr doc,
    FilePath filename,
    FileSearchPath searchPath,
    const JsonReadOptions* readOptions)
{
    searchPath.append(getEnvironmentPath());
    filename = searchPath.find(filename);

    std::ifstream inputFile;
    inputFile.open(filename.asString());
    if (!inputFile.is_open()) 
    {
        throw(Exception(string("Unable to open JSON file:") + filename.asString()));
    }
    readFromJsonStream(doc, inputFile, readOptions);
    inputFile.close();
}

//
// Writing
//

void writeToJsonStream(DocumentPtr doc, std::ostream& stream, const JsonWriteOptions* writeOptions)
{
    json materialXRoot;
    materialXRoot[JSON_MIME_TYPE_KEY] = JSON_MIME_TYPE;

    json documentRoot = json::object();

    for (const string& attrName : doc->getAttributeNames())
    {
        documentRoot[attrName] = doc->getAttribute(attrName);
    }

    // Children are in an array
    for (ElementPtr elem : doc->getChildren())
    {
        elementToJson(elem, documentRoot, writeOptions);
    }

    materialXRoot["materialx"] = documentRoot;

    // Set to stream to dump of JSON object.
    JsonWriteOptions defaultOptions;
    const string jsonString = writeOptions ?
        materialXRoot.dump(writeOptions->indent, writeOptions->indentCharacter, true) :
        materialXRoot.dump(defaultOptions.indent, defaultOptions.indentCharacter, true);
    stream << jsonString << std::endl;
}

void writeToJsonFile(DocumentPtr doc, const FilePath& filename, const JsonWriteOptions* writeOptions)
{
    std::ofstream ofs(filename.asString());
    writeToJsonStream(doc, ofs, writeOptions);
}

string writeToJsonString(DocumentPtr doc, const JsonWriteOptions* writeOptions)
{
    std::ostringstream stream;
    writeToJsonStream(doc, stream, writeOptions);
    return stream.str();
}

MATERIALX_NAMESPACE_END
