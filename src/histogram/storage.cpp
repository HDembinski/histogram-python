// Copyright 2015-2018 Hans Dembinski and Henry Schreiner
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/histogram/python/pybind11.hpp>

#include <boost/histogram/python/storage.hpp>

#include <boost/histogram.hpp>
#include <boost/histogram/storage_adaptor.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace bh = boost::histogram;

void register_storage(py::module &m) {
    
    py::class_<bh::unlimited_storage<>>(m, "unlimited_storage", "Default adaptive storage")
    .def(py::init<>(), "Default constructor")
    ;
    
    py::class_<vector_int_storage>(m, "vector_int_storage", "Integers in vectors storage type")
    .def(py::init<>(), "Default constructor")
    ;
    
    py::class_<bh::weight_storage>(m, "weight_storage", "Weighted storage type")
    .def(py::init<>(), "Default constructor")
    ;
    
    
}
