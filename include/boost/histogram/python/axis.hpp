// Copyright 2015-2018 Hans Dembinski and Henry Schreiner
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/histogram.hpp>

#include <tuple>
#include <vector>

namespace bh = boost::histogram;

// Register bh::axis::Variant as a variant for PyBind11
namespace pybind11 { namespace detail {
    template <typename... Ts>
    struct type_caster<bh::axis::variant<Ts...>> : variant_caster<bh::axis::variant<Ts...>> {};
}} // namespace pybind11::detail


namespace axis {

// These match the Python names
using regular = bh::axis::regular<>;
using regular_noflow = bh::axis::regular<double, bh::use_default, bh::use_default, bh::axis::option::none_t>;
using circular = bh::axis::circular<>;
using regular_log = bh::axis::regular<double, bh::axis::transform::log>;
using regular_sqrt = bh::axis::regular<double, bh::axis::transform::sqrt>;
using regular_pow = bh::axis::regular<double, bh::axis::transform::pow>;
    
using variable = bh::axis::variable<double>;

} // namespace axis

namespace axes {

// The following list is all types supported
using any = std::vector<bh::axis::variant<axis::regular,
                                          axis::regular_noflow,
                                          axis::circular,
                                          axis::regular_log,
                                          axis::regular_pow,
                                          axis::regular_sqrt,
                                          axis::variable>>;

// Specialization for some speed improvement
using regular = std::vector<axis::regular>;

// Specialization for some speed improvement
using regular_noflow = std::vector<axis::regular_noflow>;
    
// Specializations for maximum speed!
using regular_1D = std::tuple<axis::regular>;
using regular_2D = std::tuple<axis::regular, axis::regular>;
    
using regular_noflow_1D = std::tuple<axis::regular_noflow>;
using regular_noflow_2D = std::tuple<axis::regular_noflow, axis::regular_noflow>;
    
    
} // namespace axes
