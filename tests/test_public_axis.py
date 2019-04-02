import pytest
from pytest import approx

from boost.histogram.axis import (regular, regular_noflow,
                                  regular_log, regular_sqrt,
                                  regular_pow, circular,
                                  variable, integer,
                                  category_int as category)

import numpy as np

import abc

# compatible with Python 2 *and* 3:
ABC = abc.ABCMeta('ABC', (object,), {'__slots__': ()})

# histogram -> boost.histogram
# regular(..., noflow=True) -> regular_noflow(...)
# label -> metadata
# regular_pow(n,start,stop,power) +> regular_pow(power,n,start,stop)
# len(ax) -> ax.size() # also .extent()
# ax[i] -> ax.bin(i) # (.lower() and .upper() instead of [0] and [1]) (may return)
# Circular is very different (Boost::Histogram change)
# Variable and category take an array/list now

class Axis(ABC):

    @abc.abstractmethod
    def test_init(self):
        pass

    @abc.abstractmethod
    def test_len(self):
        pass

    @abc.abstractmethod
    def test_repr(self):
        pass

    @abc.abstractmethod
    def test_getitem(self):
        pass

    @abc.abstractmethod
    def test_iter(self):
        pass

    @abc.abstractmethod
    def test_index(self):
        pass



class TestRegular(Axis):

    def test_init(self):
        # Should not throw
        regular(1, 1.0, 2.0)
        regular(1, 1.0, 2.0, metadata="ra")
        regular_noflow(1, 1.0, 2.0)
        regular_noflow(1, 1.0, 2.0, metadata="ra")
        regular_log(1, 1.0, 2.0)
        regular_sqrt(1, 1.0, 2.0)
        regular_pow(1.5, 1, 1.0, 2.0)

        with pytest.raises(TypeError):
            regular()
        with pytest.raises(TypeError):
            regular()
        with pytest.raises(TypeError):
            regular(1)
        with pytest.raises(TypeError):
            regular(1, 1.0)
        with pytest.raises(ValueError):
            regular(0, 1.0, 2.0)
        with pytest.raises(TypeError):
            regular("1", 1.0, 2.0)
        with pytest.raises(Exception):
            regular(-1, 1.0, 2.0)
        # CLASSIC
        #with pytest.raises(ValueError):
        regular(1, 2.0, 1.0)

        with pytest.raises(ValueError):
            regular(1, 1.0, 1.0)

        # CLASSIC: this was not allowed. Now it is.
        # with pytest.raises(TypeError):
        regular(1, 1.0, 2.0, metadata=0)



        with pytest.raises(TypeError):
            regular(1, 1.0, 2.0, bad_keyword="ra")
        with pytest.raises(TypeError):
            regular_pow(1, 1.0, 2.0)

        a = regular(4, 1.0, 2.0)
        assert a == regular(4, 1.0, 2.0)
        assert a != regular(3, 1.0, 2.0)
        assert a != regular(4, 1.1, 2.0)
        assert a != regular(4, 1.0, 2.1)


    def test_len(self):
        a = regular(4, 1.0, 2.0)
        # CLASSIC: Not explicit
        # assert len(a) == 4

        assert a.size() == 4
        assert a.extent() == 6

    def test_repr(self):
        ax = regular(4, 1.1, 2.2)
        assert repr(ax) == "regular(4, 1.1, 2.2, options=underflow | overflow)"

        ax = regular(4, 1.1, 2.2, metadata='ra')
        assert repr(ax) == 'regular(4, 1.1, 2.2, metadata="ra", options=underflow | overflow)'

        ax = regular_noflow(4, 1.1, 2.2)
        assert repr(ax) == "regular(4, 1.1, 2.2, options=none)"

        ax = regular_noflow(4, 1.1, 2.2, metadata='ra')
        assert repr(ax) == 'regular(4, 1.1, 2.2, metadata="ra", options=none)'

        ax = regular_log(4, 1.1, 2.2)
        assert repr(ax) == 'regular_log(4, 1.1, 2.2, options=underflow | overflow)'

        ax = regular_sqrt(3, 1.1, 2.2)
        assert repr(ax) == 'regular_sqrt(3, 1.1, 2.2, options=underflow | overflow)'

        ax = regular_pow(0.5, 4, 1.1, 2.2)
        assert repr(ax) == 'regular_pow(4, 1.1, 2.2, options=underflow | overflow, power=0.5)'


    def test_getitem(self):
        v = [1.0, 1.25, 1.5, 1.75, 2.0]
        a = regular(4, 1.0, 2.0)
        for i in range(4):
            a.bin(i).lower() == approx(v[i])
            a.bin(i).upper() == approx(v[i+1])
        assert a.bin(-1).lower() == -float("infinity")
        assert a.bin(4).upper() == float("infinity")

        # CLASSIC: bins outside the range now have different behavior
        # with pytest.raises(IndexError):
        #     a.bin(-2)
        # with pytest.raises(IndexError):
        #     a.bin(5)

        assert a.bin(-2).lower() == -float("infinity")
        assert a.bin(-2).upper() == -float("infinity")
        assert a.bin(5).lower() == float("infinity")
        assert a.bin(5).upper() == float("infinity")

    def test_iter(self):
        v = np.array([1.0, 1.25, 1.5, 1.75, 2.0])
        a = regular(4, 1.0, 2.0)
        assert np.all(a.edges() == v)

        c = (v[:-1] + v[1:])/2
        assert np.all(a.centers() == approx(c))

    def test_index(self):
        a = regular(4, 1.0, 2.0)

        assert a.index(-1) == -1
        assert a.index(0.99) == -1
        assert a.index(1.0) == 0
        assert a.index(1.249) == 0
        assert a.index(1.250) == 1
        assert a.index(1.499) == 1
        assert a.index(1.500) == 2
        assert a.index(1.749) == 2
        assert a.index(1.750) == 3
        assert a.index(1.999) == 3
        assert a.index(2.000) == 4
        assert a.index(20) == 4

    def test_reversed_index(self):
        a = regular(4, 2.0, 1.0)

        assert a.index(-1) == 4
        assert a.index(0.99) == 4
        assert a.index(1.0) == 4
        assert a.index(1.249) == 3
        assert a.index(1.250) == 3
        assert a.index(1.499) == 2
        assert a.index(1.500) == 2
        assert a.index(1.749) == 1
        assert a.index(1.750) == 1
        assert a.index(1.999) == 0
        assert a.index(2.000) == 0
        assert a.index(20) == -1


    def test_log_transform(self):
        a = regular_log(2, 1e0, 1e2)

        assert a.index(-1) == 2
        assert a.index(0.99) == -1
        assert a.index(1.0) == 0
        assert a.index(9.99) == 0
        assert a.index(10.0) == 1
        assert a.index(99.9) == 1
        assert a.index(100) == 2
        assert a.index(1000) == 2

        assert a.bin(0).lower() == approx(1e0)
        assert a.bin(1).lower() == approx(1e1)
        assert a.bin(1).upper() == approx(1e2)

    def test_pow_transform(self):
        a = regular_pow(0.5, 2, 1.0, 9.0)

        assert a.index(-1) == 2
        assert a.index(0.99) == -1
        assert a.index(1.0) == 0
        assert a.index(3.99) == 0
        assert a.index(4.0) == 1
        assert a.index(8.99) == 1
        assert a.index(9) == 2
        assert a.index(1000) == 2

        assert a.bin(0).lower(), approx(1.0)
        assert a.bin(1).lower(), approx(4.0)
        assert a.bin(1).upper(), approx(9.0)




