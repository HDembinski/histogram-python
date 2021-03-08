import sys
from typing import TYPE_CHECKING, Any, Tuple, Union

if sys.version_info < (3, 8):
    from typing_extensions import Protocol, SupportsIndex
else:
    from typing import Protocol, SupportsIndex

if TYPE_CHECKING:
    from numpy import ufunc as Ufunc
    from numpy.typing import ArrayLike

    from boost_histogram._core.hist import _BaseHistogram as CppHistogram
else:
    ArrayLike = Any
    Ufunc = Any
    CppHistogram = Any


__all__ = (
    "Protocol",
    "CppHistogram",
    "SupportsIndex",
    "AxisLike",
    "ArrayLike",
    "Ufunc",
    "StdIndex",
    "StrIndex",
)


class AxisLike(Protocol):
    def index(self, value: float) -> int:
        ...

    def __len__(self) -> int:
        ...


StdIndex = Union[int, slice, Tuple[Union[slice, int], ...]]
StrIndex = Union[int, slice, str, Tuple[Union[slice, int, str], ...]]
