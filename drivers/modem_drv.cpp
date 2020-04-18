#include "modem_drv.h"
#include "model/log.h"

// c includes for sockets
#include <netdb.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>

// explicitly needed for include on Haiku OS
#include <fcntl.h>
#include <sys/stat.h>

#include "server_pool.h"

bool ModemDriver::readNextModemMessage(ModemEvent& mev)
{
  return _connection->readyNextPacket(&mev.payload, false);
}

ModemDriver::ModemDriver(EventSource<ModemEvent>* source, int system_port, const std::string& system_address)
    : _source(source)
    , _system_port(system_port)
    , _system_address(system_address)
{
}

ModemDriver::~ModemDriver()
{
}

bool ModemDriver::send(const vector<char>& payload)
{
  // runOnce can reset the connection, lock it
  auto lock = make_lock();
  if (!_connection || !_connection->can_write())
  {
    // modem::send failed: not connected
    return false;
  }

  int32_t data_size = static_cast<int32_t>(payload.size());
  char* p = reinterpret_cast<char*>(&data_size);
  vector<char> header{ p[0], p[1], p[2], p[3] };
  if (!_connection->write(header))
    return false;
  return _connection->write(payload);
}

bool ModemDriver::onStart()
{
  return true;
}

bool ModemDriver::runOnce()
{
  NiceWork work;
  if (!_connection)
  {
    _connection.reset(new Connection(_system_address, _system_port));
  }
  switch (_connection->state())
  {
  case ConnectionState::Starting:
    // still waiting for connection to complete
    return true;
  case ConnectionState::Failed:
    // no server
    _connection.reset(nullptr);
    if (!_local_server)
    {
      _local_server = ServerPool::create(_system_port);
    }
    return true;
  case ConnectionState::Ready:
    break;
  case ConnectionState::Finished:
  case ConnectionState::Closed:
    // the server may have closed this connection
    _connection.reset(nullptr);
    return true;
  }

  // current read next message is clearing the payload, so we're safe to reuse the ModemEvent here
  // but, I would like the code to be more obviously correct
  while (true)
  {
    ModemEvent me;
    if (!readNextModemMessage(me))
      break;
    work.set();
    _source->push(me);
  }

  return true;
}

void ModemDriver::onStop()
{
  _connection.reset(nullptr);
  if (_local_server)
  {
    _local_server->stop();
  }
  _local_server.reset(nullptr);
  // modem shutdown
}
