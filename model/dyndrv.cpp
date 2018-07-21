#include "dyndrv.h"
#include "components/component.h"
#include "drivers/fs_utils.h"
#include "log.h"

#include <map>
#include <functional>

#include <dlfcn.h>

typedef std::string get_name_t();
typedef std::unique_ptr<Component> create_object_t();

class DynamicDriverFactory::Impl
{
public:
  std::unique_ptr<Component> create(const std::string& type) const
  {
    const auto it = _creators.find(type);
    if (it == _creators.end())
      return nullptr;
    return it->second();
  }

  void load_all()
  {
    auto exec_path = fs_utils::make_proc_path("bin/components/");
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

private:
  std::map<string, create_object_t*> _creators;
};

DynamicDriverFactory::DynamicDriverFactory() :
  _impl(new DynamicDriverFactory::Impl)
{
  _impl->load_all();
}

std::unique_ptr<Component> DynamicDriverFactory::create(const std::string& type)
{
  return _impl->create(type);
}
