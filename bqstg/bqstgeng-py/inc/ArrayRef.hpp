#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>

/**
 * array_ref offers a non-const view into an array. The storage for the array is
 * not owned by the array_ref object, and it is the client's responsibility to
 * ensure the backing store reamins alive while the array_ref object is in use.
 *
 * @tparam T
 *      Type of elements in the array
 */
template <typename T>
class array_ref {
 public:
  /** Alias for the type of elements in the array */
  typedef T value_type;
  /** Alias for a pointer to value_type */
  typedef T* pointer;
  /** Alias for a constant pointer to value_type */
  typedef T const* const_pointer;
  /** Alias for a reference to value_type */
  typedef T& reference;
  /** Alias for a constant reference to value_type */
  typedef T const& const_reference;
  /** Alias for an iterator pointing at value_type objects */
  typedef T* iterator;
  /** Alias for a constant iterator pointing at value_type objects */
  typedef T const* const_iterator;
  /** Alias for a reverse iterator pointing at value_type objects */
  typedef std::reverse_iterator<iterator> reverse_iterator;
  /** Alias for a constant reverse iterator pointing at value_type objects */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  /** Alias for an unsigned integral type used to represent size related values
   */
  typedef std::size_t size_type;
  /** Alias for a signed integral type used to represent result of difference
   * computations */
  typedef std::ptrdiff_t difference_type;

  /** Default constructor */
  constexpr array_ref() noexcept = default;

  /**
   * Constructor that accepts a pointer to an array and the number of elements
   * pointed at
   *
   * @param arr
   *    Pointer to array
   * @param length
   *    Number of elements pointed at
   */
  constexpr array_ref(pointer arr, size_type length) noexcept
      : begin_(arr), length_(length) {}

  /**
   * Constructor that accepts a reference to an array
   *
   * @tparam N
   *    Number of elements in the array
   */
  template <size_type N>
  constexpr array_ref(T (&arr)[N]) noexcept : begin_(&arr[0]), length_(N) {}

  /**
   * Constructor taking a pair of pointers pointing to the first element and one
   * past the last element of the array, respectively.
   *
   * @param first
   *    Pointer to the first element of the array
   * @param last
   *    Pointer to one past the last element of the array
   */
  array_ref(pointer first, pointer last) noexcept
      : begin_(first),
        length_(static_cast<size_type>(std::distance(first, last))) {}

  /** Copy constructor */
  constexpr array_ref(array_ref const&) noexcept = default;

  /** Copy assignment operator */
  array_ref& operator=(array_ref const&) noexcept = default;

  /** Move constructor */
  constexpr array_ref(array_ref&&) noexcept = default;

  /** Move assignment operator */
  array_ref& operator=(array_ref&&) noexcept = default;

  /**
   * Returns an iterator to the first element of the array. If the array is
   * empty, the returned iterator will be equal to end().
   *
   * @return An iterator to the first element of the array
   */
  /*constexpr*/ iterator begin() noexcept { return begin_; }

  /**
   * Returns a constant iterator to the first element of the array. If the array
   * is empty, the returned iterator will be equal to end().
   *
   * @return A constant iterator to the first element of the array
   */
  constexpr const_iterator begin() const noexcept { return begin_; }

  /**
   * Returns a constant iterator to the first element of the array. If the array
   * is empty, the returned iterator will be equal to end().
   *
   * @return A constant iterator to the first element of the array
   */
  constexpr const_iterator cbegin() const noexcept { return begin_; }

  /**
   * Returns an iterator to the element following the last element of the array.
   *
   * @return An iterator to the element following the last element of the array
   */
  /*constexpr*/ iterator end() noexcept { return begin() + size(); }

  /**
   * Returns a constant iterator to the element following the last element of
   * the array.
   *
   * @return A constant iterator to the element following the last element of
   * the array
   */
  constexpr const_iterator end() const noexcept { return begin() + size(); }

  /**
   * Returns a constant iterator to the element following the last element of
   * the array.
   *
   * @return A constant iterator to the element following the last element of
   * the array
   */
  constexpr const_iterator cend() const noexcept { return cbegin() + size(); }

