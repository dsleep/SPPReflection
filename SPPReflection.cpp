// SPPReflection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>

#define SPP_CAT_IMPL(a, b) a##b
#define SPP_CAT(a, b) SPP_CAT_IMPL(a, b)

#define SPP_AUTOREG_START                                                           \
namespace SPP_CAT(spp_auto_reg_namespace_, __LINE__)								\
{																					\
    struct RegStruct																\
    {                                                                               \
        RegStruct() {

#define SPP_AUTOREG_END		\
		} \
	};						\
	const static RegStruct _reg;			\
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



#define RTTR_REGISTRATION_FUNC_EXTRACT_VARIABLES(begin_skip, end_skip)      \
    static constexpr std::size_t skip_size_at_begin = begin_skip;      \
    static constexpr std::size_t skip_size_at_end   = end_skip;        

/////////////////////////////////////////////////////////////////////////////////


RTTR_REGISTRATION_FUNC_EXTRACT_VARIABLES(22, 16)  // with " noexcept"

const char* extract_type_signature(const char* signature) noexcept
{
    //    static_assert(N > skip_size_at_begin + skip_size_at_end, "RTTR is misconfigured for your compiler.")
    return &signature[skip_size_at_begin];
}

/////////////////////////////////////////////////////////////////////////////////

template<typename T>
const char* f() noexcept
{
    return extract_type_signature(
#if RTTR_COMPILER == RTTR_COMPILER_MSVC
        __FUNCSIG__
#elif RTTR_COMPILER == RTTR_COMPILER_GNUC
        __PRETTY_FUNCTION__
#elif RTTR_COMPILER == RTTR_COMPILER_CLANG || RTTR_COMPILER == RTTR_COMPILER_APPLECLANG
        __PRETTY_FUNCTION__
#else
    #error "Don't know how the extract type signatur for this compiler! Abort! Abort!"
#endif
    );
}

struct ArrayManipulator
{
    virtual void* Element(void* ArrayPtr, int32_t Idx) = 0;
    virtual size_t Size(void *ArrayPtr) = 0;
    virtual void Resize(void *ArrayPtr, size_t NewSize) = 0;
};

template<typename T>
struct TArrayManipulator : public ArrayManipulator
{
    auto &AsType(void* ArrayPtr)
    {
        return *(T*)ArrayPtr;
    }
    virtual void* Element(void* ArrayPtr, int32_t Idx) override
    {
        return &AsType(ArrayPtr)[Idx];
    }
    virtual size_t Size(void* ArrayPtr) override
    {
        return AsType(ArrayPtr).size();
    }
    virtual void Resize(void* ArrayPtr, size_t NewSize) override
    {
        AsType(ArrayPtr).resize(NewSize);
    }
};


struct WrapManipulator
{
    virtual bool IsValid(void* ValuePtr) = 0;
    virtual void* GetValue(void* ValuePtr) = 0;
    virtual void Clear(void* ValuePtr) = 0;
};

template<typename T>
struct TWrapManipulator : public WrapManipulator
{
    auto& AsType(void* ValuePtr)
    {
        return *(T*)ValuePtr;
    }
    virtual bool IsValid(void* ValuePtr) override
    {
        if (AsType(ValuePtr))
        {
            return true;
        }

        return false;
    }
    virtual void* GetValue(void* ValuePtr) override
    {
        return AsType(ValuePtr).get();
    }
    virtual void Clear(void* ValuePtr) override
    {
        AsType(ValuePtr).reset();
    }
};

struct type_data
{
    //type_data* raw_type_data;
    //type_data* wrapped_type;
    //type_data* array_raw_type;

    std::string name;
    //std::string_view type_name;

    std::size_t get_sizeof;
    std::size_t get_pointer_dimension;


    std::unique_ptr< ArrayManipulator > arraymanipulator;
    std::unique_ptr< WrapManipulator > wrapmanipulator;

    std::unique_ptr< class ReflectedStruct > structureRef;


