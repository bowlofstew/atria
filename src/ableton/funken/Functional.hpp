// Copyright: 2014, Ableton AG, Berlin. All rights reserved.

#include <ableton/estd/type_traits.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/reverse_fold.hpp>
#include <tuple>

namespace ableton {
namespace funken {

namespace detail {

struct ComposedReducer
{
  template <typename State, typename Fn>
  auto operator()(State&& state, Fn&& next)
    -> decltype(next(std::forward<State>(state)))
  {
    return next(std::forward<State>(state));
  }
};

template<typename Fn, typename ...Fns>
struct Composed : std::tuple<Fn, Fns...>
{
  using std::tuple<Fn, Fns...>::tuple;

  std::tuple<Fn, Fns...>& asTuple() { return *this; }
  const std::tuple<Fn, Fns...>& asTuple() const { return *this; }

  template <typename Arg>
  auto operator() (Arg&& arg) const
    -> decltype(boost::fusion::reverse_fold(
                  asTuple(), arg, ComposedReducer{}))
  {
    return boost::fusion::reverse_fold(
      asTuple(), arg, ComposedReducer{});
  }
};

} // namespace detail

//!
// Returns an object *g* that composes all the given functions *f_i*,
// such that:
//                 g(x) = f_1(f_2(...f_n(x)))
//
template <typename Fn, typename ...Fns>
auto comp(Fn&& fn, Fns&& ...fns)
  -> detail::Composed<estd::decay_t<Fn>, estd::decay_t<Fns>...>
{
  return { std::forward<Fn>(fn), std::forward<Fns>(fns)... };
};

//!
// Similar to clojure.core/identity
//
constexpr struct Identity
{
  template <typename ArgT>
  constexpr auto operator() (ArgT&& x) const
    -> decltype(std::forward<ArgT>(x))
  {
    return std::forward<ArgT>(x);
  }
} identity {};

//!
// Function that forwards its argument if only one element passed,
// otherwise it makes a tuple.
//
constexpr struct Tuplify
{
  template <typename InputT>
  constexpr auto operator() (InputT&& in) const
    -> decltype(std::forward<InputT>(in))
  {
    return std::forward<InputT>(in);
  }

  template <typename ...InputTs>
  constexpr auto operator() (InputTs&& ...ins) const
    -> estd::enable_if_t<
      (sizeof...(InputTs) > 1),
      decltype(std::make_tuple(std::forward<InputTs>(ins)...))
    >
  {
    return std::make_tuple(std::forward<InputTs>(ins)...);
  }
} tuplify {};

} // namespace funken
} // namespace ableton