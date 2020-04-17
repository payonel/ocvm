#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "connection.h"
#include "worker.h"

class FileLock
{
public:
  static std::unique_ptr<FileLock> create(const std::string& path);
  ~FileLock();

private:
  FileLock(const std::string& path, int fd);
  std::string _path;
  int _fd;
};

class ServerPool : public Worker
{
public:
  static std::unique_ptr<ServerPool> create(int system_port);
  virtual ~ServerPool();

protected:
  ServerPool(int id, std::unique_ptr<FileLock> lock);
  bool onStart() override;
  bool runOnce() override;
  void onStop() override;
  bool remove(int id);

private:
  std::map<int, Connection*> _connections;
  int _id;
  std::unique_ptr<FileLock> _lock;
};
