// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#include "pybind11.hpp"

#include "axis.hpp"
#include "histogram.hpp"
#include "register_histogram.hpp"
#include "storage.hpp"
#include <boost/histogram/storage_adaptor.hpp>

void register_histograms(py::module& hist) {
    hist.attr("_axes_limit") = BOOST_HISTOGRAM_DETAIL_AXES_LIMIT;

    register_histogram<storage::int_>(
        hist,
        "any_int",
        "N-dimensional histogram for unlimited size data with any axis types.");

    register_histogram<storage::unlimited>(
        hist,
        "any_unlimited",
        "N-dimensional histogram for unlimited size data with any axis types.");

    register_histogram<storage::double_>(
        hist,
        "any_double",
        "N-dimensional histogram for real-valued data with weights with any axis "
        "types.");

    register_histogram<storage::atomic_int>(
        hist,
        "any_atomic_int",
        "N-dimensional histogram for threadsafe integer data with any axis types.");

    register_histogram<storage::weight>(
        hist,
        "any_weight",
        "N-dimensional histogram for weighted data with any axis types.");

    register_histogram<storage::mean>(
        hist,
        "any_mean",
        "N-dimensional histogram for sampled data with any axis types.");

    register_histogram<storage::weighted_mean>(
        hist,
        "any_weighted_mean",
        "N-dimensional histogram for weighted and sampled data with any axis types.");
}
