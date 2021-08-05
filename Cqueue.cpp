//#include <iostream>
//#include <thread>
//#include <mutex>
//#include <chrono>
//#include <vector>
//
//
//using namespace std;
//using namespace chrono;
//
//constexpr int MAX_THREAD = 8;
//
//class NODE {
//public:
//	int key;
//	NODE* volatile next;
//
//	NODE()
//	{
//		next = nullptr;
//	}
//
//	NODE(int x)
//	{
//		key = x;
//		next = nullptr;
//	}
//	~NODE()
//	{
//	}
//};
//
//class null_mutex
//{
//public:
//	void lock()
//	{
//	}
//	void unlock()
//	{
//	}
//};
//
//class CQUEUE {
//	NODE* head, * tail;
//	mutex deque_lock;
//	mutex enque_lock;
//public:
//	CQUEUE()
//	{
//		head = tail = new NODE();
//	}
//	~CQUEUE()
//	{
//		clear();
//		delete head;
//	}
//
//	void clear()
//	{
//		while (head != tail) {
//			NODE* to_delete = head;
//			head = head->next;
//			delete to_delete;
//		}
//		head = tail;
//	}
//
//	void Enq(int x)
//	{
//		NODE* e = new NODE(x);
//		enque_lock.lock();
//		tail->next = e;
//		tail = e;
//		enque_lock.unlock();
//	}
//
//	int Deq()
//	{
//		deque_lock.lock();
//		if (nullptr == head->next)
//		{
//			deque_lock.unlock();
//			return -1;
//		}
//		int result = head->next->key;
//		NODE* temp = head;
//		head = head->next;
//		deque_lock.unlock();
//		delete temp;
//		return result;
//
//	}
//
//
//	void display20()
//	{
//		NODE* ptr = head->next;
//		for (int i = 0; i < 20; ++i) {
//			if (nullptr == ptr) break;
//			cout << ptr->key << ", ";
//			ptr = ptr->next;
//		}
//		cout << endl;
//	}
//};
//
//CQUEUE my_queue;
//constexpr int NUM_TEST = 10000000;
//
//void benchmark(int num_threads)
//{
//	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
//		if ((rand() % 2 == 0) || (i < 2 / num_threads))
//			my_queue.Enq(i);
//		else
//			my_queue.Deq();
//	}
//}
//
//int main()
//{
//	for (int num = 1; num <= MAX_THREAD; num = num * 2) {
//		vector <thread> threads;
//		my_queue.clear();
//		auto start_t = high_resolution_clock::now();
//		for (int i = 0; i < num; ++i)
//			threads.emplace_back(benchmark, num);
//		for (auto& th : threads) th.join();
//		auto end_t = high_resolution_clock::now();
//		auto du = end_t - start_t;
//		cout << num << " Threads,  ";
//		cout << "Exec time " <<
//			duration_cast<milliseconds>(du).count() << "ms  ";
//		my_queue.display20();
//	}
//}