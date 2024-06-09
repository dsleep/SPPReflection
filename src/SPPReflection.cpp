// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#include "SPPReflection.h"

namespace NameSKIP {
    static constexpr std::size_t skip_size_at_begin = 22;
    static constexpr std::size_t skip_size_at_end = 16;
}


const char* extract_type_signature(const char* signature) noexcept
{
    //    static_assert(N > skip_size_at_begin + skip_size_at_end, "RTTR is misconfigured for your compiler.")
    return &signature[NameSKIP::skip_size_at_begin];
}

std::size_t get_size(const char* s) noexcept
{
    return (std::char_traits<char>::length(s) - NameSKIP::skip_size_at_end);
}


TypeCollection::TypeCollection()
{

}

type_data* TypeCollection::Push(std::unique_ptr<type_data>&& InData)
{
    auto outTypeData = InData.get();
    type_store.push_back(std::move(InData));
    return outTypeData;
}

TypeCollection& GetTypeCollection()
{
    static TypeCollection sO;
    return sO;
}

CPPType create_type(type_data* data) noexcept
{
    return data ? CPPType(data) : CPPType();
}