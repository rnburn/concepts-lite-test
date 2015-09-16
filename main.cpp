#include <type_traits>
#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <iostream>
#include <cassert>

namespace cpt {
//------------------------------------------------------------------------------
// Same
//------------------------------------------------------------------------------
template<class X, class Y>
concept bool Same = std::is_same<X, Y>::value;

//------------------------------------------------------------------------------
// Copyable
//------------------------------------------------------------------------------
template <class X>
concept bool Copyable =
    std::is_copy_constructible<X>::value && std::is_copy_assignable<X>::value;

//------------------------------------------------------------------------------
// DefaultConstructible
//------------------------------------------------------------------------------
template<class X>
concept bool DefaultConstructible =
  std::is_default_constructible<X>::value;

//------------------------------------------------------------------------------
// NothrowDestructible
//------------------------------------------------------------------------------
template<class X>
concept bool NothrowDestructible = 
  std::is_nothrow_destructible<X>::value;

//------------------------------------------------------------------------------
// Destructible
//------------------------------------------------------------------------------
template<class X>
concept bool Destructible =
  std::is_destructible<X>::value;

//------------------------------------------------------------------------------
// EqualityComparable
//------------------------------------------------------------------------------
template<class X>
concept bool EqualityComparable = 
  requires(X x) {
     {x == x} -> bool;
     {x != x} -> bool;
  };

//------------------------------------------------------------------------------
// EqualityComparable2
//------------------------------------------------------------------------------
template<class X, class Y>
concept bool EqualityComparable2 = 
  requires(X x, Y y) {
    requires EqualityComparable<X>;
    requires EqualityComparable<Y>;
    {x == y} -> bool;
    {y == x} -> bool;
    {x != y} -> bool;
    {y != x} -> bool;
  };

//------------------------------------------------------------------------------
// WeaklyOrdered
//------------------------------------------------------------------------------
template<class X>
concept bool WeaklyOrdered =
  requires(X x) {
    {x < x} -> bool;
    {x > x} -> bool;
    {x <= x} -> bool;
    {x >= x} -> bool;
  };

//------------------------------------------------------------------------------
// TotallyOrdered
//------------------------------------------------------------------------------
template<class X>
concept bool TotallyOrdered = WeaklyOrdered<X> && EqualityComparable<X>;

//------------------------------------------------------------------------------
// Allocatable
//------------------------------------------------------------------------------
template<class X>
concept bool Allocatable = 
  requires(X x) {
      {new X} -> X*;
      {delete new X};
      {new X[1]} -> X*;
      {delete[] new X[1]};
  };

//------------------------------------------------------------------------------
// Semiregular
//------------------------------------------------------------------------------
template<class X>
concept bool Semiregular = 
  DefaultConstructible<X> && Copyable<X> && Destructible<X> && Allocatable<X>;

//------------------------------------------------------------------------------
// Regular
//------------------------------------------------------------------------------
template<class X>
concept bool Regular = Semiregular<X> && EqualityComparable<X>;

//------------------------------------------------------------------------------
// Integral
//------------------------------------------------------------------------------
template<class X>
concept bool Integral = std::is_integral<X>::value;

//------------------------------------------------------------------------------
// SignedIntegral
//------------------------------------------------------------------------------
template<class X>
concept bool SignedIntegral = Integral<X> && std::is_signed<X>::value;

//------------------------------------------------------------------------------
// Readable
//------------------------------------------------------------------------------
template<class X>
concept bool Readable = 
  requires(X x) {
    requires Semiregular<X>;
    {*x} -> typename std::iterator_traits<X>::reference; 
  };

//------------------------------------------------------------------------------
// Writable
//------------------------------------------------------------------------------
template<class X>
concept bool Writable =
  requires(X x) {
    requires Semiregular<X>;
    {*x = std::declval<typename std::iterator_traits<X>::value_type>()};
  };

//------------------------------------------------------------------------------
// WeaklyIncrementable
//------------------------------------------------------------------------------
template<class X>
concept bool WeaklyIncrementable =
  requires(X x) {
    typename std::iterator_traits<X>::difference_type;
    {++x} -> X&;
    {x++} -> const X&;
  };

//------------------------------------------------------------------------------
// Incrementable
//------------------------------------------------------------------------------
template<class X>
concept bool Incrementable =
  requires(X x) {
    requires Regular<X>;
    requires WeaklyIncrementable<X>;
    {x++}->X;
  };

//------------------------------------------------------------------------------
// WeakIterator
//------------------------------------------------------------------------------
template<class X>
concept bool WeakIterator =
  requires(X x) {
    requires WeaklyIncrementable<X>;
    requires Copyable<X>;
    {*x};
  };

//------------------------------------------------------------------------------
// Iterator
//------------------------------------------------------------------------------
template<class X>
concept bool Iterator = WeakIterator<X> && EqualityComparable<X>;

//------------------------------------------------------------------------------
// WeakOutputIterator
//------------------------------------------------------------------------------
template<class X>
concept bool WeakOutputIterator = Writable<X> && WeakIterator<X>;

//------------------------------------------------------------------------------
// OutputIterator
//------------------------------------------------------------------------------
template<class X>
concept bool OutputIterator = WeakOutputIterator<X> && Iterator<X>;

//------------------------------------------------------------------------------
// WeakInputIterator
//------------------------------------------------------------------------------
template<class X>
concept bool WeakInputIterator =
  requires(X x) {
    requires WeakIterator<X>;
    requires Readable<X>;
    requires Readable<std::decay_t<decltype(x++)>>;
  };

//------------------------------------------------------------------------------
// InputIterator
//------------------------------------------------------------------------------
template<class X>
concept bool InputIterator =
  requires(X x) {
    requires WeakInputIterator<X>;
    requires Iterator<X>;
    {typename std::iterator_traits<X>::iterator_category()} 
      -> std::input_iterator_tag;
  };

//------------------------------------------------------------------------------
// ForwardIterator
//------------------------------------------------------------------------------
template<class X>
concept bool ForwardIterator =
  requires(X x) {
    requires InputIterator<X>;
    requires Incrementable<X>;
    {typename std::iterator_traits<X>::iterator_category()}
      -> std::forward_iterator_tag;
  };

//------------------------------------------------------------------------------
// BidirectionalIterator
//------------------------------------------------------------------------------
template<class X>
concept bool BidirectionalIterator =
  requires(X x) {
    requires ForwardIterator<X>;
    {--x} -> X&;
    {x--} -> X;
    requires Same<decltype(*x), decltype(*x--)>;
    {typename std::iterator_traits<X>::iterator_category()}
      -> std::bidirectional_iterator_tag;
  };

//------------------------------------------------------------------------------
// RandomAccessIterator
//------------------------------------------------------------------------------
template<class X>
concept bool RandomAccessIterator =
  requires(X x) {
    requires BidirectionalIterator<X>;
    requires TotallyOrdered<X>;
    {x - x} -> SignedIntegral;
    {x - x} -> typename std::iterator_traits<X>::difference_type;
    requires Same<decltype(x + (x - x)), decltype((x - x) + x)>;
    {x - (x - x)} -> X;
    {x += x - x} -> X&;
    {x -= x - x} -> X&;
    {x[x-x]} -> typename std::iterator_traits<X>::reference;
    {typename std::iterator_traits<X>::iterator_category()}
      -> std::random_access_iterator_tag;
  };
} //end namespace cpt

//------------------------------------------------------------------------------
// advance
//------------------------------------------------------------------------------
// v1
template <cpt::InputIterator I>
void advance(I& i, typename std::iterator_traits<I>::difference_type n) {
  assert(n >= 0);
  for (; n > 0; --n) ++i;
}

// v2
template <cpt::BidirectionalIterator I>
void advance(I& i, typename std::iterator_traits<I>::difference_type n) {
  if (n >= 0)
    for (; n > 0; --n) ++i;
  else
    for (; n < 0; ++n) --i;
}

// v3
template <cpt::RandomAccessIterator I>
void advance(I& i, typename std::iterator_traits<I>::difference_type n) {
  i += n;
}

int main() {
  std::forward_list<int> l1;
  std::list<int> l2;
  std::vector<int> v;
  auto i1 = l1.begin();
  auto i2 = l2.begin();
  auto i3 = v.begin();
  ::advance(i1, 0);
  ::advance(i2, 0);
  ::advance(i3, 0);
  return 0;
}
