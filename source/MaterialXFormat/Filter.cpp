//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/Filter.h>

MATERIALX_NAMESPACE_BEGIN

const GraphElementPtr MermaidFilter::read(const string&)
{
    return nullptr;
}

string MermaidFilter::write(GraphElementPtr graph, const std::vector<OutputPtr> roots, bool writeCategoryNames)
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
                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();

                processedAny = true;

                // Add upstream
                ElementPtr upstreamElemParent = upstreamElem->getParent();
                string upstreamGraphId;
                string upstreamCategory = upstreamElem->getCategory();
                string upstreamName = upstreamElem->getName();
                string upstreamLabel = upstreamName;
                if (upstreamElemParent)
                {
                    upstreamGraphId = createValidName(upstreamElemParent->getNamePath());
                    if (!upstreamGraphId.empty())
                    {
                        upstreamLabel = upstreamGraphId + "_" + upstreamName;
                        if (subGraphs.count(upstreamGraphId))
                        {
                            subGraphs[upstreamGraphId].insert(upstreamLabel);
                        }
                        else
                        {
                            StringSet newSet;
                            newSet.insert(upstreamLabel);
                            subGraphs[upstreamGraphId] = newSet;
                        }
                    }
                }
                currentGraphString += "    " + upstreamLabel + "[" + 
                    (writeCategoryNames ? upstreamCategory : upstreamLabel) + "]";

                // Add connecting element
                string upStreamPortLabel;
                if (connectingElem)
                {
                    string connectingElementString = "." + connectingElem->getName();
                    string upStreamPort = connectingElem->getAttribute(PortElement::OUTPUT_ATTRIBUTE);
                    if (!upStreamPort.empty())
                    {
                        upstreamGraphId = createValidName(upstreamElemParent->getNamePath());
                        upStreamPortLabel = upstreamGraphId.empty() ? upStreamPort :
                            upstreamGraphId + "_" + upStreamPort;
                        if (!upstreamGraphId.empty())
                        {
                            if (subGraphs.count(upstreamGraphId))
                            {
                                subGraphs[upstreamGraphId].insert(upStreamPortLabel);
                            }
                            else
                            {
                                StringSet newSet;
                                newSet.insert(upStreamPortLabel);
                                subGraphs[upstreamGraphId] = newSet;
                            }
                        }
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
                ElementPtr downstreamParent = downstreamElem->getParent();
                string downstreamGraphId;
                string downstreamCategory = downstreamElem->getCategory();
                string downstreamName = downstreamElem->getName();
                string downstreamLabel = downstreamName;
                if (downstreamParent)
                {
                    downstreamGraphId = createValidName(downstreamParent->getNamePath());
                    if (!downstreamGraphId.empty())
                    {
                        downstreamLabel = downstreamGraphId + "_" + downstreamName;
                        if (subGraphs.count(downstreamGraphId))
                        {
                            subGraphs[downstreamGraphId].insert(downstreamLabel);
                        }
                        else
                        {
                            StringSet newSet;
                            newSet.insert(downstreamLabel);
                            subGraphs[downstreamGraphId] = newSet;
                        }
                    }
                }
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
                if (upstreamNode && !processedInterfaces.count(upstreamNode->getName()))
                {
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
                                string const upstreamParentPath = createValidName(upstreamNode->getParent()->getNamePath());
                                const string graphInterfaceLabel = input->getInterfaceName();
                                const string graphInterfaceName = upstreamParentPath + "_" + graphInterfaceLabel;

                                if (!upstreamParentPath.empty())
                                {
                                    if (subGraphs.count(upstreamParentPath))
                                    {
                                        subGraphs[upstreamParentPath].insert(graphInterfaceName);
                                    }
                                    else
                                    {
                                        StringSet newSet;
                                        newSet.insert(graphInterfaceName);
                                        subGraphs[downstreamGraphId] = newSet;
                                    }
                                }

                                const string interiorNodeLabel = upstreamElem->getNamePath();
                                const string interiorNodeCategory = upstreamElem->getCategory();
                                const string interiorNodeName = createValidName(interiorNodeLabel) + "[" + 
                                    (writeCategoryNames ? interiorNodeCategory : interiorNodeLabel) + "]";

                                currentGraphString += "    " + graphInterfaceName + "([" + graphInterfaceLabel + "])";
                                currentGraphString += " ==." + input->getName() + "==> " + interiorNodeName + "\n";
                                currentGraphString += "    style " + graphInterfaceName + " fill:#0bb,color:#111\n";
                            }
                        }
                    }
                    processedInterfaces.insert(upstreamNode->getName());
                }

                processedEdges.insert(edge);
            }
        }

        if (!processedAny)
        {
            const string rootNamePath = root->getNamePath();
            const string rootNameCategory = root->getCategory();
            currentGraphString += "   " + createValidName(rootNamePath) + "[" + 
                (writeCategoryNames ? rootNameCategory : rootNamePath) + "]\n";
        }
    }

    for (auto subGraph : subGraphs)
    {
        currentGraphString += "  subgraph " + subGraph.first + "\n";
        for (auto item : subGraph.second)
        {
            currentGraphString += "    " + item + "\n";
        }
        currentGraphString += "  end\n";
    }

    string outputString = "```mermaid\n";
    outputString += "graph TD;\n";
    outputString += currentGraphString;
    outputString += "```\n";

    return outputString;
}

MATERIALX_NAMESPACE_END
