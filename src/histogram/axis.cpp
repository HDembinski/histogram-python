// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#include <boost/histogram/python/pybind11.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/eval.h>

#include <boost/histogram/python/axis.hpp>
#include <boost/histogram/python/pickle.hpp>

#include <boost/histogram/axis/ostream.hpp>
#include <boost/histogram.hpp>

#include <boost/histogram/axis/traits.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std::literals;

// Base classes to allow type checking in Python
struct regular_base {};

/// Add items to an axis where the axis values are continious
template<typename A, typename B>
void add_to_axis(B&& axis, std::false_type) {
    axis.def("bin", &A::bin, "The bin details (center, lower, upper)", "idx"_a, py::keep_alive<0, 1>());
    axis.def("bins", [](const A& self, bool flow){
        return axis_to_bins(self, flow);
    }, "flow"_a=false, py::keep_alive<0, 1>());
    axis.def("index", py::vectorize(&A::index), "The index at a point(s) on the axis", "x"_a);
    axis.def("value", py::vectorize(&A::value), "The value(s) for a fractional bin(s) in the axis", "i"_a);

    axis.def("edges", [](const A& ax, bool flow){
        return axis_to_edges(ax, flow);
    }, "flow"_a = false, "The bin edges (length: bins + 1) (include over/underflow if flow=True)");

    axis.def("centers", [](const A& ax){
        py::array_t<double> centers((unsigned) ax.size());
        std::transform(ax.begin(), ax.end(), centers.mutable_data(), [](const auto& bin){return bin.center();});
        return centers;
    }, "Return the bin centers");
}

/// Add items to an axis where the axis values are not continious (categories of strings, for example)
template<typename A, typename B>
void add_to_axis(B&& axis, std::true_type) {
    axis.def("bin", &A::bin, "The bin name", "idx"_a);
    axis.def("bins", [](const A& self, bool flow){
        return axis_to_bins(self, flow);
    }, "flow"_a=false);
    // Not that these really just don't work with string labels; they would work for numerical labels.
    axis.def("index", &A::index, "The index at a point on the axis", "x"_a);
    axis.def("value", &A::value, "The value for a fractional bin in the axis", "i"_a);
}

/// Add helpers common to all axis types
template<typename A, typename... Args>
py::class_<A> register_axis_by_type(py::module& m, Args&&... args) {
    py::class_<A> axis(m, std::forward<Args>(args)...);

    // using value_type = decltype(A::value(1.0));

    axis
    .def("__repr__", shift_to_string<A>())

    .def(py::self == py::self)
    .def(py::self != py::self)

    .def("size", [](const A& self, bool flow){
        if(flow)
            return bh::axis::traits::extent(self);
        else
            return self.size();
    } , "flow"_a=false, "Returns the number of bins, without over- or underflow unless flow=True")

    .def("update", &A::update, "Bin and add a value if allowed", "i"_a)
    .def_static("options", &A::options, "Return the options associated to the axis")
    .def_property("metadata",
                  [](const A& self){return self.metadata();},
                  [](A& self, metadata_t label){self.metadata() = label;},
                  "Set the axis label")

    ;

    // We only need keepalive if this is a reference.
    using Result = decltype(std::declval<A>().bin(std::declval<int>()));

    // This is a replacement for constexpr if
    add_to_axis<A>(axis, std::integral_constant<bool, std::is_reference<Result>::value || std::is_integral<Result>::value>{});

    axis.def(make_pickle<A>());

    return axis;
}

/// Add helpers common to all types with a range of values
template<typename A>
py::class_<bh::axis::interval_view<A>> register_axis_iv_by_type(py::module& m, const char* name) {
    using A_iv = bh::axis::interval_view<A>;
    py::class_<A_iv> axis_iv = py::class_<A_iv>(m, name, "Lightweight bin view");

    axis_iv
    .def("upper", &A_iv::upper)
    .def("lower", &A_iv::lower)
    .def("center", &A_iv::center)
    .def("width", &A_iv::width)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def("__repr__", [](const A_iv& self){
        return "<bin ["s + std::to_string(self.lower()) + ", "s + std::to_string(self.upper()) + "]>"s;
    })
    ;

    return axis_iv;
}

struct regular_type {};


