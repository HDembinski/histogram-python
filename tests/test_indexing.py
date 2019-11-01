import boost_histogram as bh
import numpy as np

import pytest


def test_1D_get_bin():

    h = bh.histogram(bh.axis.regular(10, 0, 1))
    h.fill([0.25, 0.25, 0.25, 0.15])

    assert h[0] == 0
    assert h[1] == 1
    assert h[2] == 3

    assert h[bh.loc(0)] == 0
    assert h[bh.loc(0.1)] == 1
    assert h[bh.loc(0.1) + 1] == 3
    assert h[bh.loc(0.2)] == 3

    assert h.view()[2] == h[2]

    with pytest.raises(IndexError):
        h[1, 2]


def test_2D_get_bin():

    h = bh.histogram(bh.axis.regular(10, 0, 1), bh.axis.regular(10, 0, 1))
    h.fill(0.15, [0.25, 0.25, 0.25, 0.15])

    assert h[0, 0] == 0
    assert h[0, 1] == 0
    assert h[1, 1] == 1
    assert h[1, 2] == 3
    assert h[bh.loc(0.1), bh.loc(0.2)] == 3
    assert h[bh.loc(0) + 1, bh.loc(0.3) - 1] == 3

    assert h.view()[1, 2] == h[1, 2]

    with pytest.raises(IndexError):
        h[1]


def test_get_1D_histogram():
    h = bh.histogram(bh.axis.regular(10, 0, 1))
    h.fill([0.25, 0.25, 0.25, 0.15])

    h2 = h[:]

    assert h == h2

    h.fill(0.75)

    assert h != h2


def test_get_1D_slice():
    h1 = bh.histogram(bh.axis.regular(10, 0, 1))
    h2 = bh.histogram(bh.axis.regular(5, 0, 0.5))
    h1.fill([0.25, 0.25, 0.25, 0.15])
    h2.fill([0.25, 0.25, 0.25, 0.15])

    assert h1 != h2
    assert h1[:5] == h2
    assert h1[: bh.loc(0.5)] == h2
    assert h1[2:4] == h2[2:4]
    assert h1[bh.loc(0.2) : bh.loc(0.4)] == h2[bh.loc(0.2) : bh.loc(0.4)]

    assert len(h1[2:4].view()) == 2
    assert len(h1[2 : 4 : bh.rebin(2)].view()) == 1


def test_ellipsis():

    h = bh.histogram(bh.axis.regular(10, 0, 1), bh.axis.regular(10, 0, 1))

    assert h == h[...]
    assert h == h[:, ...]
    assert h == h[..., :]
    assert h == h[:, :, ...]
    assert h == h[:, ..., :]
    assert h == h[..., :, :]

    with pytest.raises(IndexError):
        h[:, :, :, ...]
    with pytest.raises(IndexError):
        h[:, :, ..., :]
    with pytest.raises(IndexError):
        h[..., :, :, :]
    with pytest.raises(IndexError):
        h[..., ...]

    assert h[2:4, ...] == h[2:4, :]


def test_basic_projection():
    h2 = bh.histogram(
        bh.axis.regular(10, 0, 10),
        bh.axis.regular(10, 0, 10),
        bh.axis.regular(10, 0, 10),
    )
    h1 = bh.histogram(bh.axis.regular(10, 0, 10))

    contents = [[2, 2, 2, 3, 4, 5, 6], [1, 2, 2, 3, 2, 1, 2], [-12, 33, 4, 9, 2, 4, 9]]

    h1.fill(contents[0])
    h2.fill(*contents)

    assert h1 == h2[:, :: bh.project, :: bh.project]
    assert h1 == h2[..., :: bh.project, :: bh.project]
    assert h2.sum(flow=True) == h2[:: bh.project, :: bh.project, :: bh.project]


def test_slicing_projection():
    h1 = bh.histogram(
        bh.axis.regular(10, 0, 10),
        bh.axis.regular(10, 0, 10),
        bh.axis.regular(10, 0, 10),
    )

    X, Y, Z = np.mgrid[-0.5:10.5:12j, -0.5:10.5:12j, -0.5:10.5:12j]

    h1.fill(X.ravel(), Y.ravel(), Z.ravel())

    assert h1[:: bh.project, :: bh.project, :: bh.project] == 12 ** 3
    assert (
        h1[
            0 : bh.tag.end : bh.project,
            0 : bh.tag.end : bh.project,
            0 : len : bh.project,
        ]
        == 10 ** 3
    )
    assert (
        h1[0 : bh.tag.end : bh.project, 0 : len : bh.project, :: bh.project]
        == 10 * 10 * 12
    )
    assert h1[:: bh.project, 0 : len : bh.project, :: bh.project] == 10 * 12 * 12

    # make sure nothing was modified
    assert h1.sum() == 10 ** 3
    assert h1.sum(flow=True) == 12 ** 3

    h2 = h1[0 : 3 : bh.project, ...]
    assert h2[1, 2] == 3

    h3 = h2[:, 5 : 7 : bh.project]
    assert h3[1] == 6
