// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#pragma once

#include <boost/histogram/python/pybind11.hpp>

#include <boost/histogram/axis/traits.hpp>
#include <boost/histogram/detail/cat.hpp>
#include <boost/histogram/detail/iterator_adaptor.hpp>
#include <boost/histogram/python/axis.hpp>
#include <boost/histogram/python/axis_ostream.hpp>
#include <boost/histogram/python/bin_setup.hpp>
#include <boost/histogram/python/serializion.hpp>

#include <algorithm>
#include <iostream>
#include <pybind11/eval.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std::literals;

struct options {
    unsigned option;
    bool none() const { return option == bh::axis::option::none; }
    bool underflow() const {
        return static_cast<bool>(option & bh::axis::option::underflow);
    }
    bool overflow() const {
        return static_cast<bool>(option & bh::axis::option::overflow);
    }
    bool circular() const {
        return static_cast<bool>(option & bh::axis::option::circular);
    }
    bool growth() const { return static_cast<bool>(option & bh::axis::option::growth); }
};

/// Ensure metadata is valid.
inline void validate_metadata(metadata_t v) {
    try {
        py::cast<double>(v);
        throw py::type_error(
            "Numeric types not allowed for metatdata in the the constructor. Use "
            ".metadata after "
            "constructing instead if you *really* need numeric metadata.");
    } catch(const py::cast_error &) {
    }
}

/// Add a constructor for an axes, with smart handling for the metadata (will not allow
/// numeric types)"
template <class T, class... Args>
decltype(auto) construct_axes() {
    return py::init([](Args... args, metadata_t v) {
        // Check for numeric v here
        validate_metadata(v);
        return new T{std::forward<Args>(args)..., v};
    });
}

template <class A>
void vectorized_index_and_value_methods(py::class_<A> &axis) {
    axis.def("index",
             py::vectorize(&A::index),
             "Index for value (or values) on the axis",
             "x"_a)
        .def("value", py::vectorize(&A::value), "Value at index (or indices)", "i"_a);
}

template <class... Ts>
void vectorized_index_and_value_methods(
    py::class_<bh::axis::category<int, Ts...>> &axis) {
    using axis_t = bh::axis::category<int, Ts...>;
    axis.def("index",
             py::vectorize([](axis_t &self, int v) { return int(self.index(v)); }),
             "Index for value (or values) on the axis",
             "x"_a)
        .def("value",
             py::vectorize([](axis_t &self, int i) { return int(self.value(i)); }),
             "Value at index (or indices)",
             "i"_a);
}

template <class... Ts>
void vectorized_index_and_value_methods(
    py::class_<bh::axis::category<std::string, Ts...>> &axis) {
    using axis_t = bh::axis::category<std::string, Ts...>;
    axis.def(
            "index",
            [](axis_t &self, py::object arg) -> py::object {
                if(py::isinstance<py::str>(arg))
                    return py::cast(self.index(py::cast<std::string>(arg)));

                auto values = py::cast<py::array>(arg);

                std::vector<ssize_t> strides;
                strides.reserve(static_cast<std::size_t>(values.ndim()));
                for(unsigned i = 0; i < values.ndim(); ++i)
                    strides.push_back(values.strides()[i] / values.dtype().itemsize()
                                      * static_cast<ssize_t>(sizeof(int)));

                py::array_t<int> indices(
                    bh::detail::span<const ssize_t>(values.shape(),
                                                    values.shape() + values.ndim()),
                    strides);

                const auto itemsize = values.itemsize();

                switch(values.dtype().kind()) {
                case 'S': {
                    auto pvalues           = static_cast<const char *>(values.data());
                    const auto pvalues_end = pvalues + itemsize * values.size();
                    auto pindices          = indices.mutable_data();
                    for(; pvalues != pvalues_end; pvalues += itemsize) {
                        auto pend = pvalues;
                        for(unsigned i = 0; i < itemsize && *pend; ++pend, ++i)
                            ;
                        *pindices++ = self.index(std::string(pvalues, pend));
                    }
                } break;
                case 'U': {
                    // numpy seems to use utf32 encoding
                    if(itemsize % 4 != 0)
                        throw std::invalid_argument(
                            "itemsize for unicode array is not multiple of 4");
                    const auto nmax        = itemsize / 4;
                    auto pvalues           = static_cast<const char *>(values.data());
                    const auto pvalues_end = pvalues + itemsize * values.size();
                    auto pindices          = indices.mutable_data();
                    for(; pvalues != pvalues_end; pvalues += itemsize) {
                        auto pend  = pvalues;
                        unsigned n = 0;
                        while(n < nmax && *pend) {
                            if(*pend >= 128)
                                throw std::invalid_argument(
                                    "only ASCII subset of unicode is allowed");
                            ++n;
                            pend += 4;
                        }
                        std::string s;
                        s.reserve(n);
                        for(auto p = pvalues; p != pend; p += 4)
                            s.push_back(*p);
                        *pindices++ = self.index(s);
                    }
                } break;
                case 'O':
                    throw std::runtime_error("not implemented yet");
                default:
                    throw std::invalid_argument(
                        "argument must be string or sequence of strings");
                }

                return std::move(indices);
            },
            "Index for value (or values) on the axis",
            "x"_a)
        .def(
            "value",
            [](axis_t &self, py::array_t<int> indices) {
                const ssize_t itemsize
                    = (static_cast<ssize_t>(detail::max_string_length(self)) + 1) * 4;
                // to-do: return object array, since strings are highly redundant
                std::vector<ssize_t> strides;
                strides.reserve(static_cast<std::size_t>(indices.ndim()));
                for(unsigned i = 0; i < indices.ndim(); ++i)
                    strides.push_back(indices.strides()[i]
                                      / static_cast<ssize_t>(sizeof(int)) * itemsize);
                py::array values(py::dtype(bh::detail::cat("U", itemsize / 4)),
                                 bh::detail::span<const ssize_t>(
                                     indices.shape(), indices.shape() + indices.ndim()),
                                 strides);
                if(values.dtype().itemsize() != itemsize)
                    throw std::invalid_argument(
                        "itemsize of unicode array is not multiple of 4");
                auto pindices           = indices.data();
                const auto pindices_end = pindices + indices.size();
                auto pvalues            = static_cast<char *>(values.mutable_data());
                for(; pindices != pindices_end; ++pindices, pvalues += itemsize) {
                    auto ps = pvalues;
                    for(auto ch : self.value(*pindices)) {
                        if(ch >= 128)
                            throw std::invalid_argument(
                                "only ASCII subset of unicode is allowed");
                        *ps++ = ch;
                        *ps++ = 0;
                        *ps++ = 0;
                        *ps++ = 0;
                    }
                    *ps++ = 0;
                    *ps++ = 0;
                    *ps++ = 0;
                    *ps++ = 0;
                }
                return values;
            },
            "Value at index (or indices)",
            "i"_a);
}