class TestCircular(Axis):

    def test_init(self):
        # Should not throw

        # CLASSIC: This was supported. Now it is not (ambiguous)
        with pytest.raises(TypeError):
            circular(1)

        circular(4, 1.0)
        circular(4, 0.0, 1.0)
        circular(4, 1.0, metadata="pa")
        circular(4, 0.0, 1.0, metadata="pa")

        with pytest.raises(TypeError):
            circular()
        with pytest.raises(Exception):
            circular(-1)
        # CLASSIC: Used to be disallowed, now matches as metadata.
        circular(1, 1.0, 2.0, 3.0)
        # CLASSIC: Used to be disallowed.
        circular(1, 1.0, metadata=1)
        with pytest.raises(TypeError):
            circular("1")

        a = circular(4, 1.0)
        assert a == circular(4, 1.0)
        assert a != circular(2, 1.0)

        # CLASSIC: This used to do something, now is range of 0 error
        with pytest.raises(ValueError):
            circular(4, 0.0)

    def test_len(self):
        assert circular(4, 1.0).size() == 4
        assert circular(4, 1.0).extent() == 5
        assert circular(4, 0.0, 1.0).size() == 4
        assert circular(4, 0.0, 1.0).extent() == 5

    def test_repr(self):
        ax = circular(4, 1.1, 2.2)
        assert repr(ax) == "regular(4, 1.1, 2.2, options=overflow | circular)"

        ax = circular(4, 1.1, 2.2, metadata='hi')
        assert repr(ax) == 'regular(4, 1.1, 2.2, metadata="hi", options=overflow | circular)'

        ax = circular(4, 2.0)
        assert repr(ax) == "regular(4, 0, 2, options=overflow | circular)"

        ax = circular(4, 2.0, metadata='hi')
        assert repr(ax) == 'regular(4, 0, 2, metadata="hi", options=overflow | circular)'


    def test_getitem(self):
        v = [1.0,
             1.0 + 0.5 * np.pi,
             1.0 + np.pi,
             1.0 + 1.5 * np.pi,
             1.0 + 2.0 * np.pi]

        # CLASSIC: Used to be 1 (phase 2pi automatic?)
        a = circular(4, 1, 1+np.pi*2)

        for i in range(4):
            assert a.bin(i).lower() == v[i]
            assert a.bin(i).upper() == v[i + 1]

        # CLASSIC: Out of range used to raise
        # TODO: test out of range


    def test_iter(self):
        v = np.array([1.0,
             1.0 + 0.5 * np.pi,
             1.0 + np.pi,
             1.0 + 1.5 * np.pi,
             1.0 + 2.0 * np.pi])

        a = circular(4, 1, 1+np.pi*2)
        assert np.all(a.edges() == v)

        c = (v[:-1] + v[1:])/2
        assert np.all(a.centers() == approx(c))

    def test_index(self):
        a = circular(4, 1, 1+np.pi*2)
        d = 0.5 * np.pi
        assert a.index(0.99 - 4 * d) == 3
        assert a.index(0.99 - 3 * d) == 0
        assert a.index(0.99 - 2 * d) == 1
        assert a.index(0.99 - d) == 2
        assert a.index(0.99) == 3
        assert a.index(1.0) == 0
        assert a.index(1.01) == 0
        assert a.index(0.99 + d) == 0
        assert a.index(1.0 + d) == 1
        assert a.index(1.0 + 2 * d) == 2
        assert a.index(1.0 + 3 * d) == 3
        assert a.index(1.0 + 4 * d) == 0
        assert a.index(1.0 + 5 * d) == 1


