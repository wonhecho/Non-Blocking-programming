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
class CQUEUE {
	NODE* head, * tail;
	mutex deque_lock;
	mutex enque_lock;
public:
	CQUEUE()
	{
		head = tail = new NODE();
	}
	~CQUEUE()
	{
		clear();
		delete head;
	}

	void clear()
	{
		while (head != tail) {
			NODE* to_delete = head;
			head = head->next;
			delete to_delete;
		}
		head = tail;
	}

	void Enq(int x)
	{
		NODE* e = new NODE(x);
		enque_lock.lock();
		tail->next = e;
		tail = e;
		enque_lock.unlock();
	}

	int Deq()
	{
		deque_lock.lock();
		if (nullptr == head->next)
		{
			deque_lock.unlock();
			return -1;
		}
		int result = head->next->key;
		NODE* temp = head;
		head = head->next;
		deque_lock.unlock();
		delete temp;
		return result;

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
					/*cout << "EMPTY\n";
					this_thread::sleep_for(1ms);*/
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

class STPTR {
public:
	long long int ptr;
	volatile int sta;
	STPTR() { ptr = 0; }
	STPTR(NODE* p, int stamp)
	{
		ptr = reinterpret_cast<int>(p);
		sta = stamp;
	}
	void set_ptr(NODE* p, int stamp)
	{
		ptr = reinterpret_cast<int>(p);
		sta = stamp;
	}
	NODE* get_addr()
	{
		return reinterpret_cast<NODE*>(ptr);
	}
	NODE* get_addr(volatile int *stamp)
	{
		*stamp = sta;
		return reinterpret_cast<NODE*>(ptr);
	}
};
class STLFQUEUE {
	STPTR head, tail;
public:
	STLFQUEUE()
	{
		NODE* p = new NODE();
		head = tail = STPTR(p, 0);
	}
	~STLFQUEUE() {
		clear();
		delete head.get_addr();
	}
	void clear()
	{
		while (head.get_addr() != tail.get_addr()) {
			NODE* to_delete = head.get_addr();
			head.set_ptr(head.get_addr()->next, 0);
			delete to_delete;
		}
		head = tail;
	}
	bool CAS(NODE* volatile* addr, NODE* old_NODE, NODE* new_NODE)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_int*>(addr), reinterpret_cast<int*>(&old_NODE), reinterpret_cast<int>(new_NODE));
	}
	bool STPCAS(STPTR* addr, NODE* old_NODE, NODE* new_NODE, int old_stamp)
	{
		STPTR old_v{ old_NODE,old_stamp };
		STPTR new_v{ new_NODE,old_stamp + 1 };
		return atomic_compare_exchange_strong(reinterpret_cast<atomic_llong*>(addr), reinterpret_cast<long long*>(&old_v), *(reinterpret_cast<long long*>(&new_v)));
	}
	void Enq(int key)
	{
		NODE* new_node = new NODE(key);
		
		while (true)
		{
			volatile int laststamp;
			NODE* last = tail.get_addr(&laststamp);
			NODE* next = last->next;
			if (last != tail.get_addr())continue;
			if (next == nullptr)
			{
				if (CAS(&(last->next), nullptr, new_node)) {
					STPCAS(&tail, last, new_node, laststamp);
					return;
				}
			}
			else STPCAS(&tail, last, next, laststamp);
			/*if (next != nullptr) {
				STPCAS(&tail, last, new_node,laststamp,laststamp+1);
				continue;
			}
			if (false == CAS(&(last->next), nullptr, new_node)) continue;
			STPCAS(&tail, last, new_node,laststamp,laststamp+1);
			return;*/
		}
	}
	int Deq()
	{
		
		while (true) {
			volatile int firststamp;
			NODE* first = head.get_addr(&firststamp);
			volatile int laststamp;
			NODE* next = first->next;
			NODE* last = tail.get_addr(&laststamp);
			NODE* lastnext = last->next;
			if (first != head.get_addr())continue;
			if (last == first) {
				if (lastnext == nullptr)
				{
					return -1;
				}
				else
				{
					STPCAS(&tail, last, lastnext, laststamp);
					continue;
				}
			}
			if (nullptr == next)continue;
			int result = next->key;
			if (false == STPCAS(&head, first, next, firststamp))continue;
			/*first->next = nullptr;*/
			delete first;
			return result;
		}
	}
	void display20()
	{
		NODE* ptr = head.get_addr()->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}

};

STLFQUEUE my_queue;
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