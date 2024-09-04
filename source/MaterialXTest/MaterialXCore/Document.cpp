//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <map>
#include <iostream>

namespace mx = MaterialX;

TEST_CASE("Document", "[document]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Test version strings.
    REQUIRE(mx::stringStartsWith(mx::getVersionString(), doc->getVersionString()));

    // Test version integers.
    REQUIRE(doc->getVersionIntegers().first == std::get<0>(mx::getVersionIntegers()));
    REQUIRE(doc->getVersionIntegers().second == std::get<1>(mx::getVersionIntegers()));

    // Create a node graph with a constant color output.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setInputValue("value", mx::Color3(0.5f));
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    REQUIRE(output->isColorType());
    REQUIRE(doc->validate());

    // Create and test a type mismatch in a connection.
    output->setType("float");
    REQUIRE(!doc->validate());
    output->setType("color3");
    REQUIRE(doc->validate());

    // Test hierarchical name paths.
    REQUIRE(constant->getNamePath() == "nodegraph1/node1");
    REQUIRE(constant->getNamePath(nodeGraph) == "node1");

    // Test getting elements by path
    REQUIRE(doc->getDescendant("") == doc);
    REQUIRE(doc->getDescendant("nodegraph1") == nodeGraph);
    REQUIRE(doc->getDescendant("nodegraph1/node1") == constant);
    REQUIRE(doc->getDescendant("missingElement") == mx::ElementPtr());
    REQUIRE(doc->getDescendant("nodegraph1/missingNode") == mx::ElementPtr());
    REQUIRE(nodeGraph->getDescendant("") == nodeGraph);
    REQUIRE(nodeGraph->getDescendant("node1") == constant);
    REQUIRE(nodeGraph->getDescendant("missingNode") == mx::ElementPtr());

    // Create a simple shader interface.
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("", mx::SURFACE_SHADER_TYPE_STRING, "simpleSrf");
    simpleSrf->setInputValue("diffColor", mx::Color3(1.0f));
    simpleSrf->setInputValue("specColor", mx::Color3(0.0f));
    mx::InputPtr roughness = simpleSrf->setInputValue("roughness", 0.25f);
    REQUIRE(!roughness->getIsUniform());
    roughness->setIsUniform(true);
    REQUIRE(roughness->getIsUniform());

    // Instantiate shader and material nodes.
    mx::NodePtr shaderNode = doc->addNodeInstance(simpleSrf);
    mx::NodePtr materialNode = doc->addMaterialNode("", shaderNode);
    REQUIRE(materialNode->getUpstreamElement() == shaderNode);

    // Bind the diffuse color input to the constant color output.
    shaderNode->setConnectedOutput("diffColor", output);
    REQUIRE(shaderNode->getUpstreamElement() == constant);

    // Bind the roughness input to a value.
    mx::InputPtr instanceRoughness = shaderNode->setInputValue("roughness", 0.5f);
    REQUIRE(instanceRoughness->getValue()->asA<float>() == 0.5f);
    REQUIRE(instanceRoughness->getDefaultValue()->asA<float>() == 0.25f);

    // Create a collection 
    mx::CollectionPtr collection = doc->addCollection();
    REQUIRE(doc->getCollections().size() == 1);
    REQUIRE(doc->getCollection(collection->getName()));
    doc->removeCollection(collection->getName());
    REQUIRE(doc->getCollections().size() == 0);

    // Create a property set
    mx::PropertySetPtr propertySet = doc->addPropertySet();
    REQUIRE(doc->getPropertySets().size() == 1);
    REQUIRE(doc->getPropertySet(propertySet->getName()) != nullptr);
    doc->removePropertySet(propertySet->getName());
    REQUIRE(doc->getPropertySets().size() == 0);

    // Validate the document.
    REQUIRE(doc->validate());

    // Create a namespaced custom library.
    mx::DocumentPtr customLibrary = mx::createDocument();
    customLibrary->setNamespace("custom");
    mx::NodeGraphPtr customNodeGraph = customLibrary->addNodeGraph("NG_custom");
    mx::NodeDefPtr customNodeDef = customLibrary->addNodeDef("ND_simpleSrf", mx::SURFACE_SHADER_TYPE_STRING, "simpleSrf");
    mx::ImplementationPtr customImpl = customLibrary->addImplementation("IM_custom");
    customNodeGraph->addNodeInstance(customNodeDef, "custom1");
    customImpl->setNodeDef(customNodeDef);
    REQUIRE(customLibrary->validate());

    // Import the custom library.
    doc->importLibrary(customLibrary);
    mx::NodeGraphPtr importedNodeGraph = doc->getNodeGraph("custom:NG_custom");
    mx::NodeDefPtr importedNodeDef = doc->getNodeDef("custom:ND_simpleSrf");
    mx::ImplementationPtr importedImpl = doc->getImplementation("custom:IM_custom");
    mx::NodePtr importedNode = importedNodeGraph->getNode("custom1");
    REQUIRE(importedNodeDef != nullptr);
    REQUIRE(importedNode->getNodeDef() == importedNodeDef);
    REQUIRE(importedImpl->getNodeDef() == importedNodeDef);

    // Validate the combined document.
    REQUIRE(doc->validate());
}

