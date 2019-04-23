# boost-histogram for Python

[![Gitter][gitter-badge]][gitter-link]
[![Build Status][azure-badge]][azure-link]
[![Documentation Status][rtd-badge]][rtd-link]

Python bindings for [Boost::Histogram][] ([source][Boost::Histogram source]), a C++14 library. This should become one of the [fastest libraries][] for histogramming, while still providing the power of a full histogram object.

> ## Warning: These bindings are in progress and are not yet in a beta stage.
>
> Join the [discussion on gitter][gitter-link] to follow the development!



## Installation

This library is under development, but you can install directly from github if you would like. You need a C++14 compiler and Python 2.7--3.7. Boost 1.70 is not required or needed (this only depends on included header-only dependencies).
All the normal best-practices for Python apply; you should be in a virtual environment, otherwise add `--user`, etc.

```bash
python -m pip install git+https://github.com/scikit-hep/boost-histogram.git@develop
```

## Usage

This is a suggested example of usage.

```python
import boost.histogram as bh

# Compose axis however you like
hist = bh.histogram(bh.axis.regular(2, 0, 1),
                    bh.axis.regular(4, 0.0, 1.0))

# Filling can be done with arrays, one per dimension
hist.fill([.3, .5, .2],
          [.1, .4, .9])

# Numpy array view into histogram counts, no overflow bins
counts = hist.view()
```

## Features

* Many axis types (all support `metadata=...`)
    * `bh.axis.regular(n, start, stop, underflow=True, overflow=True, growth=False)`: shortcut to make the types below. `flow=False` is also supported.
        * `bh.axis.regular_uoflow(n, start, stop)`: `n` evenly spaced bins from `start` to `stop`
        * `bh.axis.regular_uflow(n, start, stop)`: `n` evenly spaced bins from `start` to `stop` (no overflow)
        * `bh.axis.regular_oflow(n, start, stop)`: `n` evenly spaced bins from `start` to `stop` (no underflow)
        * `bh.axis.regular_noflow(n, start, stop)`: `regular` but with no underflow or overflow bins
        * `bh.axis.regular_growth(n, start, stop)`: `regular` but grows if a value is added outside the range
    * `bh.axis.circular(n, start, stop)`: Value outside the range wrap into the range
    * `bh.axis.regular_log(n, start, stop)`: Regularly spaced values in log 10 scale
    * `bh.axis.regular_sqrt(n, start, stop)`: Regularly spaced value in sqrt scale
    * `bh.axis.regular_pow(n, start, stop, power)`: Regularly spaced value to some `power`
    * `bh.axis.integer(start, stop, underflow=True, overflow=True, growth=False)`: Special high-speed version of `regular` for evenly spaced bins of width 1
        * `bh.axis.integer_uoflow(start, stop)`: `n` evenly spaced bins from `start` to `stop`
        * `bh.axis.integer_uflow(start, stop)`: `n` evenly spaced bins from `start` to `stop` (no overflow)
        * `bh.axis.integer_oflow(start, stop)`: `n` evenly spaced bins from `start` to `stop` (no underflow)
        * `bh.axis.integer_noflow(start, stop)`: `integer` but with no underflow or overflow bins
        * `bh.axis.integer_growth(start, stop)`: `integer` but grows if a value is added outside the range
    * `bh.axis.variable([start, edge1, edge2, ..., stop], underflow=True, overflow=True)`: Uneven bin spacing
        * `bh.axis.variable_uoflow([start, edge1, edge2, ..., stop])`: `n` evenly spaced bins from `start` to `stop`
        * `bh.axis.variable_uflow([start, edge1, edge2, ..., stop])`: `n` evenly spaced bins from `start` to `stop` (no overflow)
        * `bh.axis.variable_oflow([start, edge1, edge2, ..., stop])`: `n` evenly spaced bins from `start` to `stop` (no underflow)
        * `bh.axis.variable_noflow([start, edge1, edge2, ..., stop])`: `variable` but with no underflow or overflow bins
    * `bh.axis.category([...], growth=False)`: Integer or string categories
        * `bh.axis.category_int([1, 2, ...])`: Integer bins
        * `bh.axis.category_int_growth([1, 2, ...])`: Integer bins where new items are added automatically
        * `bh.axis.category_str(["item1", "item2", ...])`: String bins
        * `bh.axis.category_str_growth(["item1", "item2", ...])`: String bins where new items are added automatically
