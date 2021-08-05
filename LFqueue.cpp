#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>


using namespace std;
using namespace chrono;

constexpr int MAX_THREAD = 8;

class NODE {
public:
	int key;
	NODE* volatile next;

	NODE()
	{
		next = nullptr;
	}

	NODE(int x)
	{
		key = x;
		next = nullptr;
	}
	~NODE()
	{
	}
};

class null_mutex
{
public:
	void lock()
	{
	}
	void unlock()
	{
	}
};

class LFQUEUE {
	NODE* volatile head;
	NODE* volatile tail;
public:
	LFQUEUE()
	{
		head = tail = new NODE(0);
	}
	~LFQUEUE() {}
	void clear()
	{
		while (head != tail) {
			NODE* to_delete = head;
			head = head->next;
			delete to_delete;
		}
		head = tail;
	}
	bool CAS(NODE* volatile* addr, NODE* old_NODE, NODE* new_NODE)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_int*>(addr), reinterpret_cast<int*>(&old_NODE), reinterpret_cast<int>(new_NODE));
	}
	void Enq(int key)
	{
		NODE* new_node = new NODE(key);
		while (true)
		{
			NODE* last = tail;
			NODE* next = last->next;
			if (last != tail)continue;
			if (next != nullptr) {
				CAS(&tail, last, next);
				continue;
			}
			if (false == CAS(&last->next, nullptr, new_node)) continue;
			CAS(&tail, last, new_node);
			return;
		}
	}
	int Deq()
	{
		while (true) {
			NODE* first = head;
			NODE* next = first->next;
			NODE* last = tail;
			NODE* lastnext = last->next;
			if (first != head)continue;
			if (last == first) {
				if (lastnext == nullptr)
				{
					return -1;
				}
				else
				{
					CAS(&tail, last, lastnext);
					continue;
				}
			}
			if (nullptr == next)continue;
			int result = next->key;
			if (false == CAS(&head, first, next))continue;
			first->next = nullptr;
			return result;
		}
	}
	void display20()
	{
		NODE* ptr = head->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}

};
LFQUEUE my_queue;
constexpr int NUM_TEST = 10000000;

void benchmark(int num_threads)
{
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		if ((rand() % 2 == 0) || (i < 2 / num_threads))
			my_queue.Enq(i);
		else
			my_queue.Deq();
	}
}

int main()
{
	for (int num = 1; num <= MAX_THREAD; num = num * 2) {
		vector <thread> threads;
		my_queue.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num; ++i)
			threads.emplace_back(benchmark, num);
		for (auto& th : threads) th.join();
		auto end_t = high_resolution_clock::now();
		auto du = end_t - start_t;
		cout << num << " Threads,  ";
		cout << "Exec time " <<
			duration_cast<milliseconds>(du).count() << "ms  ";
		my_queue.display20();
	}
}