    //impl::create_variant_func create_variant;
    //impl::get_base_types_func get_base_types; // FIXME: this info should not be stored, its just temporarily,
    //// thats why we store it as function pointer

    //enumeration_wrapper_base* enum_wrapper;
    //impl::get_metadata_func    get_metadata;
    //impl::create_wrapper_func  create_wrapper;
    //impl::visit_type_func      visit_type;

    //bool is_valid;
    //RTTR_FORCE_INLINE bool type_trait_value(type_trait_infos type_trait) const RTTR_NOEXCEPT { return m_type_traits.test(static_cast<std::size_t>(type_trait)); }


    //type_traits m_type_traits;
    //class_data  m_class_data;

};


class CPPType
{
public:
    typedef uintptr_t type_id;

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

CPPType create_type(type_data* data) noexcept
{
    return data ? CPPType(data) : CPPType();
}



/////////////////////////////////////////////////////////////////////////////////

std::size_t get_size(const char* s) noexcept
{
    return (std::char_traits<char>::length(s) - skip_size_at_end);
}

/////////////////////////////////////////////////////////////////////////////////

template<typename T>
std::string get_type_name() noexcept
{
    return std::string(f<T>(), get_size(f<T>()));
}


template<typename...T>
struct type_list
{
    static constexpr auto size = sizeof...(T); //!< The amount of template parameters
};

#define TYPE_LIST(...) type_list<__VA_ARGS__>
#define PARENT_CLASS(parentC) using parent_class = parentC;

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



template<typename T, typename Enable = void>
struct get_size_of
{
    static constexpr std::size_t value()
    {
        return sizeof(T);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////

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


/////////////////
/////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////

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


template<typename T>
std::unique_ptr<type_data> make_type_data()
{
    auto obj = std::unique_ptr<type_data>
        (
            new type_data
            {
                //raw_type_info<T>::get_type().m_type_data, 
                //wrapper_type_info<T>::get_type().m_type_data,
                //array_raw_type<T>::get_type().m_type_data,

                get_type_name<T>(), // get_type_name<T>(),

                get_size_of<T>::value(),
                pointer_count<T>::value
            }
        );

    if constexpr (IsSTLVector<T>)
    {
        //value_type
        obj->arraymanipulator = std::make_unique< TArrayManipulator< T > >();
    }

    if constexpr (IsUniquePtr<T>)
    {
        //element_type
        obj->wrapmanipulator = std::make_unique< TWrapManipulator< T > >();
    }

    return obj;
}



class TypeCollection
{
private:
    std::vector< std::unique_ptr<type_data> > type_store;

public:
    type_data *Push(std::unique_ptr<type_data> &&InData)
    {
        auto outTypeData = InData.get();
        type_store.push_back(std::move(InData));
        return outTypeData;
    }
};

TypeCollection& GetTypeCollection()
{
    static TypeCollection sO;
    return sO;
}

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



//template<typename T>
//type get() noexcept
//{
//    using non_ref_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
//    
//    return detail::create_or_get_type<non_ref_type>();
//}


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

    virtual std::string ToString(void* structAddr) 
    {
        return "";
    }
};

class ReflectedArguments
{
public:
    void* _baseAddress = nullptr;

public:
    ReflectedArguments() {}
    virtual ~ReflectedArguments() {}
};

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
    
    virtual std::string ToString(void* structAddr) 
    {
        return "";
    }
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

    virtual std::string ToString(void* structAddr) override
    {
        return *AccessValue(structAddr);
    }
};

template<typename T>
class TNumericalProperty : public ReflectedProperty
{
public:
    TNumericalProperty() {}
    virtual ~TNumericalProperty() {}

    T *AccessValue(void* structAddr)
    {
        return (T*)((uint8_t*)structAddr + _propOffset);
    }

