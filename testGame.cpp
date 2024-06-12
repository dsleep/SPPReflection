// SPPReflection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>

#include "SPPReflection.h"

struct SceneParent : public ObjectBase
{
    int32_t matrix;

    virtual CPPType GetCPPType() const override { return get_type < SceneParent >(); }
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

struct GuyTest : public ObjectBase
{
    ENABLE_REFLECTION;

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



struct SuperGuy : public GuyTest
{
    ENABLE_REFLECTION_C(GuyTest);


    int32_t health;
    SceneParent* parent = nullptr;
    PlayerData data;

protected:

    std::unique_ptr< std::string > HitMe;
    std::vector< std::unique_ptr< PlayerFighters > >  Players;

};

//test split auto reg
SPP_AUTOREG_START

    REFL_CLASS_START(SuperGuy)

        RC_ADD_PROP(health)
        RC_ADD_PROP(parent)
        RC_ADD_PROP(data)
        RC_ADD_PROP(HitMe)
        RC_ADD_PROP(Players)

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

#if 0
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
#endif

    {        
        // cleanse it of any type
        void* ptrToGuyNoTypeData = &guy;
            
        auto foundType = ((ObjectBase*)ptrToGuyNoTypeData)->GetCPPType();
        auto classData = foundType.GetTypeData()->structureRef.get();

        auto madeNew = (ObjectBase*)foundType.GetTypeData()->dataAllocation->Construct();

        classData->DumpString(ptrToGuyNoTypeData);

        float jumpOut = 0.0f;

        SPP_LOG(LOG_REFLECTION, LOG_INFO, "Call invoke: previous jumpOut %f", jumpOut);

        jumpOut =
            classData->Invoke<float>(
                // class ptr
                ptrToGuyNoTypeData, 
                // function name
                std::string("DoJump"), 
                // Args
                332211.0f, std::string("Test"));

        SPP_LOG(LOG_REFLECTION, LOG_INFO, " - post invoke: jumpOut %f", jumpOut);
    }
}