/// Add helpers common to all axis types
template <class A, class... Args>
py::class_<A> register_axis(py::module &m, const char *name, Args &&... args) {
    py::class_<A> ax(m, name, std::forward<Args>(args)...);

    ax.def("__repr__", &shift_to_string<A>)

        .def(py::self == py::self)
        .def(py::self != py::self)

        .def("update", &A::update, "i"_a, "Bin and add a value if allowed")
        .def_property_readonly(
            "options",
            [](const A &self) {
                return options{static_cast<unsigned>(self.options())};
            },
            "Return the options associated to the axis")
        .def_property(
            "metadata",
            [](const A &self) { return self.metadata(); },
            [](A &self, const metadata_t &label) { self.metadata() = label; },
            "Set the axis label")

        .def_property_readonly(
            "extent",
            &bh::axis::traits::extent<A>,
            "Returns the number of bins including over- or underflow")
        .def_property_readonly(
            "size", &A::size, "Return number of bins excluding over- or underflow")

        .def("bin", &axis::bin<A>, "i"_a, "Return bin at index i")

        .def("__copy__", [](const A &self) { return A(self); })
        .def("__deepcopy__",
             [](const A &self, py::object memo) {
                 A *a            = new A(self);
                 py::module copy = py::module::import("copy");
                 a->metadata()   = copy.attr("deepcopy")(a->metadata(), memo);
                 return a;
             })

        .def(
            "__iter__",
            [](A &ax) {
                struct iterator
                    : bh::detail::iterator_adaptor<iterator, int, py::object> {
                    const A &axis_;
                    iterator(const A &axis, int idx)
                        : iterator::iterator_adaptor_(idx)
                        , axis_(axis) {}

                    auto operator*() const { return axis::bin<A>(axis_, this->base()); }
                };

                iterator begin(ax, 0), end(ax, ax.size());
                return py::make_iterator(begin, end);
            },
            py::keep_alive<0, 1>())

        ;

    bh::detail::static_if<axis::is_continuous<A>>(
        [](auto &ax) {
            // for continuous axis with bins that represent intervals
            using axis_t = boost::mp11::mp_first<std::decay_t<decltype(ax)>>;
            ax.def("edges",
                   &axis::to_edges<axis_t>,
                   "flow"_a = false,
                   "Bin edges (length: len(axis) + 1) (include over/underflow if "
                   "flow=True)")
                .def("centers", &axis::to_centers<axis_t>, "Return the bin centers");
        },
        [](auto &ax) {
            // for discrete axis with bins that represent values
            using axis_t = boost::mp11::mp_first<std::decay_t<decltype(ax)>>;
            ax.def("values",
                   &axis::to_values<axis_t>,
                   "flow"_a = false,
                   "Return the bin values");
        },
        ax);

    vectorized_index_and_value_methods(ax);

    ax.def(make_pickle<A>());

    return ax;
}
