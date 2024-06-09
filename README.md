# SPPReflection
Simple C++20 game driven reflection.

## What?
Reflection is used to gather information (traditionally at runtime - while the application executes) of your class, functions, variables, etc.

## Why?
Reflection can help abstract things in C++ that aren't traditionally abstractable... Serialization, replication, mirroring into a scripting system or editor interface.

## Examples
RTTR - https://github.com/rttrorg/rttr
reflect-cpp - https://github.com/getml/reflect-cpp

## How
Well this is the part up for much debate and can engulf you into the darkest and wackiest part of C++. While C++ can often have a direct correlation into how the operating system will operate (pointers, sequential arrays, etc.) or macros and template to help in reducing repeate code, TMP (template meta programming) and just general type manipulation (erasure, casting, dynamic casting) in C++ can be a strange nightmarish hellscape that is not entirely worthy of the general programmers time.

So we'll dabble in this post apocolpytic hellscape as little as we can, and hence why I'm going with C++20. In C++20 there is the additon of Constraints and concepts (https://en.cppreference.com/w/cpp/language/constraints) and prior to that its all about SFINAE (substitution failure is not an error https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error).

### Step 1: Types, get your types here!

What I want... 
```
std::map< c++type, MyMetaInfo > _allTypes;
```
Can I?
```
std::map< typename, MyMetaInfo > _allTypes;
```
Or something on those lines... The answer is yes, but not that way.

Ever seen a C++ singleton example?
```
MyStruct &GetSingleton()
{
  static MyStruct staticOut;
  return staticOut;
}
```
Why does this work? It works due to static only running once and building a single "instance" of your MyStruct that is now returned on every subsequent call. No magic here and if you are a 40+ year old semi-retired game dev you'll have seen this more than a few times. But you are saying you don't need a singleton you need a type to meta struct... Whelp onto the next step
```
template<typename T> 
CPPType create_or_get_type() 
{
    static const CPPType val = create_cpptype_with_metainfo<T>();
    return val;
}
```
Looks like we just made ourselves a "create_or_get_type<ANYTYPE>()" and it will get us a unique CPPType singleton based on the type given. So in a way we just made our "map< c++type, metainfo >"

# TODO more to come
