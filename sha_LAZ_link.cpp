#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

using namespace std;
using namespace chrono;

class NODE {

public:
	int key;
	shared_ptr<NODE>next;
	mutex n_lock;
	bool marked;

	NODE() { next = nullptr; marked = false; }
	NODE(int key_value) {
		next = NULL;
		key = key_value;
		marked = false;

	}
	~NODE() {}
	void Lock()
	{
		n_lock.lock();
	}
	void unLock()
	{
		n_lock.unlock();
	}
};

class LLIST {
	shared_ptr<NODE>head, tail;
	

public:
	LLIST()
	{
		head = make_shared<NODE>(0x80000000);
		tail = make_shared<NODE>(0x7FFFFFFF);																
		head->next = tail;
	}
	~LLIST() {}
	void clear()																			
	{
		head->next = tail;
	}
	bool Add(int x)																			
	{
		shared_ptr<NODE>pred, curr;
		pred = head;
		curr = pred->next;
		while (curr->key < x)
		{
			pred = curr;
			curr = curr->next;
		}
		pred->Lock(); curr->Lock();
		if (validate(pred, curr)) {
			if (curr->key == x) {
				curr->unLock();
				pred->unLock();
				return false;
			}
			else {
				shared_ptr<NODE>new_node = make_shared<NODE>(x);
				new_node->next = curr;
				pred->next = new_node;
				curr->unLock();
				pred->unLock();
				return true;
			}

		}
		curr->unLock();
		pred->unLock();
	}
	bool remove(int x)																				
	{
		shared_ptr<NODE>pred, curr;
		pred = atomic_load(&head);
		curr = atomic_load(&pred->next);
		while (curr->key < x)
		{
			pred = curr; curr = curr->next;
		}
		pred->Lock(); curr->Lock();
		if (validate(pred, curr)) {
			if (curr->key == x) {
				pred->next = curr->next;
				pred->unLock();
				curr->unLock();
				return true;
			}
			else {
				pred->unLock();
				curr->unLock();
				return false;
			}
		}
		curr->unLock();
		pred->unLock();

	}
	bool contains(int x)																				
	{
		shared_ptr<NODE>pred, curr;
		curr = head;
		while (curr->key < x)
		{
			curr = curr->next;
		}
		return curr->key == x && !curr->marked;

	}
	void display20()
	{
		shared_ptr<NODE>ptr = head->next;
		for (int i = 0; i < 20; ++i)
		{
			if (tail == ptr) break;
			cout << ptr->key << ",";
			ptr = ptr->next;
		}
		cout << endl;
	}
	bool validate(const shared_ptr<NODE> &pred,const shared_ptr<NODE> &curr) {
		return !pred->marked && !curr->marked && pred->next == curr;
	}
	void recycle_freelist()
	{
		return;
	}

};


constexpr int NUM_TEXT = 4000000;

constexpr int KEY_RANGE = 1000;

LLIST my_set;

void benchmark(int num_threads)
{
	for (int i = 0; i < NUM_TEXT / num_threads; i++)
	{
		switch (rand() % 3) {
		case 0:
			my_set.Add(rand() % KEY_RANGE);
			break;
		case 1:
			my_set.remove(rand() % KEY_RANGE);
			break;
		case 2:
			my_set.contains(rand() % KEY_RANGE);
			break;

		}

	}
}
constexpr int MAX_THREAD = 8;
int main()
{
	for (int num = 1; num <= MAX_THREAD; num = num * 2) {
		my_set.clear();
		vector <thread> threads;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num; ++i)
			threads.emplace_back(benchmark, num);
		for (auto& t : threads)t.join();
		auto end_t = high_resolution_clock::now();
		auto exec_time = end_t - start_t;
		cout << num << " Threads : ";
		cout << "Exec time " << duration_cast<milliseconds>(exec_time).count() << "ms" << endl;
		my_set.display20();
	}
}

