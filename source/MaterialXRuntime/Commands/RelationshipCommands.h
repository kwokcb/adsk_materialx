//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTRELATIONSHIPCOMMANDS_H
#define MATERIALX_RTRELATIONSHIPCOMMANDS_H

/// @file
/// Commands for connection handling.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Make a connection between a relationship port and a target object.
    void makeRelationship(const RtRelationship& rel, const RtObject& target, RtCommandResult& result);

    /// Break a connection between a relationship port and a target object.
    void breakRelationship(const RtRelationship& rel, const RtObject& target, RtCommandResult& result);
}

}

#endif
