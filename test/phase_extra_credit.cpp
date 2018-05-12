#include <chloros.h>
#include <atomic>
#include <thread>
#include <vector>

constexpr int const kKernelThreads = 25;

void AsyncTask(void *arg)
{
  int t = *(int*)arg;
  printf("I won't block! %d\n", t);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  printf("I'm done! %d\n", t);
}

void GreenThreadAsyncTest(void *arg) {
  int status = 0;
  int count = 0;
  int thread = *(int*)arg;
  chloros::CreateAsyncTask(AsyncTask, arg, &status);
  
  printf("Async Task Started %d\n", thread);

  while (status != 1) {
    count++;
    if (count%100 == 0) {
      count = 1;
      printf("Not Blocked %d\n", thread);
    }
    chloros::Yield();
  }
  
  printf("Async Task Done! %d\n", thread);
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
