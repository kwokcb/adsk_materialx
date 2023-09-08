//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/JsonIo.h>
#include <MaterialXCore/Document.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyJsonIo(py::module& mod)
{
    py::class_<mx::JsonReadOptions>(mod, "JsonReadOptions")
        .def(py::init())
        .def_readwrite("upgradeVersion", &mx::JsonReadOptions::upgradeVersion);

    py::class_<mx::JsonWriteOptions>(mod, "JsonWriteOptions")
        .def(py::init())
        .def_readwrite("elementPredicate", &mx::JsonWriteOptions::elementPredicate)
        .def_readwrite("indent", &mx::JsonWriteOptions::indent)
        .def_readwrite("indentCharacter", &mx::JsonWriteOptions::indentCharacter);

    mod.def("readFromJsonFile", &mx::readFromJsonFile,
        py::arg("doc"), py::arg("filename"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("readOptions") = (mx::JsonReadOptions*) nullptr);
    mod.def("readFromJsonString", &mx::readFromJsonString,
        py::arg("doc"), py::arg("str"), py::arg("readOptions") = (mx::JsonReadOptions*) nullptr);
    mod.def("writeToJsonFile", mx::writeToJsonFile,
        py::arg("doc"), py::arg("filename"), py::arg("writeOptions") = (mx::JsonWriteOptions*) nullptr);
    mod.def("writeToJsonString", mx::writeToJsonString,
        py::arg("doc"), py::arg("writeOptions") = nullptr);

    mod.attr("JSON_EXTENSION") = mx::JSON_EXTENSION;
    mod.attr("JSON_MIME_TYPE") = mx::JSON_MIME_TYPE;
    mod.attr("JSON_CATEGORY_NAME_SEPARATOR") = mx::JSON_CATEGORY_NAME_SEPARATOR;
}