* Axis features:
    * `.bin(i)`: The bin or a bin view for continuous axis types
        * `.lower()`: The lower value
        * `.upper()`: The upper value
        * `.center()`: The center value
        * `.width()`: The bin width
    * `.bins()`: A list of bins or bin views
    * `.size()`: The number of bins (not including under/overflow)
    * `.size(flow=True)`: The number of bins (including under/overflow)
    * `.options()`: The options set on the axis (`bh.axis.options` bitfields)
    * `.edges(flow=False)`: The N+1 bin edges (if continuous)
    * `.centers(flow=False)`: The N bin centers (if continuous)
    * `.update(value)`: Grow an axis to include `value` if needed and able
    * `.index(values)`: The index at a point (or points) on the axis
    * `.value(index)`: The value for a fractional bin in the axis
* Many storage types
    * `bh.storage.int`: 64 bit unsigned integers for high performance and useful view access
    * `bh.storage.double`: Doubles for weighted values
    * `bh.storage.unlimited`: Starts small, but can go up to unlimited precision ints or doubles.
    * `bh.storage.atomic_int`: Threadsafe filling, for higher performance on multhreaded backends. Does not support growing axis in threads.
    * `bh.storage.weight`: WIP
    * `bh.storage.profile`: WIP
    * `bh.storage.weighted_profile`: WIP
