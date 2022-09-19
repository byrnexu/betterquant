#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace std::ext {

// for std::string
template <class charT, class traits, class A, class Predicate>
void erase_if(std::basic_string<charT, traits, A>& c, Predicate pred) {
  c.erase(remove_if(c.begin(), c.end(), pred), c.end());
}

// for std::deque
template <class T, class A, class Predicate>
void erase_if(std::deque<T, A>& c, Predicate pred) {
  c.erase(remove_if(c.begin(), c.end(), pred), c.end());
}

// for std::vector
template <class T, class A, class Predicate>
void erase_if(std::vector<T, A>& c, Predicate pred) {
  c.erase(remove_if(c.begin(), c.end(), pred), c.end());
}

// for std::list
template <class T, class A, class Predicate>
void erase_if(std::list<T, A>& c, Predicate pred) {
  c.remove_if(pred);
}

// for std::forward_list
template <class T, class A, class Predicate>
void erase_if(std::forward_list<T, A>& c, Predicate pred) {
  c.remove_if(pred);
}

// for std::map
template <class K, class T, class C, class A, class Predicate>
void erase_if(std::map<K, T, C, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::multimap
template <class K, class T, class C, class A, class Predicate>
void erase_if(std::multimap<K, T, C, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::set
template <class K, class C, class A, class Predicate>
void erase_if(std::set<K, C, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::multiset
template <class K, class C, class A, class Predicate>
void erase_if(std::multiset<K, C, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::unordered_map
template <class K, class T, class H, class P, class A, class Predicate>
void erase_if(std::unordered_map<K, T, H, P, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::unordered_multimap
template <class K, class T, class H, class P, class A, class Predicate>
void erase_if(std::unordered_multimap<K, T, H, P, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::unordered_set
template <class K, class H, class P, class A, class Predicate>
void erase_if(std::unordered_set<K, H, P, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

// for std::unordered_multiset
template <class K, class H, class P, class A, class Predicate>
void erase_if(std::unordered_multiset<K, H, P, A>& c, Predicate pred) {
  for (auto i = c.begin(), last = c.end(); i != last;)
    if (pred(*i))
      i = c.erase(i);
    else
      ++i;
}

class spin_mutex {
  std::atomic_flag flag = ATOMIC_FLAG_INIT;

 public:
  void lock() {
    while (flag.test_and_set(memory_order_acquire))
      ;
  }
  void unlock() { flag.clear(memory_order_release); }
};

template <class T>
inline T& tls_get() {
  thread_local T t;
  return t;
}

}  // namespace std::ext