    virtual std::string ToString(void* structAddr) override
    {
        return std::to_string( *AccessValue(structAddr) );
    }
};

//using FloatProperty = TNumericalProperty<float>;
//using DoubleProperty = TNumericalProperty<double>;
//
//using Int8Property = TNumericalProperty<int8_t>;
//using UInt8Property = TNumericalProperty<uint8_t>;
//
//using Int16Property = TNumericalProperty<int16_t>;
//using UInt16Property = TNumericalProperty<uint16_t>;
//
//using Int32Property = TNumericalProperty<int32_t>;
//using UInt32Property = TNumericalProperty<uint32_t>;
//
//using Int64Property = TNumericalProperty<int64_t>;
//using UInt64Property = TNumericalProperty<uint64_t>;

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

    virtual std::string ToString(void* structAddr) override
    {
        SE_ASSERT(_type.GetTypeData()->arraymanipulator);

        auto arrayAddr = AccessValue(structAddr);
        auto totalSize = _type.GetTypeData()->arraymanipulator->Size(arrayAddr);

        std::cout << "ARRAY: " << " size: " << totalSize << std::endl;
        for (size_t Iter = 0; Iter < totalSize; Iter++)
        {
            std::cout << "IDX: " << Iter << std::endl;
            std::cout << _inner->ToString(_type.GetTypeData()->arraymanipulator->Element(arrayAddr, (int32_t) Iter)) << std::endl;
        }
        return "";
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

    virtual std::string ToString(void* structAddr) override
    {
        SE_ASSERT(_type.GetTypeData()->wrapmanipulator);

        auto uniquePtrAddr = AccessValue(structAddr);

        if (_type.GetTypeData()->wrapmanipulator->IsValid(uniquePtrAddr))
        {
            std::cout << "UNIQUE_PTR: " << _inner->ToString(_type.GetTypeData()->wrapmanipulator->GetValue(uniquePtrAddr)) << std::endl;
        }

        return "";
    }

    UniquePtrProperty() {}
    virtual ~UniquePtrProperty() {}
};


class ReflectedStruct
{
public:
    ReflectedStruct* _parent = nullptr;

	std::string _name;
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
                std::cout << curProp->GetName() << " " << curProp->GetPropOffset() << std::endl;
                std::cout << curProp->ToString(structAddr) << std::endl;
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

    virtual std::string ToString(void* structAddr) override
    {
        auto newOffset = AccessValue(structAddr);
        _struct->DumpString(newOffset);
        return "";
    }
};

//class PropertyIterator {
//public:
//
//    using vector_iterator = std::vector< std::unique_ptr<ReflectedProperty> >::const_iterator;
//    
//    using value_type = std::unique_ptr<ReflectedProperty>;
//    using difference_type = std::ptrdiff_t;
//    using pointer = value_type*;
//    using reference = value_type&;
//    using iterator_category = std::random_access_iterator_tag;
//
//    int32_t classIdx = 0;
//    vector_iterator curIter;
//    vector_iterator endClassIter;
//    std::vector< ReflectedStruct* > classstack;
//
//    PropertyIterator() {}
//    PropertyIterator(ReflectedStruct *InStruct) 
//    {
//        auto curStruct = InStruct;
//        while (curStruct)
//        {
//            classstack.push_back(curStruct);
//            curStruct = curStruct->_parent;
//        }
//
//        curIter = InStruct->_properties.begin();    
//    }
//
//    void SetIteratorsFromClassIdx()
//    {
//        curIter = classstack[classIdx]->_properties.begin();
//        endClassIter = classstack[classIdx]->_properties.end();
//    }
//
//    auto operator*() const 
//    { 
//        return curIter->get();
//    }
//
//    auto operator->() const
//    {
//        return curIter->get();
//    }
//        
//    PropertyIterator& operator++()
//    {
//        curIter++;
//        if (curIter == endClassIter)
//        {
//            classIdx++;
//
//
//        }
//
//        return *this;
//    }
//    PropertyIterator& operator++(int junk) { (*this)++; return *this; }
//
//    friend bool operator==(const PropertyIterator& lhs, const PropertyIterator& rhs) {
//        return lhs.m_ptr == rhs.m_ptr;
//    }
//    friend bool operator!=(const PropertyIterator& lhs, const PropertyIterator& rhs) {
//        return !(lhs == rhs);
//    }
//
//private:
//    pointer m_ptr;
//};

struct ObjectBase
{
    std::string _baseName;
};

struct SceneParent : public ObjectBase
{
    int32_t matrix;
};

struct PlayerData
{
    int GUID;
    std::string TAG;
};

struct PlayerFighters
{
    std::string name;
    float health;
};

struct GuyTest
{
    float X;
    std::vector< int32_t > timeStamps;
    std::string GuyName;

