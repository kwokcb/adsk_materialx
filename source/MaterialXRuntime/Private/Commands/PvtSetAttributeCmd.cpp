//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtSetAttributeCmd.h>

#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

PvtSetAttributeCmd::PvtSetAttributeCmd(const RtAttribute& attr, const RtValue& value) :
    _attr(attr),
    _value(RtValue::clone(attr.getType(), value, attr.getParent()))
{
}

PvtCommandPtr PvtSetAttributeCmd::create(const RtAttribute& attr, const RtValue& value)
{
    return std::make_shared<PvtSetAttributeCmd>(attr, value);
}

void PvtSetAttributeCmd::execute(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Save old value for undo/redo
            _oldValue = RtValue::clone(_attr.getType(), _attr.getValue(), _attr.getParent());

            // Set the value
            _attr.setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

void PvtSetAttributeCmd::undo(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Reset the value
            _attr.setValue(_oldValue);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

void PvtSetAttributeCmd::redo(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Set the value
            _attr.setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

}
