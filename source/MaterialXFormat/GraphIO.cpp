//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/GraphIO.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

static string GRAPH_INDENT = "    ";
static string GRAPH_QUOTE = "\"";

// Base class methods

string GraphIO::addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, const ElementPtr node, const string& label) const
{   
    if (!node)
    {
        return EMPTY_STRING;
    }

    string subgraphNodeName = label;
    const ElementPtr subgraph = node->getParent();
    if (!subgraph)
    {
        return subgraphNodeName;
    }

    // Use full path to identify sub-graphs
    // A Document has no path so even though it is a GraphElement it will not be added here
    string graphId = createValidName(subgraph->getNamePath());
    if (!graphId.empty())
    {
        subgraphNodeName = graphId + "_" + subgraphNodeName;
        if (subGraphs.count(graphId))
        {
            subGraphs[graphId].insert(subgraphNodeName);
        }
        else
        {
            StringSet newSet;
            newSet.insert(subgraphNodeName);
            subGraphs[graphId] = newSet;
        }
    }

    return subgraphNodeName;
}

string GraphIO::writeGraph(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames)
{
    string currentGraphString;

    // Write out all connections.
    std::set<Edge> processedEdges;
    StringSet processedInterfaces;
    std::unordered_map<string, StringSet> subGraphs;

    std::vector<OutputPtr> outputs;
    if (!roots.empty())
    {
        for (OutputPtr out : roots)
        {
            outputs.push_back(out);
        }
    }
    else
    {
        outputs = graph->getOutputs();
    }

    for (OutputPtr output : outputs)
    {
        ElementPtr root;
        ElementPtr parent = output->getParent();
        NodePtr node = parent->asA<Node>();
        if (!parent->isA<NodeGraph>() &&
            (node && node->getType() != MATERIAL_TYPE_STRING))
        {
            root = parent;
        }
        else
        {
            root = output;
        }

        bool processedAny = false;
        for (Edge edge : root->traverseGraph())
        {
            if (!processedEdges.count(edge))
            {
                processedEdges.insert(edge);

                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();

                processedAny = true;

                // Add upstream nodes. 
                // - Add list of unique nodes to parent subgraph if it exists
                // - Output upstream node + label (id or category)
                string upstreamId = addNodeToSubgraph(subGraphs, upstreamElem, upstreamElem->getName());
                currentGraphString += writeUpstreamNode(upstreamId,
                    (writeCategoryNames ? upstreamElem->getCategory() : upstreamId));

                // Add connecting edges
                //
                string outputPort;
                string inputLabel;
                string outputLabel;
                if (connectingElem)
                {
                    inputLabel = "." + connectingElem->getName();
                    // Check for an explicit output name
                    outputLabel = connectingElem->getAttribute(PortElement::OUTPUT_ATTRIBUTE);
                    if (!outputLabel.empty())
                    {
                        // Add the output to parent subgraph if any
                        // Upstream to Output connection
                        outputPort = addNodeToSubgraph(subGraphs, upstreamElem, outputLabel);                        
                    }
                }
                currentGraphString += writeConnectingElement(outputPort, outputLabel, inputLabel);

                // Add downstream
                string downstreamCategory = downstreamElem->getCategory();
                string downstreamName = downstreamElem->getName();
                string downstreamId = addNodeToSubgraph(subGraphs, downstreamElem, downstreamName);                        

                string dowstreamLabel = writeCategoryNames ? downstreamCategory : downstreamName;
                currentGraphString += writeDownstreamNode(downstreamId, dowstreamLabel, downstreamCategory,
                                                          inputLabel);

                NodePtr upstreamNode = upstreamElem->asA<Node>();
                const string upstreamNodeName = upstreamNode ? upstreamNode->getName() : EMPTY_STRING;
                if (upstreamNode && !processedInterfaces.count(upstreamNodeName))
                {
                    processedInterfaces.insert(upstreamNodeName);

                    for (InputPtr input : upstreamNode->getInputs())
                    {
                        if (input->hasInterfaceName())
                        {
                            const string interfaceName = input->getInterfaceName();
                            const ElementPtr upstreamParent = upstreamNode->getParent() ? upstreamNode->getParent() : nullptr;
                            NodeGraphPtr upstreamGraph = nullptr;
                            if (upstreamNode->getParent())
                            {
                                upstreamGraph = upstreamNode->getParent()->asA<NodeGraph>();
                            }
                            const InputPtr interfaceInput = upstreamGraph ? upstreamGraph->getInput(interfaceName) : nullptr;
                            if (interfaceInput && !interfaceInput->getConnectedNode())
                            {
                                string graphInterfaceName = addNodeToSubgraph(subGraphs, upstreamNode, input->getInterfaceName());                        

                                const string interiorNodeId = createValidName(upstreamElem->getNamePath());
                                const string interiorNodeCategory = upstreamElem->getCategory();
                                const string interiorNodeLabel = writeCategoryNames ? interiorNodeCategory : interiorNodeId;

                                const string interfaceInputName = input->getInterfaceName();
                                const string inputName = input->getName();
                                currentGraphString += 
                                    writeInterfaceConnection(graphInterfaceName, interfaceInputName,
                                                             inputName, interiorNodeId, interiorNodeLabel);
                            }
                        }
                    }
                }

            }
        }

        if (!processedAny)
        {
            // Only add in the root node if no connections found during traversal
            const string rootName = createValidName(root->getNamePath());
            const string rootLabel= writeCategoryNames ? root->getCategory() : rootName;
            currentGraphString += writeRootNode(rootName, rootLabel);
        }
    }

    // Add output for nodes in subgraphs.
    // Needs to be done before the graph for dot outout.
    string subGraphString = writeSubgraphs(subGraphs);
    currentGraphString = subGraphString + currentGraphString;

    // Output entire graph
    const string orientation = "TD";
    string outputString = writeGraphString(currentGraphString, orientation);

    return outputString;
}

