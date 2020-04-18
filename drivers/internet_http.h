#pragma once

#include "internet_drv.h"

class PipedCommand
{
public:
  PipedCommand();
  virtual ~PipedCommand();
  bool open(const string& command, const vector<string>& args);
  Connection* stdin() const;
  Connection* stdout() const;
  Connection* stderr() const;
  int id() const;
  void close();

private:
  unique_ptr<Connection> _stdin;
  unique_ptr<Connection> _stdout;
  unique_ptr<Connection> _stderr;
  pid_t _child_id;
};

class HttpObject : public InternetConnection
{
public:
  HttpObject(const HttpAddress& addr, const string& post, const map<string, string>& header);
  int read(lua_State* lua);
  int response(lua_State* lua);

protected:
  bool update() override;
  Connection* connection() const override;

private:
  bool _response_ready;
  ValuePack _response;
  PipedCommand _cmd;

  static bool s_registered;
};
