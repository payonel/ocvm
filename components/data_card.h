#pragma once
#include "component.h"
#include "model/value.h"
#include <tuple>
#include <vector>
using std::tuple;
using std::vector;

class DataCard : public Component
{
  public:
    DataCard();
    ~DataCard();

    enum ConfigIndex {
        Tier = Component::ConfigIndex::Next,
    };

    // Tier 1
    int crc32(lua_State* lua);
    int md5(lua_State* lua);

  protected:
    bool onInitialize() override;

    int _tier = 1;

  private:
    static bool s_registered;
};