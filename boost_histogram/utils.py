class FactoryMeta(object):
    def __init__(self, f, types):
        self._f = f
        self._types = types

    def __call__(self, *args, **kwargs):
        return self._f(*args, **kwargs)

    def __instancecheck__(self, other):
        return isinstance(other, self._types)


class loc(object):
    __slots__ = ("value",)

    def __init__(self, value):
        self.value = value


class rebin(object):
    __slots__ = ("factor",)
    projection = False

    def __init__(self, value):
        self.factor = value


class _project(object):
    __slots__ = ()
    projection = True


project = _project


def indexed(histogram, flow=False):
    """Set up an iterator, returns a special accessor for bin info and content."""
    return histogram.indexed(flow=flow)


class KWArgs(object):
    def __init__(self, kwargs):
        self.kwargs = kwargs

    def __enter__(self):
        return self

    def __exit__(self, *args):
        if self.kwargs:
            raise TypeError("Keyword(s) {} not expected".format(", ".join(self.kwargs)))

    def required(self, name):
        if name in self.kwargs:
            self.kwargs.pop(name)
        else:
            raise KeyError("{0} is required".format(name))

    def optional(self, name, default=None):
        if name in self.kwargs:
            return self.kwargs.pop(name)
        else:
            return default

    def options(self, **options):
        return {option for option in options if self.optional(option, options[option])}
