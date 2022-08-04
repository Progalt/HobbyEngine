
#include "JobSystem.h"

std::condition_variable wakeCondition;   
std::mutex wakeMutex;   
uint64_t currentLabel = 0;  
std::atomic<uint64_t> finishedLabel;
uint32_t numThreads = 0;
Queue<std::function<void()>, 256> jobPool;

uint32_t JobSystem::GetThreadCount()
{
    return numThreads;
}

void JobSystem::Init()
{

	finishedLabel.store(0);

	numThreads = std::max(1u, std::thread::hardware_concurrency());

    for (uint32_t threadID = 0; threadID < numThreads; ++threadID)
    {
        std::thread worker([] {

            std::function<void()> job; 

            while (true)
            {
                if (jobPool.pop_front(job)) 
                {
                    job(); 
                    finishedLabel.fetch_add(1);
                }
                else
                {
                    std::unique_lock<std::mutex> lock(wakeMutex);
                    wakeCondition.wait(lock);
                }
            }

            });

        worker.detach(); 
    }
}

void JobSystem::Execute(std::function<void()> job)
{
    currentLabel += 1;

    while (!jobPool.push_back(job)) { Poll(); }

    wakeCondition.notify_one();
}

void JobSystem::Poll()
{
    wakeCondition.notify_one(); 
    std::this_thread::yield();
}

bool JobSystem::IsBusy()
{
    return finishedLabel.load() < currentLabel;
}

void JobSystem::Wait()
{
    while (IsBusy()) { Poll(); }
}