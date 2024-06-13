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

    struct ArrayManipulator
    {
        virtual void* Element(void* ArrayPtr, int32_t Idx) = 0;
        virtual size_t Size(void* ArrayPtr) = 0;
        virtual void Resize(void* ArrayPtr, size_t NewSize) = 0;
    };

    template<typename T>
    struct TArrayManipulator : public ArrayManipulator
    {
        auto& AsType(void* ArrayPtr)
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
}