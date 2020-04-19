#include "worker.h"

#include <thread>

void NiceWork::set()
{
  _work_done = true;
}

NiceWork::~NiceWork()
{
  if (!_work_done)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 20));
}

bool Worker::isRunning()
{
  return _pthread && _running;
}

bool Worker::start()
{
  auto lock = make_lock();
  if (isRunning())
    return false;

  _running = true;
  _continue = true;
  _pthread = new thread(&Worker::proc, this);

  return true;
}

void Worker::stop()
{
  {
    auto lock = make_lock();
    _continue = false;
  }

  if (_pthread)
    _pthread->join();

  auto lock = make_lock();
  _running = false;
  delete _pthread;
  _pthread = nullptr;
}

void Worker::proc()
{
  {
    auto lock = make_lock();
    _continue = _continue && onStart();
  }
  while (_continue)
  {
    {
      auto lock = make_lock();
      if (!runOnce())
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  auto lock = make_lock();
  onStop();
  _running = false;
}

unique_lock<mutex> Worker::make_lock()
{
  return unique_lock<mutex>(_m);
}
