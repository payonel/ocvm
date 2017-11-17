#pragma once

#include "component.h"
#include "model/prof_log.h"
#include <queue>
using std::queue;

class Computer : public Component
{
public:
    Computer();
    ~Computer();

    enum ConfigIndex
    {
        TotalMemory = Component::ConfigIndex::Next
    };

    RunState update() override;
    bool newlib(LuaProxy* proxy);
    void close();
    void setTmpAddress(const string& addr);
    void pushSignal(const ValuePack& pack);
    bool postInit() override;

    void* alloc(void* ptr, size_t osize, size_t nsize);
    void stackLog(const string& stack_log);

    int isRunning(lua_State* lua);
    int setArchitecture(lua_State* lua);
    int getArchitecture(lua_State* lua);
    int getArchitectures(lua_State* lua);
    int beep(lua_State* lua);
    int getDeviceInfo(lua_State* lua);
    int getProgramLocations(lua_State* lua);
    int pushSignal(lua_State* lua);
    int removeUser(lua_State* lua);
    int addUser(lua_State* lua);
    int isRobot(lua_State* lua);
    int tmpAddress(lua_State* lua);
    int freeMemory(lua_State* lua);
    int totalMemory(lua_State* lua);
    int energy(lua_State* lua);
    int maxEnergy(lua_State* lua);
    int realTime(lua_State* lua);
    int uptime(lua_State* lua);

    // non-spec methods (for vm debugging)
    int crash(lua_State* lua);
    int print(lua_State* lua);
protected:
    bool onInitialize() override;
    RunState resume(int nargs);

    int get_address(lua_State* lua);
    void mark_gc();
    size_t memoryUsedRaw();
    size_t memoryUsedVM();
    size_t freeMemory();
private:
    void injectCustomLua();

    double _start_time;
    string _tmp_address;
    lua_State* _state = nullptr;
    lua_State* _machine = nullptr;
    double _standby = 0;
    double _nexttrace = 0;

    size_t _peek_memory = 0; // for debugging purposes
    size_t _total_memory = 0;
    size_t _baseline = 0;
    bool _baseline_initialized = false;

    queue<ValuePack> _signals;

    size_t _gc_ticks = 0;
    ProfLog _prof;
};