    float DoJump(float howHigh, std::string TestOut)
    {
        X += howHigh;
        //TestOut = "hello";
        return X;
    }
};

struct SuperGuy : public GuyTest
{
    PARENT_CLASS(GuyTest)

    int32_t health;
    SceneParent* parent = nullptr;
    PlayerData data;
    std::unique_ptr< std::string > HitMe;
    std::vector< std::unique_ptr< PlayerFighters > >  Players;
};


template<typename T, typename U> 
constexpr size_t offsetOf(U T::* member)
{
    return (char*)&((T*)nullptr->*member) - (char*)nullptr;
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
            static_assert(false, "Unknown Property Type");
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

    template<typename Func, std::size_t... Is>
    std::vector< CPPType > getmethodargs(std::index_sequence<Is...>)
    {
        using arg_tuple = typename function_traits<Func>::arg_tuple;
        std::vector< CPPType > types;
        (types.push_back( get_type< std::tuple_element_t<Is, arg_tuple> >() ) , ...);
        return types;
    }

    template<typename ClassType, typename Func, std::size_t... Is>
    inline
    static typename void invoke(ClassType* BaseObject, Func func_ptr, Argument &retArg, const std::vector< Argument >& arguments, std::index_sequence<Is...>)
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
                invoke(classVal, method, retArg, arguments, integer_sequence{});// 1.1f, std::string("tt"));
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

SPP_AUTOREG_START

    build_class<PlayerFighters>("PlayerFighters")
        .property("name", &PlayerFighters::name)
        .property("health", &PlayerFighters::health);

    build_class<PlayerData>("PlayerData")
        .property("GUID", &PlayerData::GUID)
        .property("TAG", &PlayerData::TAG);

    build_class<GuyTest>("GuyTest")
        .property("X", &GuyTest::X)
        .property("timeStamps", &GuyTest::timeStamps)
        .property("GuyName", &GuyTest::GuyName)

        .method("DoJump", &GuyTest::DoJump);

    build_class<SuperGuy>("SuperGuy")
        .property("health", &SuperGuy::health)
        .property("parent", &SuperGuy::parent)
        .property("data", &SuperGuy::data)
        .property("HitMe", &SuperGuy::HitMe)
        .property("Players", &SuperGuy::Players);
        
SPP_AUTOREG_END



int main()
{
    std::cout << "Hello World!\n";

    SuperGuy guy;

    guy.timeStamps.push_back(4);
    guy.timeStamps.push_back(5);

    guy.data.GUID = 123456;
    guy.data.TAG = "DATATAG";

    guy.health = 123;
    guy.GuyName = "yoyoyo";
    guy.X = 321.1f;

    guy.Players.push_back(std::unique_ptr<PlayerFighters>(
        new PlayerFighters{
            "JOJO",
            12.23f
        }));

    guy.Players.push_back(std::unique_ptr<PlayerFighters>(
        new PlayerFighters{
            "James",
            0.123f
        }));

    guy.HitMe = std::make_unique< std::string >("AHHHHHHH 123");

    {
        CPPType getInfo = get_type< SuperGuy >();
        void* ptrToGuyNoTypeData = &guy;

        auto classData = getInfo.GetTypeData()->structureRef.get();

        classData->DumpString(ptrToGuyNoTypeData);

        auto jumpOut = 
            classData->Invoke<float>(ptrToGuyNoTypeData, std::string("DoJump"), 332211.0f, std::string("Test") );
    }
}