
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>

class WorkProcessor
{
public:

    using Task = std::function<void()>;

    explicit WorkProcessor(size_t num_threads)
        : stop(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back(&WorkProcessor::worker_thread, this);
        }
    }

    ~WorkProcessor()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
        {
            worker.join();
        }
    }

    void enqueue_task(Task task)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
    }

private:
    void worker_thread()
    {
        while (true)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this]()
                               { return stop || !tasks.empty(); });
                if (stop && tasks.empty())
                    return;
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

int main()
{
    WorkProcessor wp(4); // Using 4 threads for parallelism

    while (true)
    {
        wp.enqueue_task([]()
                        {
                            std::cout << "." << std::flush;
                            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate some work
                        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Add new tasks periodically
    }

    return 0;
}
