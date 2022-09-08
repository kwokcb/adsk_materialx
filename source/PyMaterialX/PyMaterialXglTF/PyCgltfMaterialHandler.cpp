
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXglTF/CgltfMaterialHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyMaterialHandler: public mx::MaterialHandler
{
  public:
    PyMaterialHandler() :
        mx::MaterialHandler()
    {
    }

    bool load(const mx::FilePath& filePath) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::MaterialHandler,
            load,
            filePath
        );

    }

    bool save(const mx::FilePath& filePath) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::MaterialHandler,
            save,
            filePath
        );
    }

};


void bindPyCgltfMaterialHandler(py::module& mod)
{
    py::class_<mx::MaterialHandler, PyMaterialHandler, mx::MaterialHandlerPtr>(mod, "MaterialHandler")
        .def(py::init<>())
        .def("extensionsSupported", &mx::MaterialHandler::extensionsSupported)
        //.def("load", &mx::MaterialHandler::load)
        //.def("save", &mx::MaterialHandler::save)
        .def("extensionsSupported", &mx::MaterialHandler::extensionsSupported)
        .def("setDefinitions", &mx::MaterialHandler::setDefinitions)
        .def("setMaterials", &mx::MaterialHandler::setMaterials)
        .def("getMaterials", &mx::MaterialHandler::getMaterials)
        .def("setGenerateAssignments", &mx::MaterialHandler::setGenerateAssignments)
        .def("getGenerateAssignments", &mx::MaterialHandler::getGenerateAssignments)
        .def("setGenerateFullDefinitions", &mx::MaterialHandler::setGenerateFullDefinitions)
        .def("getGenerateFullDefinitions", &mx::MaterialHandler::getGenerateFullDefinitions);

    py::class_<mx::CgltfMaterialHandler, mx::MaterialHandler, mx::CgltfMaterialHandlerPtr>(mod, "CgltfMaterialHandler")
        .def_static("create", &mx::CgltfMaterialHandler::create)
        .def(py::init<>())
        .def("load", &mx::CgltfMaterialHandler::load)
        .def("save", &mx::CgltfMaterialHandler::save);
}
