//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <JsMaterialX/Helpers.h>
#include "./StrContainerTypeRegistration.h"
#include <MaterialXFormat/JsonIo.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xmlio)
{
    ems::constant("JSON_EXTENSION", mx::JSON_EXTENSION);
    ems::constant("JSON_MIME_TYPE", mx::JSON_MIME_TYPE);
    ems::constant("JSON_CATEGORY_NAME_SEPARATOR", mx::JSON_CATEGORY_NAME_SEPARATOR);

    ems::class_<mx::JsonReadOptions>("JsonReadOptions")
        .constructor<>()
        .property("upgradeVersion", &mx::JsonReadOptions::upgradeVersion)                

    ems::class_<mx::JsonWriteOptions>("JsonWriteOptions")
        .constructor<>()
        .property("elementPredicate", &mx::JsonWriteOptions::elementPredicate)
        .property("indent", &mx::JsonWriteOptions::indent)
        .property("indentCharacter", &mx::JsonWriteOptions::indentCharacter)
        ;

    BIND_FUNC_RAW_PTR("readFromJsonFile", mx::readFromJsonFile, 2, 4, mx::DocumentPtr, mx::FilePath,
        mx::FileSearchPath, const mx::JsonReadOptions*);
    BIND_FUNC_RAW_PTR("readFromJsonString", mx::readFromJsonString, 1, 2, mx::DocumentPtr, const std::string,
        const mx::JsonReadOptions*);
    BIND_FUNC_RAW_PTR("writeToJsonFile", mx::writeToJsonFile, 2, 3, mx::DocumentPtr, const mx::FilePath&, const mx::JsonWriteOptions *);
    BIND_FUNC_RAW_PTR("writeToJsonString", mx::writeToJsonString, 1, 2, mx::DocumentPtr, const mx::JsonWriteOptions *);
}
