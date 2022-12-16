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

string GraphIO::addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, 
                                  const ElementPtr node, const string& label) const
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
                NodeIO nodeIO;                
                NodePtr upstreamNode = upstreamElem->asA<Node>();
                if (node)
                {
                    NodeDefPtr upstreamNodeDef = upstreamNode->getNodeDef();
                    
                    nodeIO.group = upstreamNodeDef ? upstreamNodeDef->getNodeGroup() : EMPTY_STRING;
                }
                nodeIO.identifier = upstreamId;
                nodeIO.uilabel = writeCategoryNames ? upstreamElem->getCategory() : upstreamId;
                nodeIO.category = upstreamElem->getCategory();
                nodeIO.uishape = NodeIO::Box;
                currentGraphString += writeUpstreamNode(nodeIO);

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
                currentGraphString += writeConnection(outputPort, outputLabel, inputLabel);

                // Add downstream
                string downstreamCategory = downstreamElem->getCategory();
                string downstreamName = downstreamElem->getName();
                string downstreamId = addNodeToSubgraph(subGraphs, downstreamElem, downstreamName);                        

                nodeIO.identifier = downstreamId;
                nodeIO.uilabel = writeCategoryNames ? downstreamCategory : downstreamName;
                nodeIO.category = downstreamCategory;
                nodeIO.uishape = NodeIO::Box;
                currentGraphString += writeDownstreamNode(nodeIO, inputLabel);

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

                                nodeIO.identifier = interiorNodeId;
                                nodeIO.uilabel = interiorNodeLabel;
                                nodeIO.category = interiorNodeCategory;
                                nodeIO.uishape = NodeIO::RoundedBox;
                                currentGraphString += 
                                    writeInterfaceConnection(graphInterfaceName, interfaceInputName,
                                                             inputName, nodeIO);
                            }
                        }
                    }
                }

            }
        }

        if (!processedAny)
        {
            // Only add in the root node if no connections found during traversal
            NodeIO nodeIO;
            nodeIO.identifier = createValidName(root->getNamePath());
            nodeIO.category = root->getCategory();
            nodeIO.uilabel = writeCategoryNames ? nodeIO.category : nodeIO.identifier;
            nodeIO.uishape = NodeIO::Box;
            currentGraphString += writeRootNode(nodeIO);
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

string DotGraphIO::writeRootNode(const NodeIO& root)
{
    string result;
    result += GRAPH_INDENT + root.identifier + " [label= \"" + root.uilabel + "\"]\n";
    result += GRAPH_INDENT + root.identifier + "[shape = box];\n";
    result += GRAPH_INDENT + root.identifier;
    return result;
}

string DotGraphIO::writeUpstreamNode(
    const NodeIO& node)
{
    string result;
    result += GRAPH_INDENT + node.identifier + " [label= \"" + node.uilabel + "\"];\n";
    const string shape = (node.group == NodeDef::CONDITIONAL_NODE_GROUP) ? "diamond" : "box";
    result += GRAPH_INDENT + node.identifier + "[shape = " + shape + "];\n";
    result += GRAPH_INDENT + node.identifier;
    return result;
}

string DotGraphIO::writeConnection(
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
            dot += GRAPH_INDENT + outputName + " [shape = ellipse];\n";
            dot += GRAPH_INDENT + outputName + " -> ";
        }
    }
    
    return dot;
}

string DotGraphIO::writeInterfaceConnection(const string& interfaceId,
                                            const string& interfaceInputName,
                                            const string& inputName,
                                            const NodeIO& interiorNode)
{
    string dot;
    dot += GRAPH_INDENT + interfaceId + " [label=\"" + interfaceInputName + "\"];\n";
    dot += GRAPH_INDENT + interfaceId + " [shape = ellipse];\n";
    dot += GRAPH_INDENT + interiorNode.identifier + " [label=\"" + interiorNode.uilabel + "\"];\n";
    dot += GRAPH_INDENT + interfaceId + " -> " + interiorNode.identifier +
        " [label=" + GRAPH_QUOTE + "." + inputName + GRAPH_QUOTE + "];\n";

    return dot;
}

string DotGraphIO::writeDownstreamNode(const NodeIO& node, const string& inputLabel)
{
    string result;
    result += GRAPH_INDENT + node.identifier;
    if (!inputLabel.empty())
    {
        result += " [label= \"" + inputLabel + "\"]";
    }
    result += ";\n";
    
    result += GRAPH_INDENT + node.identifier + " [label= \"" + node.uilabel + "\"];\n";
    result += GRAPH_INDENT + node.identifier + "[shape = box];\n";

    return result;
}

string DotGraphIO::writeSubgraphs(std::unordered_map<string, StringSet> subGraphs)
{
    string result;

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
}

// Mermaid graph methods

MermaidGraphIOPtr MermaidGraphIO::create()
{
    return std::shared_ptr<MermaidGraphIO>(new MermaidGraphIO());
}

string MermaidGraphIO::writeUpstreamNode(const NodeIO& node)
{
    string result;
    if (node.group == NodeDef::CONDITIONAL_NODE_GROUP)
    {
        result = GRAPH_INDENT + node.identifier + "{" + node.uilabel + "}";
    } 
    else
    {
        result = GRAPH_INDENT + node.identifier + "[" + node.uilabel + "]";
    }
    return result;
}

string MermaidGraphIO::writeConnection(const string& outputName,
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

string MermaidGraphIO::writeDownstreamNode(const NodeIO& node, const string& /*inputLabel*/)
{
    string result;
    if (node.category != Output::CATEGORY)
    {
        result = node.identifier + "[" + node.uilabel + "]" + "\n";
    }
    else
    {
        result = node.identifier + "([" + node.uilabel + "])" + "\n";
        result += GRAPH_INDENT + "style " + node.identifier + " fill:#1b1, color:#111\n";
    }
    return result;
}

string MermaidGraphIO::writeInterfaceConnection(const string& interfaceId,
                                                const string& interfaceInputName,
                                                const string& inputName,
                                                const NodeIO& interiorNode)
{
    string result;
    result = GRAPH_INDENT + interfaceId + "([" + interfaceInputName + "])";
    result += " ==." + inputName;
    result += "==> " + interiorNode.identifier + "[" + interiorNode.uilabel + "]" + "\n";
    result += GRAPH_INDENT + "style " + interfaceId + " fill:#0bb, color:#111\n";

    return result;
}

string MermaidGraphIO::writeRootNode(const NodeIO& root)
{
    string result = "   " + root.identifier + "[" + root.uilabel + "]\n";
    return result;
}

string MermaidGraphIO::writeSubgraphs(std::unordered_map<string, StringSet> subGraphs)
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