class TestVariable(Axis):

    def test_init(self):
        variable([0, 1])
        variable([0, 1, 2, 3, 4])
        variable([0, 1], metadata="va")
        with pytest.raises(TypeError):
            variable()
        with pytest.raises(ValueError):
            variable([1])
        with pytest.raises(TypeError):
            variable(1)
        with pytest.raises(ValueError):
            variable([1, -1])
        with pytest.raises(ValueError):
            variable([1, 1])
        with pytest.raises(TypeError):
            variable(["1", 2])
        with pytest.raises(TypeError):
            variable([0.0, 1.0, 2.0], bad_keyword="ra")

        a = variable([-0.1, 0.2, 0.3])
        assert a == variable([-0.1, 0.2, 0.3])
        assert a != variable([0, 0.2, 0.3])
        assert a != variable([-0.1, 0.1, 0.3])
        assert a != variable([-0.1, 0.1])

    def test_len(self):
        assert variable([-0.1, 0.2, 0.3]).size() == 2
        assert variable([-0.1, 0.2, 0.3]).extent() == 4

    def test_repr(self):
        ax = variable([-0.1, 0.2])
        assert repr(ax) == "variable(-0.1, 0.2, options=underflow | overflow)"

        ax = variable([-0.1, 0.2], metadata='hi')
        assert repr(ax) == 'variable(-0.1, 0.2, metadata="hi", options=underflow | overflow)'


    def test_getitem(self):
        v = [-0.1, 0.2, 0.3]
        a = variable(v)

        for i in range(2):
            assert a.bin(i).lower() == v[i]
            assert a.bin(i).upper() == v[i + 1]

        assert a.bin(-1).lower() == -float("infinity")
        assert a.bin(-1).upper() == v[0]

        assert a.bin(2).lower() == v[2]
        assert a.bin(2).upper() == float("infinity")

        # CLASSIC: out of range used to throw
        assert a.bin(-2).upper() == -float("infinity")
        assert a.bin(3).lower() == float("infinity")


    def test_iter(self):
        v = np.array([-0.1, 0.2, 0.3])
        a = variable(v)

        assert np.all(a.edges() == v)

        c = (v[:-1] + v[1:])/2
        assert np.all(a.centers() == approx(c))

    def test_index(self):
        a = variable([-0.1, 0.2, 0.3])
        assert a.index(-10.0) == -1
        assert a.index(-0.11) == -1
        assert a.index(-0.1) == 0
        assert a.index(0.0) == 0
        assert a.index(0.19) == 0
        assert a.index(0.2) == 1
        assert a.index(0.21) == 1
        assert a.index(0.29) == 1
        assert a.index(0.3) == 2
        assert a.index(0.31) == 2
        assert a.index(10) == 2


