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

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <memory>

#if _WIN32 && !defined(SPP_REFLECTION_STATIC)
    #ifdef SPP_REFLECTION_EXPORT
        #define SPP_REFLECTION_API __declspec(dllexport)
    #else
        #define SPP_REFLECTION_API __declspec(dllimport)
    #endif
#else
    #define SPP_REFLECTION_API 
#endif


//////////////////////////////////////////////////
//UGLY COPIES FROM OTHER CORE HEADERS
//////////////////////////////////////////////////

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

//////////////////////////////////////////////////
//END UGLY COPIES
//////////////////////////////////////////////////

SPP_REFLECTION_API const char* extract_type_signature(const char* signature) noexcept;
SPP_REFLECTION_API std::size_t get_size(const char* s) noexcept;
/////////////////////////////////////////////////////////////////////////////////

template<typename T>
const char* f() noexcept
{
    return extract_type_signature(
        __FUNCSIG__
    );
}

/////////////////////////////////////////////////////////////////////////////////

template<typename T>
std::string get_type_name() noexcept
{
    return std::string(f<T>(), get_size(f<T>()));
}

#include "SPPRDataManipulators.h"
#include "SPPRTypeTraits.h"

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

struct SPP_REFLECTION_API type_data
{
    std::string name;

    std::size_t get_sizeof;
    std::size_t get_pointer_dimension;

    std::unique_ptr< DataAllocation > dataAllocation;
    std::unique_ptr< ArrayManipulator > arrayManipulator;
    std::unique_ptr< WrapManipulator > wrapManipulator;

    std::unique_ptr< class ReflectedStruct > structureRef;

    bool bInfoSet = false;

    bool IsFullyFormed() const;
};


class SPP_REFLECTION_API CPPType
{
public:
    CPPType() : _typeData(nullptr) {}

    CPPType(type_data * data) noexcept
        : _typeData(data)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    CPPType(const CPPType& other) noexcept
        : _typeData(other._typeData)
    {
    }

    type_data* GetTypeData()
    {
        return _typeData;
    }

    bool operator==(const CPPType& InValue) const
    {
        return _typeData == InValue._typeData;
    }
    bool operator!=(const CPPType& InValue) const
    {
        return !(*this == InValue);
    }

private:
    type_data* _typeData;
};

/////////////////////////////////////////////////////////////////////////////////




#define TYPE_LIST(...) type_list<__VA_ARGS__>


#define PARENT_CLASS(parentC) using parent_class = parentC;

#define BEFRIEND_REFL_STRUCTS       \
    template<typename Class_Type>   \
    friend struct ClassBuilder;     \
    template <typename T>           \
    friend struct TRegisterStruct;


#define ENABLE_VF_REFL \
    virtual CPPType GetCPPType() const { \
        using non_ref_type = typename std::remove_cv<typename std::remove_reference< decltype(*this) >::type>::type; \
        return get_type<non_ref_type>(); \
    };

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

#define REFL_CLASS_END ; } \
    

//
//#define ENABLE_REFLECTION() \
//    BEFRIEND_REFL_STRUCTS() \
//    ENABLE_VF_REFL() \

/////////////////////////////////////////////////////////////////////////////////////////




/////////////////
/////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////



