#pragma once

#include <mutex>
#include <thread>
using std::mutex;
using std::thread;
using std::unique_lock;

class NiceWork
{
public:
  void set();
  ~NiceWork();

private:
  bool _work_done = false;
};

class Worker
{
public:
  virtual ~Worker();
  bool isRunning();

  bool start();
  void stop();

protected:
  virtual bool onStart() = 0;
  virtual bool runOnce() = 0;
  virtual void onStop() = 0;

  unique_lock<mutex> make_lock();

private:
  void proc();
  volatile bool _continue = false;
  thread* _pthread = nullptr;
  bool _running = false;
  mutex _m;
};
