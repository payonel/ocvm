#pragma once

#include <memory>
#include <string>
#include <vector>
using std::string;
using std::unique_ptr;
using std::vector;

#include "io/event.h"
#include "worker.h"

#include "connection.h"

class ServerPool;
class Connection;

class ModemDriver : public Worker
{
public:
  ModemDriver(EventSource<ModemEvent>* source, int system_port, const std::string& system_address);
  ~ModemDriver();
  bool send(const vector<char>& payload);

protected:
  bool onStart() override;
  bool runOnce() override;
  void onStop() override;
  bool readNextModemMessage(ModemEvent& mev);

private:
  // ctor assigned
  EventSource<ModemEvent>* _source;
  int _system_port;
  std::string _system_address;

  // default values
  unique_ptr<ServerPool> _local_server;
  unique_ptr<Connection> _connection;
};
