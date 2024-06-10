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
    float X;
    std::vector< int32_t > timeStamps;
    std::string GuyName;

    float DoJump(float howHigh, std::string TestOut)
    {
        X += howHigh;
        //TestOut = "hello";
        return X;
    }

    virtual CPPType GetCPPType() const override { return get_type < GuyTest >(); }
};



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
    
SPP_AUTOREG_END


struct SuperGuy : public GuyTest
{
    PARENT_CLASS(GuyTest)

    int32_t health;
    SceneParent* parent = nullptr;
    PlayerData data;
    std::unique_ptr< std::string > HitMe;
    std::vector< std::unique_ptr< PlayerFighters > >  Players;

    virtual CPPType GetCPPType() const override { return get_type < SuperGuy >(); }
};

//test split auto reg
SPP_AUTOREG_START

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

    // lets create the top level class and populate some data
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
        // cleanse it of any type
        void* ptrToGuyNoTypeData = &guy;
            
        auto foundType = ((ObjectBase*)ptrToGuyNoTypeData)->GetCPPType();
        auto classData = foundType.GetTypeData()->structureRef.get();
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