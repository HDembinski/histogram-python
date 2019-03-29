// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <functional>
#include <type_traits>

namespace py = pybind11;
using namespace pybind11::literals; // For ""_a syntax

namespace boost { namespace histogram {}}
namespace bh = boost::histogram;

/// Static if standin: define a method if expression is true
template<typename T, typename... Args>
void def_optionally(T&& module, std::true_type, Args&&... expression) {
    module.def(std::forward<Args...>(expression...));
}

/// Static if standin: Do nothing if compile time expression is false
template<typename T, typename... Args>
void def_optionally(T&&, std::false_type, Args&&...) {}
