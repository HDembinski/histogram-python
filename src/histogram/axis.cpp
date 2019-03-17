// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#include <boost/histogram/python/pybind11.hpp>

#include <pybind11/operators.h>

#include <boost/histogram/python/axis.hpp>

#include <boost/histogram/axis/ostream.hpp>
#include <boost/histogram.hpp>

#include <boost/histogram/axis/traits.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace bh = boost::histogram;

/// Add helpers common to all axis types
template<typename A, typename R=int>
py::class_<A> register_axis_by_type(py::module& m, const char* name, const char* desc) {
    py::class_<A> axis(m, name, desc);
    
    // using value_type = decltype(A::value(1.0));
    
    axis
    .def("__repr__", [](A &self){
        std::ostringstream out;
        out << self;
        return out.str();
    })
    
    .def(py::self == py::self)
    .def(py::self != py::self)
    
    .def("index", &A::index, "The index at a point on the axis", "x"_a)
    .def("bin", &A::bin, "The bin contents", "idx"_a)
    .def("value", &A::value, "The value for a fractional bin in the axis", "i"_a)
    .def("size", &A::size, "Returns the number of bins, without over- or underflow")
    .def("extent", [](const A& self){return bh::axis::traits::extend(self);},
         "Retuns the number of bins, including over- or underflow")
    .def_static("options", &A::options, "Return the options associated to the axis")
    ;
    
    
    return axis;
}

void register_axis(py::module &m) {
    
    py::module ax = m.def_submodule("axis");
    
    py::module opt = ax.def_submodule("options");
    
    opt.attr("none") =      (unsigned) bh::axis::option::none;
    opt.attr("underflow") = (unsigned) bh::axis::option::underflow;
    opt.attr("overflow") =  (unsigned) bh::axis::option::overflow;
    opt.attr("circular") =  (unsigned) bh::axis::option::circular;
    opt.attr("growth") =    (unsigned) bh::axis::option::growth;
    

    register_axis_by_type<axis::regular>(ax, "regular", "Evenly spaced bins")
    .def(py::init<unsigned, double, double>(), "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::regular_noflow>(ax, "regular_noflow", "Evenly spaced bins without over/under flow")
    .def(py::init<unsigned, double, double>(), "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::circular>(ax, "circular", "Evenly spaced bins with wraparound")
    .def(py::init<unsigned, double, double>(), "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::regular_log>(ax, "regular_log", "Evenly spaced bins in log10")
    .def(py::init<unsigned, double, double>(), "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::regular_sqrt>(ax, "regular_sqrt", "Evenly spaced bins in sqrt")
    .def(py::init<unsigned, double, double>(), "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::regular_pow>(ax, "regular_pow", "Evenly spaced bins in a power")
    .def(py::init([](double pow, unsigned n, double start, double stop){
        return new axis::regular_pow(bh::axis::transform::pow{pow}, n, start , stop);} ), "pow"_a, "n"_a, "start"_a, "stop"_a)
    ;
    
    register_axis_by_type<axis::variable>(ax, "variable", "Unevenly spaced bins")
    .def(py::init<std::vector<double>>(), "edges"_a)
    ;

    register_axis_by_type<axis::integer>(ax, "integer", "Contigious integers")
    .def(py::init<int, int>(), "min"_a, "max"_a)
    ;

    register_axis_by_type<axis::category_str, std::string>(ax, "category_str", "Text label bins")
    .def(py::init<std::vector<std::string>>(), "labels"_a)
    ;
}
