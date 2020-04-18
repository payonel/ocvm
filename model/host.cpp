#include "host.h"

#include "apis/unicode.h"
#include "drivers/fs_utils.h"

#include "components/component.h"
#include "io/frame.h"

Host::Host(string frameType)
    : _frameType(frameType)
{
}

Host::~Host()
{
  close();
}

/*static*/
bool Host::registerComponentType(const std::string& type, Host::GeneratorCallback generator)
{
  auto& gens = Host::generators();
  if (gens.find(type) != gens.end())
    return false;

  gens[type] = generator;
  return true;
}

/*static*/
std::map<std::string, Host::GeneratorCallback>& Host::generators()
{
  static std::map<std::string, Host::GeneratorCallback> _generators;
  return _generators;
}

std::unique_ptr<Component> Host::create(const string& type) const
{
  const auto& gens = Host::generators();
  const auto& genit = gens.find(type);
  if (genit == gens.end())
  {
    return nullptr;
  }
  return genit->second();
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
