// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

// RTTR credit
// 
// Much is based on processing RTTR and slimming the meat I needed from it - DS
// 
// RTTR is released under the terms of the MIT license, so it is free to use in your free or commercial projects.
// Copyright(c) 2014 - 2018 Axel Menzel info@rttr.org
// https://github.com/rttrorg/rttr

#pragma once

#include "SPPCore.h"
#include "SPPLogging.h"

//#include "json/json.h"

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <memory>
#include <array>

#if _WIN32 && !defined(SPP_REFLECTION_STATIC)
    #ifdef SPP_REFLECTION_EXPORT
        #define SPP_REFLECTION_API __declspec(dllexport)
    #else
        #define SPP_REFLECTION_API __declspec(dllimport)
    #endif
#else
    #define SPP_REFLECTION_API 
#endif

#include "SPPRDataManipulators.h"
#include "SPPRTypeTraits.h"

#define TYPE_LIST(...) type_list<__VA_ARGS__>

#define PARENT_CLASS(parentC) \
    public: \
    using parent_class = parentC; \
    protected:

#define BEFRIEND_REFL_STRUCTS       \
    template<typename Class_Type>   \
    friend struct ClassBuilder;     \
    template <typename T>           \
    friend struct TRegisterStruct;  

#define ENABLE_VF_REFL \
    public: \
    virtual CPPType GetCPPType() const { \
        using non_ref_type = typename std::remove_cv<typename std::remove_reference< decltype(*this) >::type>::type; \
        return get_type<non_ref_type>(); \
    }; \
    protected: 

#define ENABLE_REFLECTION \
    BEFRIEND_REFL_STRUCTS \
    ENABLE_VF_REFL 

#define ENABLE_REFLECTION_C(ParentClass) \
    ENABLE_REFLECTION \
    PARENT_CLASS(ParentClass) 


