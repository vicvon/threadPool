#include <iostream>
#include "threadPool.h"

using namespace std;

typedef std::function<void(int)> T;

void func(int i)
{
    cout << i << endl;
}

void func1(int i, int j)
{
    cout << i << "," << j << endl;
}

int main()
{
    ThreadPool<void> threadpool;
    threadpool.setMaxTask(10);
    threadpool.start(5);
    threadpool.run(func, 1);
    threadpool.run(func1, 2, 3);
    threadpool.run(func, 3);
    threadpool.run(func1, 4, 5);
    threadpool.run(func, 5);
    return 0;
}