// dot class methods

DotGraphIOPtr DotGraphIO::create()
{
    return std::shared_ptr<DotGraphIO>(new DotGraphIO());
}

string DotGraphIO::writeRootNode(const string& rootName,
    const string& rootLabel)
{
    string result;
    result += GRAPH_INDENT + rootName + " [label= \"" + rootLabel + "\"]\n";
    result += GRAPH_INDENT + rootName + "[shape = box];\n";
    result += GRAPH_INDENT + rootName;
    return result;
}

string DotGraphIO::writeUpstreamNode(
    const string& nodeName,
    const string& nodeLabel)
{
    string result;
    result += GRAPH_INDENT + nodeName + " [label= \"" + nodeLabel + "\"];\n";
    result += GRAPH_INDENT + nodeName + "[shape = box];\n";
    result += GRAPH_INDENT + nodeName;
    return result;
}

string DotGraphIO::writeConnectingElement(
    const string& outputName,
    const string& outputLabel,
    const string& inputLabel)
{
    string dot;

    dot += " -> ";
    if (!inputLabel.empty())
    {
        if (!outputLabel.empty() && !outputName.empty())
        {
            dot += outputName + ";\n";
            dot += GRAPH_INDENT + outputName + " [label= \"" + outputLabel + "\"];\n";
            //dot += GRAPH_INDENT + outputName + " [shape = doublecircle];\n";
            dot += GRAPH_INDENT + outputName + " -> ";
        }
    }
    
    return dot;
}

string DotGraphIO::writeInterfaceConnection(
    const string& interfaceId,
    const string& interfaceInputName,
    const string& inputName,
    const string& interiorNodeId,
    const string& interiorNodeLabel)
{
    string dot;
    dot += GRAPH_INDENT + interfaceId + " [label=\"" + interfaceInputName + "\"];\n";
    //dot += GRAPH_INDENT + interfaceId + " [shape = doublecircle];\n";
    dot += GRAPH_INDENT + interiorNodeId + " [label=\"" + interiorNodeLabel + "\"];\n";
    dot += GRAPH_INDENT + interfaceId + " -> " + interiorNodeId +
        " [label=" + GRAPH_QUOTE + "." + inputName + GRAPH_QUOTE + "];\n";

    return dot;
}

