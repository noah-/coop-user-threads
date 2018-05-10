#include <chloros.h>
#include <atomic>
#include <thread>
#include <vector>

constexpr int const kKernelThreads = 25;

void AsyncTask(void *arg)
{
  printf("I won't block! %d\n", *(int*)arg);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  printf("I'm done! %d\n", *(int*)arg);
}

void GreenThreadAsyncTest(void *arg) {
  int status = 0;
  int count = 0;
  chloros::Spawn(AsyncTask, arg, &status);
  
  printf("Async Task Started %d\n", *(int*)arg);

  while (status != 1) {
    count++;
    if (count%100 == 0) {
      count = 1;
      printf("Not Blocked %d\n", *(int*)arg);
    }
    chloros::Yield();
  }
  
  printf("Async Task Done! %d\n", *(int*)arg);
}

void GreenThreadWorker(void*) { printf("Green thread on kernel thread.\n"); }

void KernelThreadWorker(int n) {
  chloros::Initialize();
  if (n%5 == 1)
    chloros::Spawn(GreenThreadAsyncTest, &n);
  else
    chloros::Spawn(GreenThreadWorker, nullptr);
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
  return 0;
}