void register_axis(py::module &m) {

    py::module ax = m.def_submodule("axis");

    py::module opt = ax.def_submodule("options");

    opt.attr("none") =      (unsigned) bh::axis::option::none;
    opt.attr("underflow") = (unsigned) bh::axis::option::underflow;
    opt.attr("overflow") =  (unsigned) bh::axis::option::overflow;
    opt.attr("circular") =  (unsigned) bh::axis::option::circular;
    opt.attr("growth") =    (unsigned) bh::axis::option::growth;
    
    // This factory makes a class that can be used to create axes and also be used in is_instance
    py::object factory_meta_py = py::module::import("boost.histogram_utils").attr("FactoryMeta");
    
    register_axis_by_type<axis::regular_uoflow>(ax, "regular_uoflow", "Evenly spaced bins")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    ;

    register_axis_iv_by_type<axis::regular_uoflow>(ax, "_regular_internal_view");


    register_axis_by_type<axis::regular_noflow>(ax, "regular_noflow", "Evenly spaced bins without over/under flow")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::regular_noflow>(ax, "_regular_noflow_internal_view");


    register_axis_by_type<axis::regular_growth>(ax, "regular_growth", "Evenly spaced bins that grow as needed")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::regular_growth>(ax, "_regular_growth_internal_view");

    ax.def("make_regular",
           [](unsigned n, double start, double stop, metadata_t metadata, bool flow, bool growth) -> py::object {
               if(growth) {
                   return py::cast(axis::regular_growth(n, start, stop, metadata), py::return_value_policy::move);
               } else if(flow) {
                   return py::cast(axis::regular_uoflow(n, start, stop, metadata), py::return_value_policy::move);
               } else  {
                   return py::cast(axis::regular_noflow(n, start, stop, metadata), py::return_value_policy::move);
               }
           },
           "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str(), "flow"_a = true, "growth"_a = false,
           "Make a regular axis with nice keyword arguments for flow and growth");
    
    ax.attr("regular") = factory_meta_py(ax.attr("make_regular"), py::make_tuple(ax.attr("regular_uoflow"),
                                                                                 ax.attr("regular_noflow"),
                                                                                 ax.attr("regular_growth")));

    register_axis_by_type<axis::circular>(ax, "circular", "Evenly spaced bins with wraparound")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    .def(py::init([](unsigned n, double stop, metadata_t metadata){
        return new axis::circular{n, 0.0, stop, metadata};
    }), "n"_a, "stop"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::circular>(ax, "_circular_internal_view");


    register_axis_by_type<axis::regular_log>(ax, "regular_log", "Evenly spaced bins in log10")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::regular_log>(ax, "_regular_log_internal_view");


    register_axis_by_type<axis::regular_sqrt>(ax, "regular_sqrt", "Evenly spaced bins in sqrt")
    .def(py::init<unsigned, double, double, metadata_t>(), "n"_a, "start"_a, "stop"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::regular_sqrt>(ax, "_regular_sqrt_internal_view");


    register_axis_by_type<axis::regular_pow>(ax, "regular_pow", "Evenly spaced bins in a power")
    .def(py::init([](unsigned n, double start, double stop, double pow, metadata_t metadata){
        return new axis::regular_pow(bh::axis::transform::pow{pow}, n, start, stop, metadata);} ),
         "n"_a, "start"_a, "stop"_a, "power"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::regular_pow>(ax, "_regular_pow_internal_view");


    register_axis_by_type<axis::variable>(ax, "variable", "Unevenly spaced bins")
    .def(py::init<std::vector<double>, metadata_t>(), "edges"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::variable>(ax, "_variable_internal_view");


    register_axis_by_type<axis::integer_uoflow>(ax, "integer_uoflow", "Contigious integers")
    .def(py::init<int, int, metadata_t>(), "min"_a, "max"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::integer_uoflow>(ax, "_integer_internal_view");

    register_axis_by_type<axis::integer_noflow>(ax, "integer_noflow", "Contigious integers with no under/overflow")
    .def(py::init<int, int, metadata_t>(), "min"_a, "max"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::integer_noflow>(ax, "_integer_noflow_internal_view");

    register_axis_by_type<axis::integer_growth>(ax, "integer_growth", "Contigious integers with growth")
    .def(py::init<int, int, metadata_t>(), "min"_a, "max"_a, "metadata"_a = py::str())
    ;
    register_axis_iv_by_type<axis::integer_growth>(ax, "_integer_integer_growth_internal_view");

    register_axis_by_type<axis::category_int>(ax, "category_int", "Text label bins")
    .def(py::init<std::vector<int>, metadata_t>(), "labels"_a, "metadata"_a = py::str())
    ;


    register_axis_by_type<axis::category_int_growth>(ax, "category_int_growth", "Text label bins")
    .def(py::init<std::vector<int>, metadata_t>(), "labels"_a, "metadata"_a = py::str())
    .def(py::init<>())
    ;


    register_axis_by_type<axis::category_str>(ax, "category_str", "Text label bins")
    .def(py::init<std::vector<std::string>, metadata_t>(), "labels"_a, "metadata"_a = py::str())
    ;


    register_axis_by_type<axis::category_str_growth>(ax, "category_str_growth", "Text label bins")
    .def(py::init<std::vector<std::string>, metadata_t>(), "labels"_a, "metadata"_a = py::str())
    // Add way to allow empty list of strings
    .def(py::init<>())
    ;

}
