// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#include <boost/histogram/python/pybind11.hpp>
#include <pybind11/operators.h>

#include <boost/histogram/python/storage.hpp>
#include <boost/histogram/python/cereal.hpp>

#include <boost/histogram.hpp>
#include <boost/histogram/storage_adaptor.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

/// Add helpers common to all storage types
template<typename A, typename T>
py::class_<A> register_storage_by_type(py::module& m, const char* name, const char* desc) {
    py::class_<A> storage(m, name, desc);
    
    storage
    .def(py::init<>())
    .def("__getitem__", [](A& self, size_t ind){return self.at(ind);})
    .def("__setitem__", [](A& self, size_t ind, T val){self.at(ind) = val;})
    .def("push_back", [](A& self, T val){self.push_back(val);})
    .def(py::self == py::self)
    .def(py::self != py::self)
    //.def(py::pickle([](const A &p){return pickle_totuple(p);},
    //                [](py::tuple t){return pickle_fromtuple<A>(t);}
    //                ))
    ;
    
    return storage;
}


void register_storage(py::module &m) {

    py::module storage = m.def_submodule("storage");

    // Fast storages

    register_storage_by_type<storage::int_, unsigned>(storage, "int", "Integers in vectors storage type");
    
    py::class_<storage::double_>(storage, "double", "Weighted storage without variance type (fast but simple)")
    .def(py::init<>())
    ;
    
    py::class_<storage::atomic_int>(storage, "atomic_int", "Threadsafe (not growing axis) integer storage")
    .def(py::init<>())
    ;

    // Default storages

    py::class_<storage::unlimited>(storage, "unlimited", "Optimized for unweighted histograms, adaptive")
    .def(py::init<>())
    ;

    py::class_<storage::weight>(storage, "weight", "Dense storage which tracks sums of weights and a variance estimate")
    .def(py::init<>())
    ;

    py::class_<storage::profile>(storage, "profile", "Dense storage which tracks means of samples in each cell")
    .def(py::init<>())
    ;

    py::class_<storage::weighted_profile>(storage, "weighted_profile", "Dense storage which tracks means of weighted samples in each cell")
    .def(py::init<>())
    ;

}

storage::any_variant extract_storage(py::kwargs kwargs) {

    if(kwargs.contains("storage")) {
        try {
            return py::cast<storage::int_>(kwargs["storage"]);
        } catch(const py::cast_error&) {}

        try {
            return py::cast<storage::double_>(kwargs["storage"]);
        } catch(const py::cast_error&) {}

        try {
            return py::cast<storage::unlimited>(kwargs["storage"]);
        } catch(const py::cast_error&) {}

        try {
            return py::cast<storage::weight>(kwargs["storage"]);
        } catch(const py::cast_error&) {}

        try {
            return py::cast<storage::atomic_int>(kwargs["storage"]);
        } catch(const py::cast_error&) {}

        throw std::runtime_error("Storage type not supported");

    } else {
        // Default storage if not is specified
        return storage::int_();
    }
}