string DotGraphIO::writeDownstreamNode(
    const string& nodeName,
    const string& nodeLabel,
    const string& /*category*/, 
    const string& inputLabel)
{
    string result;
    result += GRAPH_INDENT + nodeName;
    if (!inputLabel.empty())
    {
        result += " [label= \"" + inputLabel + "\"]";
    }
    result += ";\n";
    
    result += GRAPH_INDENT + nodeName + " [label= \"" + nodeLabel + "\"];\n";
    result += GRAPH_INDENT + nodeName + "[shape = box];\n";

    return result;
}

string DotGraphIO::writeSubgraphs(
    std::unordered_map<string, StringSet> subGraphs)
{
    string result;
    //result += GRAPH_INDENT + "fillcolor = white;\n";

    unsigned int clusterNumber = 1;
    const string CLUSTER_STRING = "cluster_";
    for (auto subGraph : subGraphs)
    {
        // Note that the graph must start with the prefix "cluster"
        result += GRAPH_INDENT + "subgraph " + CLUSTER_STRING + std::to_string(clusterNumber)+ "{\n";
        result += GRAPH_INDENT + "  style = filled;\n";
        result += GRAPH_INDENT + "  fillcolor = lightyellow;\n";
        result += GRAPH_INDENT + "  color = black;\n";
        result += GRAPH_INDENT + "  node[style = filled, fillcolor = white];\n";
        result += GRAPH_INDENT + "  label = \"" + subGraph.first + "\";\n";

        for (auto item : subGraph.second)
        {
            result += GRAPH_INDENT + "  " + item + "\n";
        }
        result += GRAPH_INDENT  + "}\n\n";

        clusterNumber++;
    }
    return result;
}

string DotGraphIO::writeGraphString(const string& graphString, const string& /*orientation*/)
{
    string dot = "digraph {\n";
    dot += graphString;
    dot += "}\n";

    return dot;
}

string DotGraphIO::write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames)
{
    return writeGraph(graph, roots, writeCategoryNames);
#if 0
    string dot = "digraph {\n";

    // Create a unique name for each child element.
    // Either use the category name or the path name
    vector<ElementPtr> children = graph->topologicalSort();
    StringMap nameMap;
    StringSet nameSet;
    for (ElementPtr elem : children)
    {
        string uniqueName = writeCategoryNames ? elem->getCategory() : createValidName(elem->getNamePath());
        while (nameSet.count(uniqueName))
        {
            uniqueName = incrementName(uniqueName);
        }
        nameMap[elem->getName()] = uniqueName;
        nameSet.insert(uniqueName);
    }

    // Write out all nodes.
    for (ElementPtr elem : children)
    {
        NodePtr node = elem->asA<Node>();
        if (node)
        {
            dot += GRAPH_INDENT + "\"" + nameMap[node->getName()] + "\" ";
            NodeDefPtr nodeDef = node->getNodeDef();
            const string& nodeGroup = nodeDef ? nodeDef->getNodeGroup() : EMPTY_STRING;
            if (nodeGroup == NodeDef::CONDITIONAL_NODE_GROUP)
            {
                dot += "[shape=diamond];\n";
            }
            else
            {
                dot += "[shape=box];\n";
            }
        }
    }

    // Write out all connections.
    std::set<Edge> processedEdges;
    StringSet processedInterfaces;
    for (OutputPtr output : graph->getOutputs())
    {
        for (Edge edge : output->traverseGraph())
        {
            if (!processedEdges.count(edge))
            {
                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();

                dot += GRAPH_INDENT + "\"" + nameMap[upstreamElem->getName()];
                dot += "\" -> \"" + nameMap[downstreamElem->getName()];
                dot += "\" [label=\"";
                dot += connectingElem ? connectingElem->getName() : EMPTY_STRING;
                dot += "\"];\n";

                NodePtr upstreamNode = upstreamElem->asA<Node>();
                if (upstreamNode && !processedInterfaces.count(upstreamNode->getName()))
                {
                    for (InputPtr input : upstreamNode->getInputs())
                    {
                        if (input->hasInterfaceName())
                        {
                            dot += GRAPH_INDENT + "\"" + input->getInterfaceName();
                            dot += "\" -> \"" + nameMap[upstreamElem->getName()];
                            dot += "\" [label=\"";
                            dot += input->getName();
                            dot += "\"];\n";
                        }
                    }
                    processedInterfaces.insert(upstreamNode->getName());
                }

                processedEdges.insert(edge);
            }
        }
    }

    dot += "}\n";

    return dot;
#endif
}

