#include "host.h"

#include "apis/unicode.h"
#include "drivers/fs_utils.h"

#include "components/component.h"
#include "components/screen.h"
#include "components/gpu.h"
#include "components/eeprom.h"
#include "components/computer.h"
#include "components/filesystem.h"
#include "components/keyboard.h"
#include "components/internet.h"
#include "components/modem.h"
#include "components/sandbox.h"

#include <dlfcn.h>

typedef std::string get_name_t();

Host::Host(string frameType) :
    _frameType(frameType)
{
    load_all();
}

void Host::load_all()
{
    auto exec_path = fs_utils::make_proc_path("bin/plugins/");
    auto files = fs_utils::list(exec_path);
    const std::string so_ext = ".so";
    for (const auto& file : files)
    {
        if (file.find(so_ext) + so_ext.size() == file.length())
        {
            char* error;
            void* handle = dlopen(file.c_str(), RTLD_LAZY);
            if ((error = dlerror()) != nullptr)
            {
                fprintf(stderr, "dlopen failed: %s\n", error);
                continue;
            }

            get_name_t* get_name = (get_name_t*)dlsym(handle, "name");
            if ((error = dlerror()) != nullptr)
            {
                fprintf(stderr, "bad so, does not declare name: %s\n", error);
                dlclose(handle);
                continue;
            }

            std::string name = get_name();

            create_object_t* creator = (create_object_t*)dlsym(handle, "create_object");
            if ((error = dlerror()) != nullptr)
            {
                fprintf(stderr, "bad so, does not declare create_object: %s\n", error);
                dlclose(handle);
                continue;
            }

            _creators[name] = creator;
        }
    }
}

Host::~Host()
{
    close();
}

std::unique_ptr<Component> Host::create(const string& type) const
{
    std::unique_ptr<Component> result;
    if (type == "screen")
    {
        result.reset(new Screen);
    }
    else if (type == "gpu")
    {
        result.reset(new Gpu);
    }
    else if (type == "eeprom")
    {
        result.reset(new Eeprom);
    }
    else if (type == "computer")
    {
        result.reset(new Computer);
    }
    else if (type == "filesystem")
    {
        result.reset(new Filesystem);
    }
    else if (type == "keyboard")
    {
        result.reset(new Keyboard);
    }
    else if (type == "internet")
    {
        result.reset(new Internet);
    }
    else if (type == "modem")
    {
        result.reset(new Modem);
    }
    else if (type == "sandbox")
    {
        result.reset(new Sandbox);
    }
    else if (_creators.find(type) != _creators.end())
    {
        const auto creator_iterator = _creators.find(type);
        if (creator_iterator != _creators.end())
        {
            result = creator_iterator->second();
        }
    }

    return result;
}

Frame* Host::createFrame() const
{
    return Factory::create_frame(_frameType);
}

void Host::close()
{
}

string Host::stackLog() const
{ 
    return _stack_log;
}

void Host::stackLog(const string& stack_log)
{
    _stack_log = stack_log;
}

string Host::biosPath() const
{
    return _bios_path;
}

void Host::biosPath(const string& bios_path)
{
    _bios_path = bios_path;
}

string Host::fontsPath() const
{
    return _fonts_path;
}

void Host::fontsPath(const string& fonts_path)
{
    _fonts_path = fonts_path;
    if (!UnicodeApi::configure(_fonts_path))
    {
        std::cerr << "Failed lot load fonts: " << fonts_path << std::endl;
        ::exit(1);
    }
}

string Host::machinePath() const
{
    return _machine_path;
}

void Host::machinePath(const string& machine_path)
{
    _machine_path = machine_path;
}
