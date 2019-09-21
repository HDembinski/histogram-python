// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#pragma once

#include <boost/histogram/python/pybind11.hpp>

#include <boost/histogram/axis.hpp>

#include <tuple>
#include <type_traits>
#include <vector>

/// Register bh::axis::variant as a variant for PyBind11
namespace pybind11 {
namespace detail {
template <class... Ts>
struct type_caster<bh::axis::variant<Ts...>>
    : variant_caster<bh::axis::variant<Ts...>> {};
} // namespace detail
} // namespace pybind11

inline bool PyObject_Check(void *value) { return value != nullptr; }

class metadata_t : public py::object {
    PYBIND11_OBJECT_DEFAULT(metadata_t, object, PyObject_Check);

    bool operator==(const metadata_t &other) const {
        return py::cast<bool>(this->attr("__eq__")(other));
    }

    bool operator!=(const metadata_t &other) const {
        return py::cast<bool>(this->attr("__ne__")(other));
    }
};

namespace axis {
// this or something similar should move to boost::histogram::axis::traits
template <class Axis>
using get_value_type = std::decay_t<decltype(std::declval<Axis>().value(0))>;

// this or something similar should move to boost::histogram::axis::traits
template <class Axis>
using is_continuous =
    typename std::is_same<get_value_type<Axis>, bh::axis::real_index_type>::type;

/// Utility to convert an axis to edges array
template <class A>
py::array_t<double> to_edges(const A &ax, bool flow) {
    const bh::axis::index_type underflow
        = flow && (bh::axis::traits::options(self) & bh::axis::option::underflow);
    const bh::axis::index_type overflow
        = flow && (bh::axis::traits::options(self) & bh::axis::option::overflow);

    py::array_t<double> edges(
        static_cast<std::size_t>(ax.size() + 1 + overflow + underflow));

    for(bh::axis::index_type i = -underflow; i <= ax.size() + overflow; ++i)
        edges.mutable_at(i + underflow) = ax.value(i);

    return edges;
}

template <class A>
auto bin(const A &ax, bh::axis::index_type i) {
    return bh::detail::static_if<
        std::is_same<axis::get_value_type<A>, bh::axis::index_type>>(
        [i](const auto &ax) { // is discrete
            return py::cast(ax[i]);
        },
        [i](const auto &ax) { // is continuous
            return py::make_tuple(ax.lower(), ax.upper());
        },
        ax);
}

template <class A>
auto to_values(const A &ax, bool flow) {
    static_assert(is_continuous<A>::value == false, "");

    const bh::axis::index_type underflow
        = flow && (bh::axis::traits::options(self) & bh::axis::option::underflow);
    const bh::axis::index_type overflow
        = flow && (bh::axis::traits::options(self) & bh::axis::option::overflow);

    py::array_t<get_value_type<A>> result(
        static_cast<std::size_t>(self.size() + 1 + overflow + underflow));

    for(auto i = -underflow; i < self.size() + overflow; i++)
        result[static_cast<std::size_t>(i + underflow)] = ax.value(i);

    return result;
}

template <class... Ts>
auto to_values(const bh::axis::category<std::string, Ts...> &ax, bool flow) {
    const auto n = max_string_length(self);
    py::array result(py::dtype(bh::detail::cat("S", n)), self.size());

    std::size_t i = 0;
    for(auto &&s : self) {
        auto sout = static_cast<char *>(result.mutable_data(i++));
        std::copy(s.begin(), s.end(), sout);
        if(s.size() < n)
            sout[s.size()] = 0;
    }

    return result;
}

template <class A>
auto to_centers(const A &ax) {
    static_assert(is_continuous<A>::value, "");

    py::array_t<value_type> result(static_cast<std::size_t>(ax.size()));

    std::transform(ax.begin(), ax.end(), result.mutable_data(), [](const auto &b) {
        return b.center();
    });

    return result;
}

// These match the Python names

using _regular_uoflow = bh::axis::regular<double, bh::use_default, metadata_t>;
using _regular_uflow  = bh::axis::
    regular<double, bh::use_default, metadata_t, bh::axis::option::underflow_t>;
using _regular_oflow = bh::axis::
    regular<double, bh::use_default, metadata_t, bh::axis::option::overflow_t>;
using _regular_noflow
    = bh::axis::regular<double, bh::use_default, metadata_t, bh::axis::option::none_t>;
using _regular_growth = bh::axis::
    regular<double, bh::use_default, metadata_t, bh::axis::option::growth_t>;

using circular     = bh::axis::circular<double, metadata_t>;
using regular_log  = bh::axis::regular<double, bh::axis::transform::log, metadata_t>;
using regular_sqrt = bh::axis::regular<double, bh::axis::transform::sqrt, metadata_t>;
using regular_pow  = bh::axis::regular<double, bh::axis::transform::pow, metadata_t>;

using _variable_uoflow = bh::axis::variable<double, metadata_t>;
using _variable_uflow
    = bh::axis::variable<double, metadata_t, bh::axis::option::underflow_t>;
using _variable_oflow
    = bh::axis::variable<double, metadata_t, bh::axis::option::overflow_t>;
using _variable_noflow
    = bh::axis::variable<double, metadata_t, bh::axis::option::none_t>;

using _integer_uoflow = bh::axis::integer<int, metadata_t>;
using _integer_uflow
    = bh::axis::integer<int, metadata_t, bh::axis::option::underflow_t>;
using _integer_oflow = bh::axis::integer<int, metadata_t, bh::axis::option::overflow_t>;
using _integer_noflow = bh::axis::integer<int, metadata_t, bh::axis::option::none_t>;
using _integer_growth = bh::axis::integer<int, metadata_t, bh::axis::option::growth_t>;

using _category_int = bh::axis::category<int, metadata_t>;
using _category_int_growth
    = bh::axis::category<int, metadata_t, bh::axis::option::growth_t>;

using _category_str = bh::axis::category<std::string, metadata_t>;
using _category_str_growth
    = bh::axis::category<std::string, metadata_t, bh::axis::option::growth_t>;

} // namespace axis

// The following list is all types supported
using axis_variant = bh::axis::variant<axis::_regular_uoflow,
                                       axis::_regular_uflow,
                                       axis::_regular_oflow,
                                       axis::_regular_noflow,
                                       axis::_regular_growth,
                                       axis::circular,
                                       axis::regular_log,
                                       axis::regular_pow,
                                       axis::regular_sqrt,
                                       axis::_variable_uoflow,
                                       axis::_variable_oflow,
                                       axis::_variable_uflow,
                                       axis::_variable_noflow,
                                       axis::_integer_uoflow,
                                       axis::_integer_oflow,
                                       axis::_integer_uflow,
                                       axis::_integer_noflow,
                                       axis::_integer_growth,
                                       axis::_category_int,
                                       axis::_category_int_growth,
                                       axis::_category_str,
                                       axis::_category_str_growth>;

// This saves a little typing
using vector_axis_variant = std::vector<axis_variant>;