TEST_CASE("Document equivalence", "[document]")
{
    mx::DocumentPtr doc = mx::createDocument();
    std::multimap<std::string, std::string> inputMap;

    inputMap.insert({ "color3", "  1.0,   +2.0,  3.0   " });
    inputMap.insert({ "color4", "1.0,   2.00, 0.3000, -4" });
    inputMap.insert({ "float", "  1.2e-10  " });
    inputMap.insert({ "float", "  00.1000  " });
    inputMap.insert({ "integer", "  12 " });
    inputMap.insert({ "matrix33",
                      "01.0,         2.0,  0000.2310, "
                      "   01.0,         2.0,  0000.2310, "
                      "01.0,         2.0,  0000.2310       " });
    inputMap.insert({ "matrix44",
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100" });
    inputMap.insert({ "vector2", "1.0,   0.012345608" }); // For precision check
    inputMap.insert({ "vector3", "  1.0,   +2.0,  3.0   " });
    inputMap.insert({ "vector4", "1.0,   2.00, 0.3000, -4" });
    inputMap.insert({ "string", "mystring" });
    inputMap.insert({ "boolean", "false" });
    inputMap.insert({ "filename", "filename1" });

    unsigned int index = 0;
    for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
    {
        const std::string inputType = (*it).first;
        mx::InputPtr input = doc->addInput("input" + std::to_string(index), inputType);
        if (inputType == "float")
        {
            input->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "  0.0100 ");
            input->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "  01.0100 ");
        }
        input->setValueString((*it).second);
        index++;
    }

    mx::DocumentPtr doc2 = mx::createDocument();
    std::multimap<std::string, std::string> inputMap2;
    inputMap2.insert({ "color3", "1, 2, 3" });
    inputMap2.insert({ "color4", "1, 2, 0.3, -4" });
    inputMap2.insert({ "float", "1.2e-10" });
    inputMap2.insert({ "float", "0.1" });
    inputMap2.insert({ "integer", "12" });
    inputMap2.insert({ "matrix33", "1, 2, 0.231,  1, 2, 0.231,  1, 2, 0.231,  1, 2, 0.231" });
    inputMap2.insert({ "matrix44", "1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1" });
    inputMap2.insert({ "vector2", "1, 0.012345611" }); // For precision check
    inputMap2.insert({ "vector3", "1, 2, 3" });
    inputMap2.insert({ "vector4", "1, 2, 0.3, -4" });
    inputMap2.insert({ "string", "mystring" });
    inputMap2.insert({ "boolean", "false" });
    inputMap2.insert({ "filename", "filename1" });

    index = 0;
    for (auto it = inputMap2.begin(); it != inputMap2.end(); ++it)
    {
        const std::string inputType = (*it).first;
        mx::InputPtr input = doc2->addInput("input" + std::to_string(index), inputType);
        input->setValueString((*it).second);
        if (inputType == "float")
        {
            input->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "  0.0100 ");
            input->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "  01.0100 ");
        }
        index++;
    }

    // Check attibute values 
    mx::ElementEquivalenceOptions options;
    bool equivalent = doc->isEquivalent(doc2, options);
    if (!equivalent)
    {
        std::cout << "Document 1: " << mx::prettyPrint(doc) << std::endl;
        std::cout << "Document 2: " << mx::prettyPrint(doc2) << std::endl;
    }
    REQUIRE(equivalent);

    unsigned int currentPrecision = mx::Value::getFloatPrecision();
    // This will compare 0.012345608 versus: 1, 0.012345611 for input10
    options.precision = 8;
    options.status = mx::EMPTY_STRING;
    equivalent = doc->isEquivalent(doc2, options);
    if (equivalent)
    {
        std::cout << "Document 1: " << mx::prettyPrint(doc) << std::endl;
        std::cout << "Document 2: " << mx::prettyPrint(doc2) << std::endl;
    }
    else
    {
        std::cout << "Expected difference: " << options.status << std::endl;
    }
    REQUIRE(!equivalent);
    options.precision = currentPrecision;

    // Check attribute order ignore. Some inputs have differing attribute order
    // which will be caught.
    options.ignoreAttributeOrder = false;
    options.status = mx::EMPTY_STRING;
    equivalent = doc->isEquivalent(doc2, options);
    if (equivalent)
    {
        std::cout << "Document 1: " << mx::prettyPrint(doc) << std::endl;
        std::cout << "Document 2: " << mx::prettyPrint(doc2) << std::endl;
    }
    else
    {
        std::cout << "Expected difference: " << options.status << std::endl;
    }
    REQUIRE(!equivalent);

    // Check attribute filtering of inputs with differing attribute ordering.
    options.ignoreAttributeOrder = false;
    options.status = mx::EMPTY_STRING;
    options.skipAttributes = { mx::ValueElement::UI_MIN_ATTRIBUTE, mx::ValueElement::UI_MAX_ATTRIBUTE };
    equivalent = doc->isEquivalent(doc2, options);
    if (!equivalent)
    {
        std::cout << "Document 1: " << mx::prettyPrint(doc) << std::endl;
        std::cout << "Document 2: " << mx::prettyPrint(doc2) << std::endl;
    }
    REQUIRE(equivalent);

}