  /**
   * Returns a reverse iterator to the first element of the reversed array. It
   * corresponds to the last element of the non-reversed array.
   *
   * @return A reverse iterator to the first element of the reversed array
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  /**
   * Returns a constant reverse iterator to the first element of the reversed
   * array. It corresponds to the last element of the non-reversed array.
   *
   * @return A constant reverse iterator to the first element of the reversed
   * array
   */
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  /**
   * Returns a constant reverse iterator to the first element of the reversed
   * array. It corresponds to the last element of the non-reversed array.
   *
   * @return A constant reverse iterator to the first element of the reversed
   * array
   */
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  /**
   * Returns a reverse iterator to the element following the last element of the
   * reversed array. It corresponds to the element preceding the first element
   * of the non-reversed array.
   *
   * @return A reverse iterator to the element following the last element of the
   * reversed array
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  /**
   * Returns a constant reverse iterator to the element following the last
   * element of the reversed array. It corresponds to the element preceding the
   * first element of the non-reversed array.
   *
   * @return A constant reverse iterator to the element following the last
   * element of the reversed array
   */
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  /**
   * Returns a constant reverse iterator to the element following the last
   * element of the reversed array. It corresponds to the element preceding the
   * first element of the non-reversed array.
   *
   * @return A constant reverse iterator to the element following the last
   * element of the reversed array
   */
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  /**
   * Returns the number of elements in the array.
   *
   * @return The number of elements in the array
   */
  constexpr size_type size() const noexcept { return length_; }

  /**
   * Indicates whether the array has no elements
   *
   * @return true if the array has no elements, false otherwise
   */
  constexpr bool empty() const noexcept { return size() == 0; }

  /**
   * Returns a reference to the element at the specified location.
   *
   * @return A reference to the element at the specified location
   * @pre i < size()
   */
  /*constexpr*/ reference operator[](size_type i) {
#ifndef NDEBUG
    return at(i);
#else
    return *(begin() + i);
#endif
  }

  /**
   * Returns a constant reference to the element at the specified location.
   *
   * @return A constant reference to the element at the specified location
   * @pre i < size()
   */
  constexpr const_reference operator[](size_type i) const {
#ifndef NDEBUG
    return at(i);
#else
    return *(begin() + i);
#endif
  }

  /**
   * Returns a reference to the element at the specified location, with bounds
   * checking.
   *
   * @return A reference to the element at the specified location
   * @throw std::out_of_range if the specified index is not within the range of
   * the array
   */
  /*constexpr*/ reference at(size_type i) {
    if (i >= size()) {
      throw std::out_of_range("index out of range");
    }
    return *(begin() + i);
  }

  /**
   * Returns a constant reference to the element at the specified location, with
   * bounds checking.
   *
   * @return A constant reference to the element at the specified location
   * @throw std::out_of_range if the specified index is not within the range of
   * the array
   */
  /*constexpr*/ const_reference at(size_type i) const {
    if (i >= size()) {
      throw std::out_of_range("index out of range");
    }
    return *(begin() + i);
  }

  /**
   * Returns a reference to the first element of the array
   *
   * @return A reference to the first element of the array
   * @pre empty() == false
   */
  /*constexpr*/ reference front() noexcept { return *begin(); }

  /**
   * Returns a reference to the first element of the array
   *
   * @return A reference to the first element of the array
   * @pre empty() == false
   */
  constexpr const_reference front() const noexcept { return *begin(); }

  /**
   * Returns a reference to the last element of the array
   *
   * @return A reference to the last element of the array
   * @pre empty() == false
   */
  /*constexpr*/ reference back() noexcept { return *(end() - 1); }

  /**
   * Returns a constant reference to the last element of the array
   *
   * @return A constant reference to the last element of the array
   * @pre empty() == false
   */
  constexpr const_reference back() const noexcept { return *(end() - 1); }

  /**
   * Returns a pointer to the address of the first element of the array
   *
   * @return A pointer to the address of the first element of the array
   */
  /*constexpr*/ pointer data() noexcept { return begin(); }

  /**
   * Returns a constant pointer to the address of the first element of the array
   *
   * @return A constant pointer to the address of the first element of the array
   */
  constexpr const_pointer data() const noexcept { return begin(); }

  /**
   * Resets the operand back to its default constructed state
   *
   * @post empty() == true
   */
  void clear() noexcept {
    begin_ = nullptr;
    length_ = 0;
  }

 private:
  /** Pointer to the first element of the referenced array */
  pointer begin_ = nullptr;
  /** Number of elements in the referenced array */
  size_type length_ = size_type();
};