class TestCategory(Axis):

    def test_init(self):
        category([1, 2, 3])
        category([1, 2, 3], metadata="ca")
        with pytest.raises(TypeError):
            category()
        with pytest.raises(TypeError):
            category(["1"])
        with pytest.raises(TypeError):
            category([1, "2"])
        # CLASSIC: Used to raise TypeError
        category([1, 2], metadata=1)
        with pytest.raises(TypeError):
            category([1, 2, 3], uoflow=True)

        assert category([1, 2, 3]) == category([1, 2, 3])
        assert category([1, 2, 3]) != category([1, 3, 2])

    def test_len(self):
        assert category([1,2,3]).size() == 3
        assert category([1,2,3]).extent() == 4

    def test_repr(self):
        ax = category([1,2,3])
        assert repr(ax) == "category(1, 2, 3, options=overflow)"

        ax = category([1,2,3], metadata='hi')
        assert repr(ax) == 'category(1, 2, 3, metadata="hi", options=overflow)'


    def test_getitem(self):
        c = [1, 2, 3]
        a = category(c)

        for i in range(3):
            assert a.bin(i) == c[i]

        # CLASSIC: out of range used to throw
        # TODO: Check out of range bin values


    def test_iter(self):
        c = [1, 2, 3]
        a = category(c)

        with pytest.raises(AttributeError):
            a.edges()

        with pytest.raises(AttributeError):
            a.centers()

    def test_index(self):
        c = [1, 2, 3]
        a = category(c)

        assert a.index(1) == 0
        assert a.index(2) == 1
        assert a.index(3) == 2
