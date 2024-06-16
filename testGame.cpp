// SPPReflection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>

#include "SPPReflection.h"

using namespace SPP;

struct SceneParent : public ObjectBase
{
    int32_t matrix;

    virtual CPPType GetCPPType() const override { return get_type < SceneParent >(); }
};

struct PlayerData
{
    int GUID = -1;
    std::string TAG = "tagnotset";
};

struct PlayerFighters
{
    std::string name;
    float health;
};

struct GuyTest : public ObjectBase
{
    ENABLE_REFLECTION;

    float X;
    std::vector< int32_t > timeStamps;
    std::string GuyName;

    float DoJump(float howHigh, std::string &TestOut)
    {
        X += howHigh;
        TestOut = "hello";
        return X;
    }

};

SPP_AUTOREG_START

    REFL_CLASS_START(PlayerFighters)

        RC_ADD_PROP(name)
        RC_ADD_PROP(health)

    REFL_CLASS_END

    REFL_CLASS_START(PlayerData)

        RC_ADD_PROP(GUID)
        RC_ADD_PROP(TAG)

    REFL_CLASS_END


    REFL_CLASS_START(GuyTest)

        RC_ADD_PROP(X)
        RC_ADD_PROP(timeStamps)
        RC_ADD_PROP(GuyName)

        RC_ADD_METHOD(DoJump)

    REFL_CLASS_END

SPP_AUTOREG_END

enum class EGuyType
{
    BadGuy,
    GoodGuy,
    Unknown
};

struct SuperGuy : public GuyTest
{
    ENABLE_REFLECTION_C(GuyTest);


    int32_t health = 0;
    SceneParent* parent = nullptr;
    PlayerData data;

    EGuyType ourGuy = EGuyType::Unknown;

protected:   

    std::unique_ptr< std::string > HitMe;
    std::vector< std::unique_ptr< PlayerFighters > >  Players;

public:

    SuperGuy() {}
    SuperGuy(const std::string &InName, int32_t InHealth)
    {
        GuyName = InName;
        health = InHealth;
    }

    auto& GetHitMe() { return HitMe; }
    auto& GetPlayers() { return Players; }

};




//test split auto reg
SPP_AUTOREG_START

    REFL_ENUM_START(EGuyType)
        RC_ENUM_VALUE(EGuyType::BadGuy, "BadGuy")
        RC_ENUM_VALUE(EGuyType::GoodGuy, "GoodGuy")
        RC_ENUM_VALUE(EGuyType::Unknown, "Unknown")
    REFL_ENUM_END

    REFL_CLASS_START(SuperGuy)

        RC_ADD_PROP(health)
        RC_ADD_PROP(parent)
        RC_ADD_PROP(data)
        RC_ADD_PROP(HitMe)
        RC_ADD_PROP(Players)
        RC_ADD_PROP(ourGuy)

        RC_ADD_CONSTRUCTOR(const std::string &, int32_t)

    REFL_CLASS_END
                

SPP_AUTOREG_END


int main()
{
    std::cout << "Hello World!\n";

    // lets create the top level class and populate some data
    SuperGuy guy;
    guy.timeStamps.push_back(4);
    guy.timeStamps.push_back(5);
    guy.data.GUID = 123456;
    guy.data.TAG = "DATATAG";
    guy.health = 123;
    guy.GuyName = "yoyoyo";
    guy.X = 321.1f;
    guy.ourGuy = EGuyType::GoodGuy;

    guy.GetPlayers().push_back(std::unique_ptr<PlayerFighters>(
        new PlayerFighters{
            "JOJO",
            12.23f
        }));
    guy.GetPlayers().push_back(std::unique_ptr<PlayerFighters>(
        new PlayerFighters{
            "James",
            0.123f
        }));
    guy.GetHitMe() = std::make_unique< std::string >("AHHHHHHH 123");


    {        
        // cleanse it of any type
        void* ptrToGuyNoTypeData = &guy;
            
        auto foundType = ((ObjectBase*)ptrToGuyNoTypeData)->GetCPPType();
        auto classData = foundType.GetTypeData()->structureRef.get();

        //int32_t valueSet = 1337;
        auto newClass = classData->Invoke_Constructor<SuperGuy*>(
            // Args
            std::string("Te23232st"),
            132);

        classData->LogOut(ptrToGuyNoTypeData);

        float jumpOut = 0.0f;

        SPP_LOG(LOG_REFLECTION, LOG_INFO, "Call invoke: previous jumpOut %f", jumpOut);

        std::string stringREf("Test");

        jumpOut =
            classData->Invoke<float>(
                // class ptr
                ptrToGuyNoTypeData, 
                // function name
                std::string("DoJump"), 
                // Args
                332211.0f, stringREf);

        SPP_LOG(LOG_REFLECTION, LOG_INFO, " - post invoke: jumpOut %f", jumpOut);
    }
}