template<typename T>
std::unique_ptr<type_data> make_type_data()
{
    auto obj = std::unique_ptr<type_data>
        (
            new type_data
            {
                get_type_name<T>(), 
                get_size_of<T>::value(),
                pointer_count<T>::value
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

    return obj;
}

class SPP_REFLECTION_API TypeCollection
{
    NO_COPY_ALLOWED(TypeCollection);

private:
    std::vector< std::unique_ptr<type_data> > type_store;

public:
    TypeCollection();
    type_data* Push(std::unique_ptr<type_data>&& InData);
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
    //using non_ref_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;    
    return create_or_get_type<T>();
}

template<>
CPPType get_type<void>() noexcept
{
    static const CPPType val = create_type(nullptr);
    return val;
}

/////////////////////////////////////////////////////////////////////////////////

// HAAXXORS to fake your objects
struct ObjectBase
{
    std::string _baseName;
    
    ObjectBase() {}

    virtual CPPType GetCPPType() const { return get_type < ObjectBase >(); }
    virtual ~ObjectBase() {}
};

/////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////
//
// PROPRTIES
// 
////////////////////////////////////////////

class ReflectedProperty
{
    template<typename Class_Type>
    friend struct ClassBuilder;

protected:
    std::string _name;
    size_t _propOffset = 0;
    CPPType _type;

public:
    ReflectedProperty() {}
    virtual ~ReflectedProperty() {}

    const auto& GetName() const { return _name; }
    auto GetPropOffset() const { return _propOffset; }

    virtual void LogOut(void* structAddr) {}
};

class StringProperty : public ReflectedProperty
{
public:
    StringProperty() {}
    virtual ~StringProperty() {}

    std::string* AccessValue(void* structAddr)
    {
        return (std::string*)((uint8_t*)structAddr + _propOffset);
    }

    virtual void LogOut(void* structAddr) override
    {
        SPP_LOG(LOG_REFLECTION, LOG_INFO, "String: %s", AccessValue(structAddr)->c_str());
    }
};

template<typename T>
class TNumericalProperty : public ReflectedProperty
{
public:
    TNumericalProperty() {}
    virtual ~TNumericalProperty() {}

    T* AccessValue(void* structAddr)
    {
        return (T*)((uint8_t*)structAddr + _propOffset);
    }

    virtual void LogOut(void* structAddr) override
    {
        SPP_LOG(LOG_REFLECTION, LOG_INFO, "Number: %s", std::to_string(*AccessValue(structAddr)).c_str());
    }
};

class DynamicArrayProperty : public ReflectedProperty
{

public:
    DynamicArrayProperty() {}
    virtual ~DynamicArrayProperty() {}

    std::unique_ptr<ReflectedProperty> _inner;

    void* AccessValue(void* structAddr)
    {
        return (void*)((uint8_t*)structAddr + _propOffset);
    }

    virtual void LogOut(void* structAddr) override
    {
        SE_ASSERT(_type.GetTypeData()->arrayManipulator);

        auto arrayAddr = AccessValue(structAddr);
        auto totalSize = _type.GetTypeData()->arrayManipulator->Size(arrayAddr);

        SPP_LOG(LOG_REFLECTION, LOG_INFO, "ARRAY: size: %zd", totalSize);
        for (size_t Iter = 0; Iter < totalSize; Iter++)
        {
            SPP_LOG(LOG_REFLECTION, LOG_INFO, "IDX: %zd", Iter);
            _inner->LogOut(_type.GetTypeData()->arrayManipulator->Element(arrayAddr, (int32_t)Iter));
        }
    }
};

class ObjectProperty : public ReflectedProperty
{

public:
    ObjectProperty() {}
    virtual ~ObjectProperty() {}
};

class UniquePtrProperty : public ReflectedProperty
{
public:
    std::unique_ptr<ReflectedProperty> _inner;

    void* AccessValue(void* structAddr)
    {
        return (void*)((uint8_t*)structAddr + _propOffset);
    }

    virtual void LogOut(void* structAddr) override
    {
        SE_ASSERT(_type.GetTypeData()->wrapManipulator);

        auto uniquePtrAddr = AccessValue(structAddr);

        if (_type.GetTypeData()->wrapManipulator->IsValid(uniquePtrAddr))
        {
            SPP_LOG(LOG_REFLECTION, LOG_INFO, "UNIQUE_PTR: ");
            _inner->LogOut(_type.GetTypeData()->wrapManipulator->GetValue(uniquePtrAddr));
        }
    }

    UniquePtrProperty() {}
    virtual ~UniquePtrProperty() {}
};


////////////////////////////////////////////
//
// METHODS
// 
////////////////////////////////////////////

struct Argument
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

class ReflectedMethod
{
    template<typename Class_Type>
    friend struct ClassBuilder;

protected:
    std::string _name;

    CPPType _type;
    CPPType _returnType;
    std::vector< CPPType > _propertyTypes;
    std::function<void(void*, Argument &, const std::vector< Argument >&)> _method;

public:
    const auto& GetName() const { return _name; }
    const auto& GetCaller() const { return _method; }
    const auto& GetArgTypes() const { return _propertyTypes; }
    const auto& GetReturnType() const { return _returnType; }
    
    virtual void LogOut(void* structAddr) {}
};


////////////////////////////////////////////
//
// Structure/Class
// 
////////////////////////////////////////////

class ReflectedStruct
{
public:
    ReflectedStruct* _parent = nullptr;

	std::vector< std::unique_ptr<ReflectedProperty> > _properties;
    std::vector< std::unique_ptr<ReflectedMethod> > _methods;

public:
	ReflectedStruct() {}

    virtual void DumpString(void* structAddr) 
    {
        auto curStruct = this;

        while (curStruct)
        {
            for (const auto& curProp : curStruct->_properties)
            {
                SPP_LOG(LOG_REFLECTION, LOG_INFO, "NAME: %s OFFSET: %zd", curProp->GetName().c_str(), curProp->GetPropOffset());
                curProp->LogOut(structAddr);
            }

            curStruct = curStruct->_parent;
        }
    }

    template<typename Ret, typename ...Args>
    Ret Invoke(void* structAddr, const std::string &MethodName, Args ...args) const
    {
        auto curStruct = this;        

        std::vector< Argument > arguments;
        (arguments.push_back(Argument( get_type < decltype(args) > (), (void*) & args)), ...);

        while (curStruct)
        {
            for (const auto& method : curStruct->_methods)
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
                            if (arguments[Iter].type != argsTypes[Iter])
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
};

// For nested structures
class StructProperty : public ReflectedProperty
{
public:
    ReflectedStruct* _struct = nullptr;

    StructProperty() {}
    virtual ~StructProperty() {}

    void* AccessValue(void* structAddr)
    {
        return (void*)((uint8_t*)structAddr + _propOffset);
    }

    virtual void LogOut(void* structAddr) override
    {
        SPP_LOG(LOG_REFLECTION, LOG_INFO, "STRUCT PROP");
        auto newOffset = AccessValue(structAddr);
        _struct->DumpString(newOffset);
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

template<typename ClassType, typename Func, std::size_t... Is>
inline void invoke(ClassType* BaseObject, Func func_ptr, Argument& retArg, const std::vector< Argument >& arguments, std::index_sequence<Is...>)
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
        *(return_type*)retArg.reference = (BaseObject->*func_ptr)(*arguments[Is].GetValue< typename std::tuple_element_t<Is, arg_tuple> >()...);
    }
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

        classType.GetTypeData()->dataAllocation = std::make_unique< TDataAllocation< Class_Type > >();
    }

    template<typename T, typename ClassSet = Class_Type>
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, T ClassSet::* prop)
    {
        auto calcOffset = offsetOf(prop);
        if constexpr (std::is_arithmetic_v<T>)
        {
            auto newProp = std::make_unique< TNumericalProperty<T> >();
            newProp->_name = InName;
            newProp->_propOffset = calcOffset;
            newProp->_type = get_type<T>();
            return std::move(newProp);
        }
        else if constexpr (std::is_pointer_v<T> &&
            std::is_base_of_v<ObjectBase, std::remove_pointer_t<T> >)
        {
            auto newProp = std::make_unique< ObjectProperty >();
            newProp->_name = InName;
            newProp->_propOffset = calcOffset;
            newProp->_type = get_type<T>();
            return std::move(newProp);
        }
        else if constexpr (std::is_same_v<std::string, T>)
        {
            auto newProp = std::make_unique< StringProperty >();
            newProp->_name = InName;
            newProp->_propOffset = calcOffset;
            newProp->_type = get_type<T>();
            return std::move(newProp);
        }
        else if constexpr (std::is_class_v<T>)
        {
            CPPType structType = get_type<T>();

            auto newProp = std::make_unique< StructProperty >();
            newProp->_name = InName;
            newProp->_propOffset = offsetOf(prop);
            newProp->_type = get_type<T>();
            newProp->_struct = structType.GetTypeData()->structureRef.get();

            return std::move(newProp);
        }
        else
        {
            SE_ASSERT(false);
            //static_assert(false, "Unknown Property Type");
        }

        return nullptr;
    }


    template<typename T, typename ClassSet = Class_Type>
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, std::vector<T> ClassSet::* prop)
    {
        auto arraytype = get_type< std::vector<T> >();

        struct Dummy
        {
            T inner;
        };

        auto newProp = std::make_unique< DynamicArrayProperty >();
        newProp->_name = InName;
        newProp->_propOffset = offsetOf(prop);
        newProp->_type = arraytype;
        newProp->_inner = CreateProperty("inner", &Dummy::inner);
        return std::move(newProp);
    }

    template<typename T, typename ClassSet = Class_Type>
    std::unique_ptr< ReflectedProperty > CreateProperty(const char* InName, std::unique_ptr<T> ClassSet::* prop)
    {
        auto propType = get_type< std::unique_ptr<T> >();

        struct Dummy
        {
            T inner;
        };

        auto newProp = std::make_unique< UniquePtrProperty >();
        newProp->_name = InName;
        newProp->_propOffset = offsetOf(prop);
        newProp->_type = propType;
        newProp->_inner = CreateProperty("inner", &Dummy::inner);
        return std::move(newProp);
    }

    template<typename T>
    ClassBuilder& property(const char* InName, T Class_Type::* prop)
    {
        _class->_properties.push_back(CreateProperty(InName, prop));
        return *this;
    }

    
    template<typename Func>
    ClassBuilder& method(const char* InName, Func Class_Type::* method)
    {
        static_assert(std::is_function_v<Func>);

        using return_type = typename function_traits<Func>::return_type;
        using arg_tuple = typename function_traits<Func>::arg_tuple;
        using integer_sequence = std::make_index_sequence< std::tuple_size_v<arg_tuple> >;

        auto retType = get_type< return_type >();
        auto methodArgs = getmethodargs< Func >(integer_sequence{});

        auto callMethod = [method](void* InClassAddr, Argument &retArg, const std::vector< Argument > &arguments) -> void
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
};

template<typename Class_Type>
ClassBuilder< Class_Type> build_class(std::string_view name)
{   
    return ClassBuilder< Class_Type>();
}
