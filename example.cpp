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
    threadpool.run([](int xx)
    {
        cout << xx << endl;
    }, 33);
    int x = 10, y = 20;
    threadpool.run([x, y]()
    {
        cout << x << "-" << y << endl;
    });
    threadpool.run(std::bind(func, 100));

    char c;
    cin >> c;

    return 0;
}