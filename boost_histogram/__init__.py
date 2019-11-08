from __future__ import absolute_import, division, print_function

# Sadly, some tools (IPython) do not respect __all__
# as a list of public items in a module. So we need
# to delete / hide any extra items manually.
del absolute_import, division, print_function

from ._internal.hist import Histogram
from . import axis, storage, accumulators, algorithm, numpy
from .tag import loc, rebin, sum, underflow, overflow
from .version import __version__

Histogram.__module__ = "boost_histogram"

from .version import __version__

__all__ = (
    "Histogram",
    "axis",
    "storage",
    "accumulators",
    "algorithm",
    "numpy",
    "loc",
    "rebin",
    "sum",
    "underflow",
    "overflow",
    "__version__",
)

# Workarounds for smooth transitions from 0.5 series. Will be removed after 0.6.

from .tag import project


class histogram(Histogram):
    __slots__ = ()

    def __init__(self, *args, **kwargs):
        import warnings

        warnings.warn("Use Histogram instead", DeprecationWarning)
        super(histogram, self).__init__(*args, **kwargs)
