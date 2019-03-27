import pytest
from pytest import approx

import histogram as bh
import numpy as np


def test_weighted_sum():
    v = bh.accumulator.weighted_sum(1.5, 2.5)

    assert v.value == 1.5
    assert v.variance == 2.5

    v += 1.5

    assert v.value == 3.0
    assert v.variance == 4.75

def test_weighted_mean():
    v = bh.accumulator.weighted_mean()
    v(1,4)
    v(2,1)

    assert v.sum_of_weights == 3.0
    assert v.variance == 4.5
    assert v.value == 2.0

def test_mean():
    v = bh.accumulator.mean()
    v(1)
    v(2)
    v(3)

    assert v.count == 3
    assert v.variance == 1
    assert v.value == 2


def test_sum():
    v = bh.accumulator.sum()
    v += 1
    v += 2
    v += 3

    assert v.value == 6

