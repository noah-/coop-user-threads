#include <chloros.h>
#include <thread>
#include <vector>
#include <mutex>
#include "common.h"

struct Race {
  uint32_t count;

  Race() { 
    count = 0xdeadbeef;
  }
};

static Race *race = NULL;
std::mutex race_lock{};

void Create(void*) {
  race = new Race();
  race_lock.unlock();
}

void Ready(void*) {
  if (race) {
     ASSERT(race->count == 0xdeadbeef);
     return;
  }

  std::lock_guard<std::mutex> lock{race_lock};
  ASSERT(race->count == 0xdeadbeef);
}

void KernelThreadWorker(int n) {
  chloros::Initialize();
  if (n == 0) {
    race_lock.lock();
    chloros::Spawn(Create, nullptr);
  } else
    chloros::Spawn(Ready, nullptr);
  chloros::Wait();
  printf("Finished thread %d.\n", n);
}

int main() {
  std::vector<std::thread> threads{};
  threads.emplace_back(KernelThreadWorker, 0);
  threads.emplace_back(KernelThreadWorker, 1);
  threads[0].join();
  threads[1].join();

  if (race)
    delete race;

  return 0;
} 
