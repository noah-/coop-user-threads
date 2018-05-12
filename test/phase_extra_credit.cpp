#include <chloros.h>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <pthread.h>
#include "common.h"

struct Race {
  uint32_t count;

  Race() { 
    count = 0xdeadbeef;
  }
};

static Race *race = NULL;

void Create(void*) {
  race = new Race();
}

void Ready(void*) {
  if (race) {
     ASSERT(race->count == 0xdeadbeef);
  }
}

constexpr int const kKernelThreads = 2;

void KernelThreadWorker(int n) {
  chloros::Initialize();
  if (n%2 == 0)
    chloros::Spawn(Create, nullptr);
  else
    chloros::Spawn(Ready, nullptr);
  chloros::Wait();
  printf("Finished thread %d.\n", n);
}

int main() {
  std::vector<std::thread> threads{};
  for (int i = 0; i < kKernelThreads; ++i) {
    threads.emplace_back(KernelThreadWorker, i);
  }
  for (int i = 0; i < kKernelThreads; ++i) {
    threads[i].join();
  }

  if (race)
    delete race;

  return 0;
} 