#define REFL_CLASS_START(InClass)       \
    {                                   \
    using _REF_CC = InClass;            \
    build_class<_REF_CC>(#InClass)

#define RC_ADD_PROP(InProp) \
    .property( #InProp, &_REF_CC::InProp )

#define RC_ADD_METHOD(InMethod) \
    .method( #InMethod, &_REF_CC::InMethod )

#define RC_ADD_CONSTRUCTOR(...) \
    .constructor< __VA_ARGS__ >()

#define REFL_CLASS_END ; } \


#define REFL_ENUM_START(InEnumType) \
    { \
        auto EnumCPP = get_type< InEnumType >(); \
        auto newEnum = std::make_unique< EnumCollection >(); 

#define RC_ENUM_VALUE(InEnumValue, InEnumString) \
        newEnum->EnumValues.push_back({ #InEnumString, (int32_t)InEnumValue });

#define REFL_ENUM_END \
        EnumCPP.GetTypeData()->enumCollection = std::move(newEnum); \
    }

namespace SPP
{
    SPP_REFLECTION_API extern LogEntry LOG_REFLECTION;

    SPP_REFLECTION_API const char* extract_type_signature(const char* signature) noexcept;

    SPP_REFLECTION_API const char* GetIndent(uint8_t InValue);


    template<typename T>
    const char* f() noexcept
    {
        return extract_type_signature(
            __FUNCSIG__
        );
    }

    SPP_REFLECTION_API std::size_t get_size(const char* s) noexcept;
    /////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    std::string get_type_name() noexcept
    {
        return std::string(f<T>(), get_size(f<T>()));
    }

    struct DataAllocation
    {
        virtual void* Construct() = 0;
    };

    template<typename T>
    struct TDataAllocation : public DataAllocation
    {
        virtual void* Construct()
        {
            return (new T());
        }
    };


    struct EnumCollection
    {
        std::vector< std::tuple< std::string, int32_t  > > EnumValues;
    };

    struct SPP_REFLECTION_API type_data
    {
        std::string name;

        std::size_t get_sizeof;
        std::size_t get_pointer_dimension;

        uint32_t is_class = 1;
        uint32_t is_enum : 1;
        uint32_t is_array : 1;
        uint32_t is_const : 1;
        uint32_t is_volatile : 1;
        uint32_t is_pointer : 1;
        uint32_t is_arithmetic : 1;
        uint32_t is_function : 1;
        uint32_t is_member_object_pointer : 1;
        uint32_t is_member_function_pointer : 1;
        uint32_t is_reference : 1;
        uint32_t is_lvalue_reference : 1;
        uint32_t is_rvalue_reference : 1;

        type_data* raw_type_data = nullptr;

        std::unique_ptr< struct DataAllocation > dataAllocation;
        std::unique_ptr< struct ArrayManipulator > arrayManipulator;
        std::unique_ptr< struct WrapManipulator > wrapManipulator;
        std::unique_ptr< struct EnumCollection > enumCollection;
        std::unique_ptr< class ReflectedStruct > structureRef;
    };

    class SPP_REFLECTION_API CPPType
    {
    public:
        CPPType() : _typeData(nullptr) {}

        CPPType(type_data* data) noexcept
            : _typeData(data)
        {
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        CPPType(const CPPType& other) noexcept
            : _typeData(other._typeData)
        {
        }

        type_data* GetTypeData() const
        {
            return _typeData;
        }

        bool ConvertibleTo(const CPPType& InValue) const
        {
            if (_typeData == InValue._typeData)
            {
                return true;
            }

            //NOT A GOOD CHEAT TODO, CREATE A CONVE
            if (_typeData == InValue._typeData->raw_type_data ||
                _typeData->raw_type_data == InValue._typeData)
            {
                return true;
            }

            return false;
        }

        bool DerivedFrom(const CPPType& InValue) const;

        bool operator==(const CPPType& InValue) const
        {
            return (_typeData == InValue._typeData);
        }
        bool operator!=(const CPPType& InValue) const
        {
            return !(*this == InValue);
        }

    private:
        type_data* _typeData;
    };

    /////////////////////////////////////////////////////////////////////

    //FORWARD DECLARE
    template<typename T>
    std::unique_ptr<type_data> make_type_data();

    class SPP_REFLECTION_API TypeCollection
    {
        NO_COPY_ALLOWED(TypeCollection);

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;

    public:
        TypeCollection();
        type_data* Push(std::unique_ptr<type_data>&& InData);
        type_data* GetType(const char* InName);
    };

    SPP_REFLECTION_API TypeCollection& GetTypeCollection();
    SPP_REFLECTION_API CPPType create_type(type_data* data) noexcept;

    template<typename T>
    using is_complete_type = std::integral_constant<bool, !std::is_function<T>::value && !std::is_same<T, void>::value>;

    template<typename T> requires is_complete_type<T>::value
        CPPType create_or_get_type() noexcept
    {
        // when you get an error here, then the type was not completely defined
        // (a forward declaration is not enough because base_classes will not be found)
        using type_must_be_complete = char[sizeof(T) ? 1 : -1];
        (void)sizeof(type_must_be_complete);
        static const CPPType val = create_type(GetTypeCollection().Push(make_type_data<T>()));
        return val;
    }

    /////////////////////////////////////////////////////////////////////////////////

    template<typename T> requires (!is_complete_type<T>::value)
        CPPType create_or_get_type() noexcept
    {
        static const CPPType val = create_type(GetTypeCollection().Push(make_type_data<T>()));
        return val;
    }

    template<typename T>
    CPPType get_type() noexcept
    {
        return create_or_get_type<T>();
    }

    template<>
    CPPType get_type<void>() noexcept
    {
        static const CPPType val = create_type(nullptr);
        return val;
    }

    CPPType get_type_by_name(const char* InString);

    template<typename T>
    std::unique_ptr<type_data> make_type_data()
    {
        auto obj = std::unique_ptr<type_data>
            (
                new type_data
                {
                    get_type_name<T>(),
                    get_size_of<T>::value(),
                    pointer_count<T>::value,

                    std::is_class_v<T>, //is_class
                    std::is_enum_v<T>, //is_enum
                    std::is_array_v<T>, //is_array
                    std::is_const_v<T>, //is_const
                    std::is_volatile_v<T>, //is_volatile
                    std::is_pointer_v<T>, //is_pointer
                    std::is_arithmetic_v<T>, //is_arithmetic
                    std::is_function_v<T>, //is_function_pointer
                    std::is_member_object_pointer_v<T>, //is_member_object_pointer
                    std::is_member_function_pointer_v<T>, //is_member_function_pointer
                    std::is_reference_v<T>, //is_reference
                    std::is_lvalue_reference_v<T>, //is_lvalue_reference
                    std::is_rvalue_reference_v<T> //is_rvalue_reference
                }
            );

        if constexpr (IsSTLVector<T>)
        {
            //value_type
            obj->arrayManipulator = std::make_unique< TArrayManipulator< T > >();
        }

        if constexpr (IsUniquePtr<T>)
        {
            //element_type
            obj->wrapManipulator = std::make_unique< TWrapManipulator< T > >();
        }

        if constexpr (!std::is_same_v<T, typename raw_type<T>::type >)
        {
            obj->raw_type_data = get_type<typename raw_type<T>::type>().GetTypeData();
        }

        return obj;
    }

    class ReflectedStruct;
    class ReflectedProperty;

    

    struct IVisitor
    {
        virtual bool EnterStructure(const ReflectedStruct& inValue) { return true; }
        virtual void ExitStructure(const ReflectedStruct& inValue) {}

        virtual bool EnterProprety(const ReflectedProperty& inValue) { return true; }
        virtual void ExitProprety(const ReflectedProperty& inValue) { }

        // ARRAY
        virtual void BeginArray(const ReflectedProperty& inValue) { }
        virtual void BeginArrayItem(size_t InIdx) { }
        virtual void EndArrayItem(size_t InIdx) { }
        virtual void EndArray(const ReflectedProperty& inValue) { }

        virtual bool DataTypeResolved(const CPPType& inValue) { return false; }    

        template<typename T>
        void VisitValue(const ReflectedProperty& InProperty, T& InValue) {}
    };

    

    ////////////////////////////////////////////
    //
    // PROPRTIES
    // 
    ////////////////////////////////////////////

    class SPP_REFLECTION_API ReflectedProperty
    {
        BEFRIEND_REFL_STRUCTS

    protected:
        std::string _name;
        CPPType _type;
        size_t _propOffset = 0;

    public:
        ReflectedProperty(const std::string &InName, CPPType InType, size_t InOffset = 0) : _name(InName), _type(InType), _propOffset(InOffset) {}
        virtual ~ReflectedProperty() {}

        const auto& GetName() const { return _name; }
        auto GetPropOffset() const { return _propOffset; }

        virtual void Visit(void* InStruct, IVisitor* InVisitor) {}
        virtual void LogOut(void* structAddr, int8_t Indent = 0) {}
    };

    class SPP_REFLECTION_API StringProperty : public ReflectedProperty
    {
    public:
        StringProperty(const std::string& InName, CPPType InType, size_t InOffset = 0) :
            ReflectedProperty(InName, InType, InOffset) {}
        virtual ~StringProperty() {}

        std::string* AccessValue(void* structAddr)
        {
            return (std::string*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            auto& value = *AccessValue(InStruct);
            VisitValue(*this, value);
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sString: %s", GetIndent(Indent), AccessValue(structAddr)->c_str());
        }
    };

    template<typename T>
    class TNumericalProperty : public ReflectedProperty
    {
    public:
        TNumericalProperty(const std::string& InName, CPPType InType, size_t InOffset = 0) :
            ReflectedProperty(InName, InType, InOffset) {}
        virtual ~TNumericalProperty() {}

        T* AccessValue(void* structAddr)
        {
            return (T*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            auto& value = *AccessValue(InStruct);
            VisitValue(*this, value);
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sNumber: %s", GetIndent(Indent), std::to_string(*AccessValue(structAddr)).c_str());
        }
    };

    class SPP_REFLECTION_API EnumProperty : public ReflectedProperty
    {
    public:
        EnumProperty(const std::string& InName, CPPType InType, size_t InOffset = 0) :
            ReflectedProperty(InName, InType, InOffset) {}

        virtual ~EnumProperty() {}

        int32_t* AccessValue(void* structAddr)
        {
            return (int32_t*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            auto& value = *AccessValue(InStruct);
            VisitValue(*this, value);
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            auto curValue = *AccessValue(structAddr);

            SE_ASSERT(_type.GetTypeData()->enumCollection);

            for (const auto& pairs : _type.GetTypeData()->enumCollection->EnumValues)
            {
                if (curValue == std::get<1>(pairs))
                {
                    SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sEnum Value: %s", GetIndent(Indent), std::get<0>(pairs).c_str());
                }
            }

            SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sUnknown enum value", GetIndent(Indent));
        }
    };

    class SPP_REFLECTION_API DynamicArrayProperty : public ReflectedProperty
    {
        BEFRIEND_REFL_STRUCTS

    protected:
        std::unique_ptr<ReflectedProperty> _inner;

    public:
        DynamicArrayProperty(const std::string& InName, CPPType InType, 
            std::unique_ptr<ReflectedProperty> && InInner,
            size_t InOffset = 0) :
            ReflectedProperty(InName, InType, InOffset), _inner(std::move(InInner)) {}
        virtual ~DynamicArrayProperty() {}

        void* AccessValue(void* structAddr)
        {
            return (void*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            SE_ASSERT(_type.GetTypeData()->arrayManipulator);

            InVisitor->BeginArray(*this);

            auto arrayAddr = AccessValue(InStruct);
            auto totalSize = _type.GetTypeData()->arrayManipulator->Size(arrayAddr);
            for (size_t Iter = 0; Iter < totalSize; Iter++)
            {
                InVisitor->BeginArrayItem(Iter);
                _inner->Visit(_type.GetTypeData()->arrayManipulator->Element(arrayAddr, (int32_t)Iter), InVisitor);
                InVisitor->EndArrayItem(Iter);
            }

            InVisitor->EndArray(*this);
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            SE_ASSERT(_type.GetTypeData()->arrayManipulator);

            auto arrayAddr = AccessValue(structAddr);
            auto totalSize = _type.GetTypeData()->arrayManipulator->Size(arrayAddr);

            SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sARRAY: size: %zd", GetIndent(Indent), totalSize);
            for (size_t Iter = 0; Iter < totalSize; Iter++)
            {
                SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sIDX: %zd", GetIndent(Indent), Iter);
                _inner->LogOut(_type.GetTypeData()->arrayManipulator->Element(arrayAddr, (int32_t)Iter), Indent + 1);
            }
        }
    };

    class SPP_REFLECTION_API UniquePtrProperty : public ReflectedProperty
    {
        BEFRIEND_REFL_STRUCTS

    protected:
        std::unique_ptr<ReflectedProperty> _inner;

    public:
        UniquePtrProperty(const std::string& InName, 
            CPPType InType, 
            std::unique_ptr<ReflectedProperty> && InInner, 
            size_t InOffset = 0) :
            ReflectedProperty(InName, InType, InOffset), _inner(std::move(InInner)) {}
        virtual ~UniquePtrProperty() {}

        void* AccessValue(void* structAddr)
        {
            return (void*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            SE_ASSERT(_type.GetTypeData()->wrapManipulator);
            auto uniquePtrAddr = AccessValue(InStruct);
            if (_type.GetTypeData()->wrapManipulator->IsValid(uniquePtrAddr))
            {
                _inner->Visit(_type.GetTypeData()->wrapManipulator->GetValue(uniquePtrAddr), InVisitor);
            }
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            SE_ASSERT(_type.GetTypeData()->wrapManipulator);

            auto uniquePtrAddr = AccessValue(structAddr);

            if (_type.GetTypeData()->wrapManipulator->IsValid(uniquePtrAddr))
            {
                SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sUNIQUE_PTR: ", GetIndent(Indent));
                _inner->LogOut(_type.GetTypeData()->wrapManipulator->GetValue(uniquePtrAddr));
            }
        }
    };


    ////////////////////////////////////////////
    //
    // METHODS
    // 
    ////////////////////////////////////////////

    struct SPP_REFLECTION_API Argument
    {
        CPPType type;
        void* reference = nullptr;

        Argument() {}
        Argument(CPPType InType, void* InRef) : type(InType), reference(InRef) {}

        template<typename T>
        T* GetValue() const
        {
            return (T*)reference;
        }
    };

    class SPP_REFLECTION_API ReflectedMethod
    {
        BEFRIEND_REFL_STRUCTS

    protected:
        std::string _name;

        CPPType _type;
        CPPType _returnType;
        std::vector< CPPType > _propertyTypes;
        std::function<void(void*, Argument&, const std::vector< Argument >&)> _method;

    public:
        const auto& GetName() const { return _name; }
        const auto& GetCaller() const { return _method; }
        const auto& GetArgTypes() const { return _propertyTypes; }
        const auto& GetReturnType() const { return _returnType; }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) {}
    };


    ////////////////////////////////////////////
    //
    // Structure/Class
    // 
    ////////////////////////////////////////////

    class SPP_REFLECTION_API ReflectedStruct
    {
        BEFRIEND_REFL_STRUCTS
        NO_COPY_ALLOWED(ReflectedStruct);

    protected:
        CPPType _type;
        ReflectedStruct* _parent = nullptr;

        std::vector< std::unique_ptr<ReflectedProperty> > _properties;
        std::vector< std::unique_ptr<ReflectedMethod> > _methods;
        std::vector< std::unique_ptr<ReflectedMethod> > _constructors;

    public:
        ReflectedStruct() {}

        virtual void LogOut(void* structAddr, int8_t Indent = 0)
        {
            auto curStruct = this;

            while (curStruct)
            {
                for (const auto& curProp : curStruct->_properties)
                {
                    SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sNAME: %s OFFSET: %zd", GetIndent(Indent), curProp->GetName().c_str(), curProp->GetPropOffset());
                    curProp->LogOut(structAddr, Indent + 1);
                }

                curStruct = curStruct->_parent;
            }
        }

        bool DerivedFrom(const CPPType& InValue) const
        {
            auto curStruct = this;

            while (curStruct)
            {
                if (curStruct->_type == InValue)
                {
                    return true;
                }
                curStruct = curStruct->_parent;
            }

            return false;
        }

        void Visit(void* InStruct, struct IVisitor* InVisitor);

        template<typename Ret, typename ...Args>
        Ret Invoke(void* structAddr, const std::string& MethodName, Args&& ...args) const
        {
            auto curStruct = this;

            std::vector< Argument > arguments;
            (arguments.push_back(Argument(get_type< std::remove_reference_t< decltype(args) > >(), (void*)&args)), ...);

            while (curStruct)
            {
                const auto& methodsToIter = (structAddr ? curStruct->_methods : curStruct->_constructors);

                for (const auto& method : methodsToIter)
                {
                    if (method->GetName() == MethodName)
                    {
                        const auto returnType = get_type < Ret >();
                        if (returnType != method->GetReturnType())
                        {
                            continue;
                        }

                        const auto argsTypes = method->GetArgTypes();
                        if (arguments.size() == argsTypes.size())
                        {
                            bool bValid = true;
                            for (size_t Iter = 0; Iter < arguments.size(); Iter++)
                            {
                                //TODO: needs to be a way to be more like std::is_convertible_v
                                if (!arguments[Iter].type.ConvertibleTo(argsTypes[Iter]))
                                {
                                    bValid = false;
                                    break;
                                }
                            }

                            if (bValid)
                            {
                                if constexpr (std::is_same<Ret, void>::value)
                                {
                                    Argument returnArgument(get_type < void >(), nullptr);
                                    method->GetCaller()(structAddr, returnArgument, arguments);
                                    return;
                                }
                                else
                                {
                                    Ret oVal = { };
                                    Argument returnArgument(get_type < Ret >(), (void*)&oVal);
                                    method->GetCaller()(structAddr, returnArgument, arguments);
                                    return oVal;
                                }
                            }
                        }
                    }
                }

                curStruct = curStruct->_parent;
            }

            if constexpr (!std::is_same<Ret, void>::value)
            {
                return {};
            }
        }

        template<typename Ret, typename ...Args>
        Ret Invoke_Constructor(Args&& ...args) const
        {
            static_assert(!std::is_same_v<Ret, void>);
            return Invoke<Ret>(nullptr, std::string("constructor"), std::forward<Args>(args)...);
        }
    };

    // For nested structures
    class SPP_REFLECTION_API StructProperty : public ReflectedProperty
    {
        BEFRIEND_REFL_STRUCTS

    protected:
        ReflectedStruct* _struct = nullptr;

    public:
        StructProperty(const std::string& InName, CPPType InType, size_t InOffset = 0) : 
            ReflectedProperty(InName, InType, InOffset)
        {
            _struct = InType.GetTypeData()->structureRef.get();
            SE_ASSERT(_struct);
        }
        virtual ~StructProperty() {}

        void* AccessValue(void* structAddr)
        {
            return (void*)((uint8_t*)structAddr + _propOffset);
        }

        virtual void Visit(void* InStruct, IVisitor* InVisitor)
        {
            auto newOffset = AccessValue(InStruct);
            _struct->Visit(newOffset, InVisitor);
        }

        virtual void LogOut(void* structAddr, int8_t Indent = 0) override
        {
            SPP_LOG(LOG_REFLECTION, LOG_INFO, "%sSTRUCT PROP", GetIndent(Indent));
            auto newOffset = AccessValue(structAddr);
            _struct->LogOut(newOffset, Indent + 1);
        }
    };


    template<typename T, typename U>
    constexpr size_t offsetOf(U T::* member)
    {
        return (char*)&((T*)nullptr->*member) - (char*)nullptr;
    }

    template<typename Func, std::size_t... Is>
    std::vector< CPPType > getmethodargs(std::index_sequence<Is...>)
    {
        using arg_tuple = typename function_traits<Func>::arg_tuple;
        std::vector< CPPType > types;
        (types.push_back(get_type< std::tuple_element_t<Is, arg_tuple> >()), ...);
        return types;
    }


    template<typename T, typename ClassSet> requires (std::is_arithmetic_v<T>)
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, T ClassSet::* prop)
    {
        auto calcOffset = offsetOf(prop);
        auto curType = get_type<T>();
        auto newProp = std::make_unique< TNumericalProperty<T> >(InName, curType, calcOffset);
        return std::move(newProp);
    }

    template<typename T, typename ClassSet> requires (std::is_same_v<std::string, T>)
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, T ClassSet::* prop)
    {
        auto calcOffset = offsetOf(prop);
        auto curType = get_type<T>();
        auto newProp = std::make_unique< StringProperty >(InName, curType, calcOffset);
        return std::move(newProp);
    }

    template<typename T, typename ClassSet> requires (std::is_enum_v<T>)
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, T ClassSet::* prop)
    {
        static_assert(sizeof(T) == sizeof(int32_t));
        auto calcOffset = offsetOf(prop);
        auto curType = get_type<T>();
        auto newProp = std::make_unique< EnumProperty >(InName, curType, calcOffset);
        return std::move(newProp);
    }    

    template<typename T, typename ClassSet>
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, std::vector<T> ClassSet::* prop)
    {
        auto arraytype = get_type< std::vector<T> >();

        struct Dummy
        {
            T inner;
        };

        auto newProp = std::make_unique< DynamicArrayProperty >(InName, arraytype, CreateProperty("inner", &Dummy::inner), offsetOf(prop));
        return std::move(newProp);
    }

    template<typename T, typename ClassSet>
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, std::unique_ptr<T> ClassSet::* prop)
    {
        auto propType = get_type< std::unique_ptr<T> >();

        struct Dummy
        {
            T inner;
        };

        auto newProp = std::make_unique< UniquePtrProperty >(InName, propType, CreateProperty("inner", &Dummy::inner), offsetOf(prop));
        return std::move(newProp);
    }

    template<typename T, typename ClassSet>
    concept C_CreateProperty = requires (T ClassSet:: * prop)
    {
        CreateProperty<T, ClassSet>("", prop);
    };

    // if it failed at the rest try to make this a normal struct, maybe make it smart?
    // this will fail if it is NOT a defined reflected struct
    template<typename T, typename ClassSet> requires (!C_CreateProperty<T, ClassSet>)
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, T ClassSet::* prop)
    {
        auto calcOffset = offsetOf(prop);
        auto curType = get_type<T>();
        auto newProp = std::make_unique< StructProperty >(InName, curType, calcOffset);
        return std::move(newProp);
    }

    template<typename Class_Type>
    struct ClassBuilder
    {
        std::unique_ptr< ReflectedStruct > _class;

        template<typename U = Class_Type> requires HasParentClass<U>
        void LinkParents()
        {
            CPPType parentClass = get_type< typename U::parent_class >();

            if (parentClass.GetTypeData()->structureRef)
            {
                _class->_parent = parentClass.GetTypeData()->structureRef.get();
            }
            else
            {
                //TODO check link order at startup? or each go?
            }
        }

        template<typename ClassType, typename Func, std::size_t... Is>
        static inline void invoke(ClassType* BaseObject, Func func_ptr, Argument& retArg, const std::vector< Argument >& arguments, std::index_sequence<Is...>)
        {
            using arg_tuple = typename function_traits<Func>::arg_tuple;
            using return_type = typename function_traits<Func>::return_type;

            if constexpr (std::is_same<return_type, void>::value)
            {
                (BaseObject->*func_ptr)(*arguments[Is].GetValue< typename std::tuple_element_t<Is, arg_tuple> >()...);
            }
            else
            {
                SE_ASSERT(retArg.reference);
                *(return_type*)retArg.reference = (BaseObject->*func_ptr)
                    (*arguments[Is].GetValue< typename std::remove_reference< typename std::tuple_element_t<Is, arg_tuple> >::type >()...);
            }
        }

        template<typename ClassType, typename ArgTuple, std::size_t... Is>
        static inline void invoke_constructor(Argument& retArg, const std::vector< Argument >& arguments, std::index_sequence<Is...>)
        {
            SE_ASSERT(retArg.reference);
            *(ClassType**)retArg.reference = new ClassType(
                *arguments[Is].GetValue< typename std::remove_reference< typename std::tuple_element_t<Is, ArgTuple> >::type >()...);
        }

        template<typename U = Class_Type> requires (!HasParentClass<U>)
            void LinkParents() {}

        ClassBuilder()
        {
            _class = std::make_unique< ReflectedStruct >();
            LinkParents();
        }

        ~ClassBuilder()
        {
            CPPType classType = get_type< Class_Type >();
            classType.GetTypeData()->structureRef = std::move(_class);
        }
                
        template<typename T> //requires C_CreateProperty<T, Class_Type>
        ClassBuilder& property(const char* InName, T Class_Type::* prop)
        {
            _class->_properties.push_back(CreateProperty(InName, prop));
            return *this;
        }

        //template<typename T> requires (!C_CreateProperty<T, Class_Type>)
        //ClassBuilder& property(const char* InName, T Class_Type::* prop)
        //{
        //    auto calcOffset = offsetOf(prop);
        //    auto curType = get_type<T>();
        //    // will assert if not struct
        //    auto newProp = std::make_unique< StructProperty >(InName, curType, calcOffset);
        //    _class->_properties.push_back(std::move(newProp));
        //    return *this;
        //}

       

        template<typename Func>
        ClassBuilder& method(const char* InName, Func Class_Type::* method)
        {
            static_assert(std::is_function_v<Func>);

            using return_type = typename function_traits<Func>::return_type;
            using arg_tuple = typename function_traits<Func>::arg_tuple;
            using integer_sequence = std::make_index_sequence< std::tuple_size_v<arg_tuple> >;

            auto retType = get_type< return_type >();
            auto methodArgs = getmethodargs< Func >(integer_sequence{});

            auto callMethod = [method](void* InClassAddr, Argument& retArg, const std::vector< Argument >& arguments) -> void
            {
                auto classVal = ((Class_Type*)InClassAddr);
                if (arguments.size() == function_traits<Func>::arg_count)
                {
                    invoke(classVal, method, retArg, arguments, integer_sequence{});
                }
            };

            auto newMethod = std::make_unique< ReflectedMethod >();
            newMethod->_name = InName;
            newMethod->_returnType = retType;
            newMethod->_propertyTypes = methodArgs;
            newMethod->_type = get_type< Func >();
            newMethod->_method = callMethod;

            _class->_methods.push_back(std::move(newMethod));

            return *this;
        }

        template<typename ...Args>
        ClassBuilder& constructor()
        {
            constexpr size_t ArgCount = sizeof...(Args);
            using arg_tuple = std::tuple<Args...>;
            using integer_sequence = std::make_index_sequence< std::tuple_size_v<arg_tuple> >;

            auto retType = get_type< Class_Type* >();
            std::vector< CPPType > methodArgs;
            (methodArgs.push_back(get_type< Args >()), ...);

            auto callMethod = [](void* InClassAddr, Argument& retArg, const std::vector< Argument >& arguments) -> void
            {
                if (arguments.size() == ArgCount)
                {
                    invoke_constructor<Class_Type, arg_tuple>(retArg, arguments, integer_sequence{});
                }
            };

            auto newMethod = std::make_unique< ReflectedMethod >();
            newMethod->_name = "constructor";
            newMethod->_returnType = retType;
            newMethod->_propertyTypes = methodArgs;
            newMethod->_type = get_type< type_list<Args...> >();
            newMethod->_method = callMethod;

            _class->_constructors.push_back(std::move(newMethod));

            return *this;
        }
    };

    template<typename Class_Type>
    ClassBuilder< Class_Type> build_class(std::string_view name)
    {
        return ClassBuilder< Class_Type>();
    }    
}