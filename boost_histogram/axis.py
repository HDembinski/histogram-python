from __future__ import absolute_import, division, print_function

del absolute_import, division, print_function

from ._core.axis import options

from ._core import axis as ca

from ._utils import KWArgs as _KWArgs


class Axis(object):
    __slots__ = ("_ax",)

    def index(self, x):
        """
        Index for value (or values) on the axis.
        """

        return self._ax.index(x)

    def value(self, i):
        """
        Value at index (or indices).
        """

        return self._ax.value(i)

    def __repr__(self):
        return repr(self._ax)

    def __eq__(self, other):
        return self._ax == other._ax

    def __ne__(self, other):
        return self._ax != other._ax

    @property
    def options(self):
        return self._ax.options

    @property
    def metadata(self):
        return self._ax.metadata

    @metadata.setter
    def metadata(self, value):
        self._ax.metadata = value

    @property
    def size(self):
        """
        Return number of bins excluding under- and overflow
        """
        return self._ax.size

    @property
    def extent(self):
        return self._ax.extent

    def bin(self, i):
        return self._ax.bin(i)

    def __len__(self):
        return self._ax.size

    def __getitem__(self, i):
        if i < 0:
            i += self._ax.size
        if i >= self._ax.size:
            raise IndexError("Out of range access")
        return self.bin(i)

    def __iter__(self):
        return self._ax.__iter__()

    @property
    def edges(self):
        return self._ax.edges

    @property
    def centers(self):
        return self._ax.centers

    @property
    def widths(self):
        return self._ax.widths


class Regular(Axis):
    __slots__ = ()
    _CLASSES = {
        ca.regular_uoflow,
        ca.regular_uoflow_growth,
        ca.regular_uflow,
        ca.regular_oflow,
        ca.regular_none,
        ca.regular_log,
        ca.regular_pow,
        ca.regular_sqrt,
        ca.regular_numpy,
        ca.circular,
    }

    def __init__(self, bins, start, stop, **kwargs):
        """
        Make a regular axis with nice keyword arguments for underflow,
        overflow, and growth.
        """

        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
            options = k.options(
                underflow=True, overflow=True, growth=False, circular=False
            )

        if options == {"growth", "underflow", "overflow"}:
            self._ax = ca.regular_uoflow_growth(bins, start, stop, metadata)
        elif options == {"underflow", "overflow"}:
            self._ax = ca.regular_uoflow(bins, start, stop, metadata)
        elif options == {"underflow"}:
            self._ax = ca.regular_uflow(bins, start, stop, metadata)
        elif options == {"overflow"}:
            self._ax = ca.regular_oflow(bins, start, stop, metadata)
        elif options == {"circular", "underflow", "overflow"}:
            self._ax = ca.circular(bins, start, stop, metadata)
        elif options == set():
            self._ax = ca.regular_none(bins, start, stop, metadata)
        else:
            raise KeyError("Unsupported collection of options")

    @classmethod
    def sqrt(cls, bins, start, stop, **kwargs):
        self = cls.__new__(cls)
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
        self._ax = ca.regular_sqrt(bins, start, stop, metadata)
        return self

    @classmethod
    def log(cls, bins, start, stop, **kwargs):
        self = cls.__new__(cls)
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
        self._ax = ca.regular_log(bins, start, stop, metadata)
        return self

    @classmethod
    def pow(cls, bins, start, stop, power, **kwargs):
        self = cls.__new__(cls)
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
        self._ax = ca.regular_pow(bins, start, stop, power, metadata)
        return self

    @classmethod
    def circular(cls, bins, start, stop, **kwargs):
        self = cls.__new__(cls)
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
        self._ax = ca.circular(bins, start, stop, metadata)
        return self


class Variable(Axis):
    __slots__ = ()
    _CLASSES = {
        ca.variable_none,
        ca.variable_uflow,
        ca.variable_oflow,
        ca.variable_uoflow,
        ca.variable_uoflow_growth,
    }

    def __init__(self, edges, **kwargs):
        """
        Make a variable axis with nice keyword arguments for underflow,
        overflow, and growth.
        """
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
            options = k.options(underflow=True, overflow=True, growth=False)

        if options == {"growth", "underflow", "overflow"}:
            self._ax = ca.variable_uoflow_growth(edges, metadata)
        elif options == {"underflow", "overflow"}:
            self._ax = ca.variable_uoflow(edges, metadata)
        elif options == {"underflow"}:
            self._ax = ca.variable_uflow(edges, metadata)
        elif options == {"overflow"}:
            self._ax = ca.variable_oflow(edges, metadata)
        elif options == set():
            self._ax = ca.variable_none(edges, metadata)
        else:
            raise KeyError("Unsupported collection of options")


class Integer(Axis):
    __slots__ = ()
    _CLASSES = {
        ca.integer_none,
        ca.integer_uflow,
        ca.integer_oflow,
        ca.integer_uoflow,
        ca.integer_growth,
    }

    def __init__(self, start, stop, **kwargs):
        """
        Make an integer axis with nice keyword arguments for underflow,
        overflow, and growth.
        """
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
            options = k.options(underflow=True, overflow=True, growth=False)

        # underflow and overflow settings are ignored, integers are always
        # finite and thus cannot end up in a flow bin when growth is on
        if "growth" in options and "circular" not in options:
            self._ax = ca.integer_growth(start, stop, metadata)
        elif options == {"underflow", "overflow"}:
            self._ax = ca.integer_uoflow(start, stop, metadata)
        elif options == {"underflow"}:
            self._ax = ca.integer_uflow(start, stop, metadata)
        elif options == {"overflow"}:
            self._ax = ca.integer_oflow(start, stop, metadata)
        elif options == set():
            self._ax = ca.integer_none(start, stop, metadata)
        else:
            raise KeyError("Unsupported collection of options")


class Category(Axis):
    __slots__ = ()
    _CLASSES = {
        ca.category_int_growth,
        ca.category_str_growth,
        ca.category_int,
        ca.category_str,
    }

    def __init__(self, categories, **kwargs):
        """
        Make a category axis with ints or strings and with nice keyword
        arguments for growth.
        """
        with _KWArgs(kwargs) as k:
            metadata = k.optional("metadata")
            options = k.options(growth=False)

        if isinstance(categories, str):
            categories = list(categories)

        if options == {"growth"}:
            try:
                self._ax = ca.category_int_growth(categories, metadata)
            except TypeError:
                self._ax = ca.category_str_growth(categories, metadata)
        elif options == set():
            try:
                self._ax = ca.category_int(categories, metadata)
            except TypeError:
                self._ax = ca.category_str(categories, metadata)
        else:
            raise KeyError("Unsupported collection of options")


def _to_axis(ax):
    for base in Axis.__subclasses__():
        if ax.__class__ in base._CLASSES:
            nice_ax = base.__new__(base)
            nice_ax._ax = ax
            return nice_ax
    raise TypeError("Invalid axes passed in")


regular = Regular
variable = Variable
integer = Integer
category = Category

circular = Regular.circular
regular_log = Regular.log
regular_pow = Regular.pow
regular_sqrt = Regular.sqrt
