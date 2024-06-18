// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#pragma once

#include <iostream>

// move to private include
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 4996 )
//conversion from ... possible loss of data
#pragma warning( disable : 4100 )
#endif

#define SPP_CAT_IMPL(a, b) a##b
#define SPP_CAT(a, b) SPP_CAT_IMPL(a, b)

template <typename T>
struct TRegisterStruct {};

#define SPP_AUTOREG_START                                                           \
namespace SPP_CAT(spp_auto_reg_namespace_, __LINE__)								\
{																					\
    struct RegStruct {};                                                            \
    template<>                                                                      \
    struct TRegisterStruct< RegStruct >                                             \
    {                                                                               \
        TRegisterStruct() {

#define SPP_AUTOREG_END		\
		} \
	};						\
	const static TRegisterStruct< RegStruct > _reg;			\
}

#define NO_COPY_ALLOWED(ClassName)						\
	ClassName(ClassName const&) = delete;				\
	ClassName& operator=(ClassName const&) = delete;

#define NO_MOVE_ALLOWED(ClassName)						\
	ClassName(ClassName&&) = delete;					\
	ClassName& operator=(ClassName&&) = delete;	


#define SE_CRASH_BREAK *reinterpret_cast<int32_t*>(3) = 0xDEAD;
#define SE_ASSERT(x) { if(!(x)) { SE_CRASH_BREAK; } } 
#define SPP_LOG(cat,level,S, ...) printf(S, ##__VA_ARGS__); printf("\r\n")

template <typename T, std::size_t N>
constexpr std::size_t ARRAY_SIZE(const T(&)[N]) { return N; }
