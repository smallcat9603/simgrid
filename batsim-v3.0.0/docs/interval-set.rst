.. _interval_set:

Interval set
============

Resources are represented as interval sets in the :ref:`protocol`
(e.g., in :ref:`proto_EXECUTE_JOB`) and in some output files
(e.g., :ref:`output_jobs`).
As the name suggests, an interval set is a set of intervals.
More precisely, the intervals are closed integer intervals.

As an example, the set of resources :math:`\{1, 2, 3, 5, 7\}` equals to the
:math:`[1,3]\cup[5,5]\cup[7,7]` interval set.

.. _interval_set_string_representation:

Interval set string representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Batsim uses a compact string representation of interval sets.

Each interval :math:`[a,b]` is represented as ``a-b``
or more simply by ``a`` if the degenerate case :math:`a = b`.
The delimiter between ``a`` and ``b`` is the minus sign (ASCII_/`UTF-8`_ 0x2D).
As an example, the :math:`[37,42]=\{37,38,39,40,41,42\}` interval is represented as ``37-42``.

Interval sets are represented as intervals separated by a space (ASCII_/`UTF-8`_ 0x20).
As an example, the interval set :math:`[1,3]\cup[5,5]\cup[7,7] = \{1, 2, 3, 5, 7\}` is represented as ``1-3 5 7``.

.. note::
    The same set of resources can have **many** string representations.
    For example, :math:`\{1, 2, 3, 5, 7\}` can be represented as ``1-3 5 7``, ``1-2 3 5 7``, ``1 2 3 5 7``,
    ``1-2 1-3 5 7`` or even ``5 2-2 7 1-3``.

Canonical string representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The canonical string representation of an interval set respects the following rules.

- Intervals are disjoint (their intersection is empty).
- Intervals are as big as possible — *e.g.*, ``1-3 4-5`` is not a canonical representation of :math:`[1,5]`.
- Intervals of size 1 are represented as ``a``, not ``a-a``.
- Intervals are sorted in ascending order.

This representation is unique for an interval set and is the way to go if you
need to manually generate interval set string representations.

All implementations should be able to read canonical string representations and to generate them.
Canonical string representations are the only representations that are ensured to remain the same
by being converted into an interval set and by being converted back to a string representation.
:math:`canon\_repr \rightarrow interval set \rightarrow canon\_repr`.

Interval set libraries
~~~~~~~~~~~~~~~~~~~~~~

Software libraries allow to operate interval sets from different programming languages.

- C++: intervalset_ (used by Batsim)
- D: dintervalset_
- python: `procset.py`_
- Rust: `procset.rs`_

.. _ASCII: https://en.wikipedia.org/wiki/ASCII
.. _CSV: https://en.wikipedia.org/wiki/Comma-separated_values
.. _dintervalset: https://gitlab.inria.fr/batsim/dintervalset
.. _intervalset: https://framagit.org/batsim/intervalset
.. _procset.py: https://gitlab.inria.fr/bleuse/procset.py
.. _procset.rs: https://github.com/adfaure/procset.rs
.. _intervalset C++ library quick example: https://intervalset.readthedocs.io/en/latest/usage.html#quick-example
.. _UTF-8: https://en.wikipedia.org/wiki/UTF-8
