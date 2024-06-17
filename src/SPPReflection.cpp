// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#include "SPPReflection.h"
#include <mutex>

namespace SPP
{
    LogEntry LOG_REFLECTION("REFL");

    static std::array< const char*, 10 > constexpr CONST_Indents = {
            "",
            "  ",
            "    ",
            "      ",
            "        ",
            "          ",
            "            ",
            "              ",
            "                ",
            "                  "
    };

    const char* GetIndent(uint8_t InValue)
    {
        return CONST_Indents[InValue % 10];
    }

    namespace NameSKIP {
        static constexpr std::size_t skip_size_at_begin = 27;
        static constexpr std::size_t skip_size_at_end = 16;
    }

    //INFLUENCED BY NAMESPACE AND FUNCTION "F"
    const char* extract_type_signature(const char* signature) noexcept
    {
        //    static_assert(N > skip_size_at_begin + skip_size_at_end, "RTTR is misconfigured for your compiler.")
        return &signature[NameSKIP::skip_size_at_begin];
    }

    std::size_t get_size(const char* s) noexcept
    {
        return (std::char_traits<char>::length(s) - NameSKIP::skip_size_at_end);
    }

    struct TypeCollection::Impl
    {
        //std::mutex storeAccess;
        std::vector< std::unique_ptr<type_data> > type_store;        
    };

    TypeCollection::TypeCollection() : _impl(new Impl())
    {

    }

    type_data* TypeCollection::Push(std::unique_ptr<type_data>&& InData)
    {
        auto outTypeData = InData.get();
        _impl->type_store.push_back(std::move(InData));
        return outTypeData;
    }

    type_data* TypeCollection::GetType(const char* InName)
    {
        for (const auto& curType : _impl->type_store)
        {
            return curType.get();
        }
        return nullptr;
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

    CPPType get_type_by_name(const char* InString)
    {
        return CPPType(GetTypeCollection().GetType(InString));
    }

    bool CPPType::DerivedFrom(const CPPType& InValue) const
    {
        if (_typeData->structureRef)
        {
            return _typeData->structureRef->DerivedFrom(InValue);
        }
        return false;
    }
}