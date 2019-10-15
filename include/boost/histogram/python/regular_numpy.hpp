// Copyright 2018-2019 Henry Schreiner and Hans Dembinski
//
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/scikit-hep/boost-histogram for details.

#pragma once

#include <boost/histogram/python/pybind11.hpp>

#include <boost/histogram/axis/regular.hpp>
#include <boost/histogram/python/axis_setup.hpp>
#include <boost/histogram/python/serializion.hpp>

namespace bh = boost::histogram;

namespace axis {

/// Mimics the numpy behavoir exactly.
class regular_numpy : public bh::axis::regular<double, bh::use_default, metadata_t> {
    using value_type = double;
    double stop_;

  public:
    regular_numpy(unsigned n, value_type start, value_type stop, metadata_t meta = {})
        : regular(n, start, stop, meta)
        , stop_(stop) {}

    regular_numpy()
        : regular()
        , stop_(0){};

    boost::histogram::axis::index_type index(value_type v) const {
        return v <= stop_ ? std::min(regular::index(v), size() - 1) : regular::index(v);
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned version) {
        static_cast<bh::axis::regular<double, bh::use_default, metadata_t> *>(this)
            ->serialize(ar, version);
        ar &serialization::make_nvp("stop", stop_);
    }
};

} // namespace axis
