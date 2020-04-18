#pragma once
#include <sstream>
#include <string>

using std::endl;
using std::string;
using std::stringstream;

struct LoggerContext
{
  std::string path;
};

class Logger
{
public:
  static void context(LoggerContext ctx);
  static LoggerContext context();
  static Logger& getSingleLogger();

  Logger& operator<<(const string& text);
  Logger& operator<<(std::ostream& (*)(std::ostream&));
  Logger& operator<<(const char* cstr);
  template <typename T>
  Logger& operator<<(const T& t)
  {
    stringstream ss;
    ss << t;
    return *this << ss.str();
  }

  string serialize(stringstream& ss)
  {
    return ss.str() + "\n";
  }

  template <typename T, typename... Ts>
  string serialize(stringstream& ss, const T& arg, const Ts&... args)
  {
    ss << arg;
    return serialize(ss, args...);
  }

  template <typename... Ts>
  Logger& write(const Ts&... args)
  {
    stringstream ss;
    return *this << serialize(ss, args...);
  }
  static Logger& lout;

private:
  Logger()
  {
  }                         // cannot create mulitple loggers
  Logger(Logger&) = delete; // no copies

  static LoggerContext s_context;
};

namespace Logging
{
extern Logger& lout;
};