* Accumulators
    * `bh.accumulator.weighted_sum`: Tracks a weighted sum and variance
    * `bh.accumulator.weighted_mean`: Tracks a weighted sum, mean, and variance (West's incremental algorithm)
    * `bh.accumulator.sum`: High accuracy sum (Neumaier)
    * `bh.accumulator.mean`: Running count, mean, and variance (Welfords's incremental algorithm)
* Histogram operations
    * `(a, b, ...)`: Fill with arrays or single values
    * `+`: Add two histograms
    * `.rank()`: The number of dimensions
    * `.size()`: The number of bins (include under/overflow bins)
    * `.reset()`: Set counters to 0
    * `*=`: Multiply by a scaler (not all storages) (`hist * scalar` and `scalar * hist` supported too)
    * `/=`: Divide by a scaler (not all storages) (`hist / scalar` supported too)
    * `.to_numpy(flow=False)`: Convert to a numpy style tuple (with or without under/overflow bins)
    * `.view(flow=False)`: Get a view on the bin contents (with or without under/overflow bins)
    * `np.asarray(...)`: Get a view on the bin contents with under/overflow bins
    * `.axis(i)`: Get the `i`th axis
    * `.at(i, j, ...)`: Get the bin contents as a location 
    * `.sum()`: The total count of all bins
    * `.project(ax1, ax2, ...)` Project down to listed axis (numbers)
* Details
    * Use `bh.histogram(..., storage=...)` to make a histogram (there are several different types) 
    * Several common combinations are optimized, such as regular axes + int storage
    * Regular axis can be made in `bh.histogram` using a tuple
    * Variable axis can be made in `bh.histogram` using a list


## Supported platforms

#### Binaries available:

The easiest way to get boost-histogram is to use a binary wheel. These are the supported platforms for which wheels are produced:

| System | Arch | Python versions |
|---------|-----|------------------|
| ManyLinux1 (custom GCC 8.3) | 64 & 32-bit | 2.7, 3.5, 3.6, 3.7 |
| ManyLinux2010 | 64-bit | 2.7, 3.5, 3.6, 3.7 |
| macOS 10.9+ | 64-bit | 2.7, 3.6, 3.7 |
| Windows | 64 & 32-bit | 2.7, 3.6, 3.7 |


* Linux: I'm not supporting 3.4 because I have to build the Numpy wheels to do so.
* manylinux1: Using a custom docker container with GCC 8.3; should work but can't be called directly other compiled extensions unless they do the same thing (think that's the main caveat). Supporting 32 bits because it's there.
* manylinux2010: Requires pip 10+ and a version of Linux newer than 2010. This is very new technology.
* MacOS: Using the dedicated 64 bit 10.9+ Python.org builds. Not supporting 3.5 because those no longer provide binaries (could add a 32+64 fat 10.6+ that really was 10.9+, but not worth it unless there is a need for it).
* Windows: PyBind11 is able to compile with a newer copy of Visual Studio than Python 2.7's Visual Studio 2008; you might need to have the Visual Studio 2017 distributable though.

If you are on a Linux system that is not part of the "many" in manylinux, such as Alpine or ClearLinux, building from source is usually fine, since the compilers on those systems are often quite new. It will just take a little longer to install when it's using the sdist instead of a wheel.

Conda support is planned.

#### Source builds

For a source build, for example from an "sdist" package, the only requirements are a C++14 compatible compiler. If you are using Python 2.7 on Windows, you will need to use a recent version of Visual studio and force distutils to use it, or just upgrade to Python 3.6 or newer. Check the PyBind11 documentation for [more help](https://pybind11.readthedocs.io/en/stable/faq.html#working-with-ancient-visual-studio-2009-builds-on-windows). On some Linux systems, you may need to use a newer compiler than the one your distribution ships with. 

## Developing

This repository has dependencies in submodules. Check out the repository like this:

```bash
git clone --recursive https://github.com/scikit-hep/boost-histogram.git
cd boost-histogram
```


<details><summary>Faster version (click to expand)</summary>

```bash
git clone https://github.com/scikit-hep/boost-histogram.git
cd boost-histogram
git submodule update --init --depth 10
```

</details>

Make a build directory and run CMake. If you have a specific Python you want to use, add `-DPYTHON_EXECUTABLE=$(which python)` or similar to the CMake line.

```bash
mkdir build
cd build
cmake ..
make -j4
```

Run the unit tests (requires pytest and numpy). Use `ctest` or `make test`, like this:

```bash
make test
# or
python -m pytest
```

The tests require `numpy`, `pytest`, and `pytest-benchmark`. If you are using Python 2, you will need `futures` as well.

You can enable benchmarking with `--benchmark-enable`. You can also run explicit performance tests with `scripts/performance_report.py`.

To install using the pip method for development instead, run:

```bash
python3 -m venv .env
. .env/bin/activate
python -m pip install .[test]
```

You'll need to reinstall it if you want to rebuild.

<details><summary>Updating dependencies (click to expand)</summary>

This will checkout new versions of the dependencies. Example given using the fish shell.

```fish
for f in *
    cd $f
    git fetch
    git checkout boost-1.70.0 || echo "Not found"
    cd ..
end
```

</details>

## Talks and other documentation/tutorial sources

* [2019-4-15 IRIS-HEP Topical meeting](https://indico.cern.ch/event/803122/)

[gitter-badge]:            https://badges.gitter.im/HSF/PyHEP-histogramming.svg
[gitter-link]:             https://gitter.im/HSF/PyHEP-histogramming?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge
[azure-badge]:             https://dev.azure.com/scikit-hep/boost-histogram/_apis/build/status/scikit-hep.boost-histogram?branchName=develop
[azure-link]:              https://dev.azure.com/scikit-hep/boost-histogram/_build/latest?definitionId=2&branchName=develop
[rtd-badge]:               https://readthedocs.org/projects/boost-histogram/badge/?version=latest
[rtd-link]:                https://boost-histogram.readthedocs.io/en/latest/?badge=latest

[Boost::Histogram]:        https://www.boost.org/doc/libs/1_70_0/libs/histogram/doc/html/index.html
[Boost::Histogram source]: https://github.com/boostorg/histogram
[fastest libraries]:       https://iscinumpy.gitlab.io/post/histogram-speeds-in-python/
