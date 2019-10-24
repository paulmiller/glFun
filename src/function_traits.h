#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H

#include <tuple>

// from https://functionalcpp.wordpress.com/2013/08/05/function-traits/

template<class F>
struct function_traits;

template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {};

template<class R, class... Args>
struct function_traits<R(Args...)> {
  using return_type = R;

  static constexpr std::size_t arity = sizeof...(Args);

  template<std::size_t N>
  struct argument {
    static_assert(N < arity);
    using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
  };
};

#endif
