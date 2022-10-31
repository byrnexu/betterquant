#pragma once

#include <exception>
#include <tuple>

namespace bq {

template <typename A>
int tuple_length(const A&) {
  return std::tuple_size<A>::value;
}

template <int cidx, typename... A>
typename std::enable_if<cidx >= sizeof...(A), boost::python::object>::type
get_tuple_item_(const std::tuple<A...>& a, int idx, void* = nullptr) {
  throw std::out_of_range{"Ur outta range buddy"};
}

template <int cidx, typename... A,
          typename = std::enable_if<(cidx < sizeof...(A))>>
    typename std::enable_if <
    cidx<sizeof...(A), boost::python::object>::type get_tuple_item_(
        const std::tuple<A...>& a, int idx, int = 42) {
  if (idx == cidx)
    return boost::python::object{std::get<cidx>(a)};
  else
    return get_tuple_item_<cidx + 1>(a, idx);
};

template <typename A>
boost::python::object get_tuple_item(const A& a, int index) {
  return get_tuple_item_<0>(a, index);
}

}  // namespace bq
