//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/GraphIO.h>

MATERIALX_NAMESPACE_BEGIN

const GraphElementPtr MermaidGraphIO::read(const string&)
{
    return nullptr;
}

string MermaidGraphIO::addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, const ElementPtr node, const string& label) const
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

string MermaidGraphIO::write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames)
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
                string upstreamLabel = addNodeToSubgraph(subGraphs, upstreamElem, upstreamElem->getName());
                currentGraphString += "    " + upstreamLabel + "[" + 
                    (writeCategoryNames ? upstreamElem->getCategory() : upstreamLabel) + "]";

                // Add connecting edges
                //
                string upStreamPortLabel;
                if (connectingElem)
                {
                    string connectingElementString = "." + connectingElem->getName();
                    // Check for an explicit output name
                    string upStreamPort = connectingElem->getAttribute(PortElement::OUTPUT_ATTRIBUTE);
                    if (!upStreamPort.empty())
                    {
                        // Add the output to parent subgraph if any
                        // Upstream to Output connection
                        upStreamPortLabel = addNodeToSubgraph(subGraphs, upstreamElem, upStreamPort);                        
                        currentGraphString += " --> " + upStreamPortLabel + "([" + upStreamPort + "])" +
                            " --" + connectingElementString + "--> ";
                    }
                    else
                    {
                        currentGraphString += " --" + connectingElementString + "--> ";
                    }
                }
                else
                {
                    currentGraphString += "--> ";
                }

                // Add downstream
                string downstreamCategory = downstreamElem->getCategory();
                string downstreamName = downstreamElem->getName();
                string downstreamLabel = addNodeToSubgraph(subGraphs, downstreamElem, downstreamName);                        

                if (!downstreamElem->isA<Output>())
                {
                    currentGraphString += downstreamLabel + 
                        "[" + (writeCategoryNames ? downstreamCategory : downstreamName) + "]" + "\n";
                }
                else
                {
                    currentGraphString += downstreamLabel + 
                        "([" + (writeCategoryNames ? downstreamCategory : downstreamName) + "])" + "\n";
                    currentGraphString += "    style " + downstreamLabel + " fill:#1b1,color:#111\n";
                }

                if (!upStreamPortLabel.empty())
                {
                    currentGraphString += "    style " + upStreamPortLabel + " fill:#1b1,color:#111\n";
                }

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

                                const string interiorNodeLabel = upstreamElem->getNamePath();
                                const string interiorNodeCategory = upstreamElem->getCategory();
                                const string interiorNodeName = createValidName(interiorNodeLabel) + "[" + 
                                    (writeCategoryNames ? interiorNodeCategory : interiorNodeLabel) + "]";

                                currentGraphString += "    " + graphInterfaceName + "([" + input->getInterfaceName() + "])";
                                currentGraphString += " ==." + input->getName() + "==> " + interiorNodeName + "\n";
                                currentGraphString += "    style " + graphInterfaceName + " fill:#0bb,color:#111\n";
                            }
                        }
                    }
                }

            }
        }

        if (!processedAny)
        {
            // Only add in the root node if no connections found during traversal
            const string rootNamePath = root->getNamePath();
            const string rootNameCategory = root->getCategory();
            currentGraphString += "   " + createValidName(rootNamePath) + "[" + 
                (writeCategoryNames ? rootNameCategory : rootNamePath) + "]\n";
        }
    }

    // Add output for nodes in subgraphs
    for (auto subGraph : subGraphs)
    {
        currentGraphString += "  subgraph " + subGraph.first + "\n";
        for (auto item : subGraph.second)
        {
            currentGraphString += "    " + item + "\n";
        }
        currentGraphString += "  end\n";
    }

    // Output entire graph
    string outputString = "```mermaid\n";
    outputString += "graph TD;\n";
    outputString += currentGraphString;
    outputString += "```\n";

    return outputString;
}

MATERIALX_NAMESPACE_END
