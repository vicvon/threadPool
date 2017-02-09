#include ¡°threadPool.h¡±

using namespace std;

void func(int i)
{
	cout << i << endl;
}

int main()
{
	ThreadPool<void, int> threadpool;
	threadpool.setMaxTask(10);
	threadpool.start(5);
	threadpool.run(func, 1);
	threadpool.run(func, 2);
	threadpool.run(func, 3);
	threadpool.run(func, 4);
	threadpool.run(func, 5);

	return 0;
}
