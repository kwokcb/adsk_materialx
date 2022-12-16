//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/GraphIO.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyGraphIO : public mx::GraphIO
{
  public:
    std::string write(mx::GraphElementPtr graph, const std::vector<mx::OutputPtr> roots, bool writeCategoryNames = true) override
    {
        PYBIND11_OVERLOAD_PURE(
            std::string,
            mx::GraphIO,
            write,
            graph,
            roots,
            writeCategoryNames
        );
    }
};

void bindPyGraphIO(py::module& mod)
{
    py::enum_<mx::NodeIO::NodeShape>(mod, "NodeShape")
        .value("Box", mx::NodeIO::NodeShape::Box)
        .value("RoundedBox", mx::NodeIO::NodeShape::RoundedBox)
        .value("Diamond", mx::NodeIO::NodeShape::Diamond)
        .export_values();

    py::class_<mx::NodeIO>(mod, "NodeIO")
        .def_readwrite("identifier", &mx::NodeIO::identifier)
        .def_readwrite("uilabel", &mx::NodeIO::uilabel)
        .def_readwrite("category", &mx::NodeIO::category)
        .def_readwrite("group", &mx::NodeIO::group)
        .def_readwrite("uishape", &mx::NodeIO::uishape)
        .def(py::init<>());

    py::class_<mx::GraphIO, PyGraphIO, mx::GraphIOPtr>(mod, "GraphIO")
        .def("write", (std::string (mx::GraphIO::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>, bool)) &mx::GraphIO::write)
        .def("supportsFormats", &mx::GraphIO::supportsFormats);

    py::class_<mx::DotGraphIO, mx::GraphIO, mx::DotGraphIOPtr>(mod, "DotGraphIO")
        .def_static("create", &mx::DotGraphIO::create)
        .def("write", (std::string (mx::DotGraphIO::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>, bool)) &mx::DotGraphIO::write);

    py::class_<mx::MermaidGraphIO, mx::GraphIO, mx::MermaidGraphIOPtr>(mod, "MermaidGraphIO")
        .def_static("create", &mx::MermaidGraphIO::create)
        .def("write", (std::string (mx::MermaidGraphIO::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>, bool)) &mx::MermaidGraphIO::write);

    py::class_<mx::GraphIORegistry, mx::GraphIORegistryPtr>(mod, "GraphIORegistry")
        .def_static("create", &mx::GraphIORegistry::create)
        .def("addGraphIO", &mx::GraphIORegistry::addGraphIO)
        .def("write", &mx::GraphIORegistry::write);
}