// Mermaid graph methods

MermaidGraphIOPtr MermaidGraphIO::create()
{
    return std::shared_ptr<MermaidGraphIO>(new MermaidGraphIO());
}

string MermaidGraphIO::writeUpstreamNode(const string& nodeName,
    const string& nodeLabel)
{
    return (GRAPH_INDENT + nodeName + "[" + nodeLabel + "]");
}

string MermaidGraphIO::writeConnectingElement(const string& outputName,
    const string& outputLabel, const string& inputLabel)
{
    string result;

    if (!inputLabel.empty())
    {
        if (!outputLabel.empty() && !outputName.empty())
        {
            result = " --> " + outputName + "([" + outputLabel + "])\n";
            result += GRAPH_INDENT + "style " + outputName + " fill:#1b1, color:#111\n";
            result += GRAPH_INDENT + outputName + " --" + inputLabel + "--> ";
        }
        else
        {
            result = " --" + inputLabel + "--> ";
        }
    }
    else
    {
        result = " --> ";
    }

    return result;
}

string MermaidGraphIO::writeDownstreamNode(const string& nodeName,
    const string& nodeLabel, const string& category, const string& /*inputLabel*/)
{
    string result;
    if (category != Output::CATEGORY)
    {
        result = nodeName + "[" + nodeLabel + "]" + "\n";
    }
    else
    {
        result = nodeName + "([" + nodeLabel + "])" + "\n";
        result += GRAPH_INDENT + "style " + nodeName + " fill:#1b1, color:#111\n";
    }
    return result;
}

string MermaidGraphIO::writeInterfaceConnection(
    const string& interfaceId,
    const string& interfaceInputName,
    const string& inputName,
    const string& interiorNodeId,
    const string& interiorNodeLabel)
{
    string result;
    result = GRAPH_INDENT + interfaceId + "([" + interfaceInputName + "])";
    result += " ==." + inputName;
    result += "==> " + interiorNodeId + "[" + interiorNodeLabel + "]" + "\n";
    result += GRAPH_INDENT + "style " + interfaceId + " fill:#0bb, color:#111\n";

    return result;
}

string MermaidGraphIO::writeRootNode(const string& rootName,
                                     const string& rootLabel) 
{
    string result = "   " + rootName + "[" + rootLabel + "]\n";
    return result;
}

string MermaidGraphIO::writeSubgraphs(
    std::unordered_map<string, StringSet> subGraphs)
{
    string result;
    for (auto subGraph : subGraphs)
    {
        result += "  subgraph " + subGraph.first + "\n";
        for (auto item : subGraph.second)
        {
            result += GRAPH_INDENT + item + "\n";
        }
        result += "  end\n";
    }
    return result;
}

string MermaidGraphIO::writeGraphString(const string& graphString, const string& orientation)
{
    string result = "```mermaid\n";
    result += "graph " + orientation + "; \n";
    result += graphString;
    result += "```\n";

    return result;
}

string MermaidGraphIO::write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames)
{
    return writeGraph(graph, roots, writeCategoryNames);
}

GraphIORegistryPtr GraphIORegistry::create()
{
    return std::shared_ptr<GraphIORegistry>(new GraphIORegistry());
}

void GraphIORegistry::addGraphIO(GraphIOPtr graphIO)
{
    if (graphIO)
    {
        const StringSet& formats = graphIO->supportsFormats();
        for (const auto& format : formats)
        {
            _graphIOs[format].push_back(graphIO);
        }
    }
}

string GraphIORegistry::write(const string& format, GraphElementPtr graph, const std::vector<OutputPtr> roots, 
                bool writeCategoryNames)

{
    string result = EMPTY_STRING;
    for (GraphIOPtr graphIO : _graphIOs[format])
    {
        try
        {
            result = graphIO->write(graph, roots, writeCategoryNames);
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception in graph I/O library: " << e.what() << std::endl;
        }
        if (!result.empty())
        {
            return result;
        }
    }    
    return result;
}

MATERIALX_NAMESPACE_END
