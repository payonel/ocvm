#pragma once

#include <string>
#include <memory>

class Component;

class DynamicDriverFactory
{
public:
  DynamicDriverFactory();
  std::unique_ptr<Component> create(const std::string& type); // returns null if type isn't found
private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

