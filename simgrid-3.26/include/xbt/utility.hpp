/* Copyright (c) 2016-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef XBT_UTILITY_HPP
#define XBT_UTILITY_HPP

#include <tuple>
#include <functional>

namespace simgrid {
namespace xbt {

/** @brief A hash which works with more stuff
 *
 *  It can hash pairs: the standard hash currently doesn't include this.
 */
template <class X> class hash : public std::hash<X> {
};

template <class X, class Y> class hash<std::pair<X, Y>> {
public:
  std::size_t operator()(std::pair<X, Y> const& x) const
  {
    hash<X> h1;
    hash<X> h2;
    return h1(x.first) ^ h2(x.second);
  }
};

/** @brief Comparator class for using with std::priority_queue or boost::heap.
 *
 * Compare two std::pair by their first element (of type double), and return true when the first is greater than the
 * second.  Useful to have priority queues with the smallest element on top.
 */
template <class Pair> class HeapComparator {
public:
  bool operator()(const Pair& a, const Pair& b) const { return a.first > b.first; }
};

/** @brief Erase an element given by reference from a boost::intrusive::list.
 */
template <class List, class Elem> inline void intrusive_erase(List& list, Elem& elem)
{
  list.erase(list.iterator_to(elem));
}

}
}
#endif
