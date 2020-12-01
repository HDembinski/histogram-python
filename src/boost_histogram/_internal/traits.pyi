from typing import Any


class Traits:
    underflow
    overflow: bool
    circular: bool
    growth: bool
    continuous: bool
    inclusive: bool
    ordered: bool

    def __init__(
        self,
        underflow=False,
        overflow=False,
        circular=False,
        growth=False,
        continuous=False,
        inclusive=False,
        ordered=False,
    ): ...

    @property
    def discrete(self) -> bool: ...

    def __eq__(self, other: Any) -> bool: ...

    def __ne__(self, other: Any) -> bool: ...

    def __repr__(self) -> str: ...
