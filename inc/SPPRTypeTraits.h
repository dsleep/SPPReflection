// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <memory>

namespace SPP
{
    template<typename...T>
    struct type_list
    {
        static constexpr auto size = sizeof...(T); //!< The amount of template parameters
    };

    template <typename T>
    concept HasParentClass = requires
    {
        typename T::parent_class;
    };

    template <class T>
    struct is_vector {
        static constexpr bool value = false;
    };
    template <class T, class A>
    struct is_vector<std::vector<T, A> > {
        static constexpr bool value = true;
    };


    template <class T>
    struct is_unique_ptr {
        static constexpr bool value = false;
    };
    template <class T, class D>
    struct is_unique_ptr<std::unique_ptr<T, D> > {
        static constexpr bool value = true;
    };


    template <typename T>
    concept IsSTLVector = is_vector<T>::value;

    template <typename T>
    concept IsUniquePtr = is_unique_ptr<T>::value;

    /////////////////////////////////////////////////////////////////////////////////////////
    // This trait will removes cv-qualifiers, pointers and reference from type T.
    template<typename T, typename Enable = void>
    struct raw_type
    {
        using type = std::remove_cv_t<T>;
    };

    //template<typename T> struct raw_type<T, std::enable_if_t<std::is_pointer<T>::value && !std::is_function_ptr<T>::value>>
    //{
    //    using type = typename raw_type< detail::remove_pointer_t<T>>::type;
    //};

    template<typename T> struct raw_type<T, std::enable_if_t<std::is_reference<T>::value> >
    {
        using type = typename raw_type< std::remove_reference_t<T> >::type;
    };

    template<typename T>
    using raw_type_t = typename raw_type<T>::type;

    ////////

    template<typename T, typename Enable = void>
    struct get_size_of
    {
        static constexpr std::size_t value()
        {
            return sizeof(T);
        }
    };

    template<typename T>
    struct get_size_of<T, std::enable_if_t<std::is_same<T, void>::value || std::is_function<T>::value>>
    {
        static constexpr std::size_t value()
        {
            return 0;
        }
    };

    template<typename T>
    struct is_function_ptr : std::integral_constant<bool, std::is_pointer<T>::value&&
        std::is_function<std::remove_pointer<T>>::value>
    {
    };

    template <typename T>
    struct function_traits : function_traits< decltype(&T::operator()) > {};

    template<typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        static constexpr size_t arg_count = sizeof...(Args);

        using return_type = R;
        using arg_types = type_list<Args...>;
        using arg_tuple = std::tuple<Args...>;
    };

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> { };

    template<typename R, typename... Args>
    struct function_traits<R(&)(Args...)> : function_traits<R(Args...)> { };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...)> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) volatile> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const volatile> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...) noexcept> : function_traits<R(Args...)> { };

    template<typename R, typename... Args>
    struct function_traits<R(&)(Args...) noexcept> : function_traits<R(Args...)> { };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) noexcept> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const noexcept> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) volatile noexcept> : function_traits<R(Args...)> { using class_type = C; };

    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const volatile noexcept> : function_traits<R(Args...)> { using class_type = C; };

    /////////////////////////////////////////////////////////////////////////////////////
    // use it like e.g:
    // param_types<F, 0>::type

    template<typename F, size_t Index>
    struct param_types
    {
        using type = typename std::tuple_element<Index, typename function_traits<F>::arg_tuple>::type;
    };

    template<typename F, size_t Index>
    using param_types_t = typename param_types<F, Index>::type;

    /////////////////////////////////////////////////////////////////////////////////////
    // pointer_count<T>::value Returns the number of pointers for a type
    // e.g. pointer_count<char**>::value => 2
    //      pointer_count<char*>::value  => 1
    //      pointer_count<char>::value   => 0
    template<typename T, typename Enable = void>
    struct pointer_count_impl
    {
        static constexpr std::size_t size = 0;
    };

    template<typename T>
    struct pointer_count_impl<T, std::enable_if_t<std::is_pointer<T>::value &&
        !is_function_ptr<T>::value &&
        !std::is_member_pointer<T>::value>>
    {
        static constexpr std::size_t size = pointer_count_impl< std::remove_pointer<T> >::size + 1;
    };

    template<typename T>
    using pointer_count = std::integral_constant<std::size_t, pointer_count_impl<T>::size>;
}