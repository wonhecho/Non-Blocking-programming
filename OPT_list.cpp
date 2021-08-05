#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

using namespace std;
using namespace chrono;

class NODE {

public:
	int key;
	NODE* next;
	mutex n_lock;																	

	NODE() { next = NULL; }
	NODE(int key_value) {
		next = NULL;
		key = key_value;

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

class OLIST {
	NODE head, tail;
	NODE* freelist;
	NODE freetail;
	mutex OPT;

public:
	OLIST()
	{
		head.key = 0x80000000;																
		tail.key = 0x7FFFFFFF;																
		head.next = &tail;
		freetail.key = 0x7FFFFFFF;
		freelist = &freetail;
	}
	~OLIST() {}
	void clear()																			
	{
		NODE* ptr = head.next;
		while (ptr != &tail)
		{
			NODE* to_delete = ptr;
			ptr = ptr->next;
			delete to_delete;
		}
		head.next = &tail;
	}
	bool Add(int x)																			
	{
		NODE* pred = &head;
		NODE* curr = pred->next;
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
				NODE* new_node = new NODE(x);
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
		NODE* pred = &head;
		NODE* curr = pred->next;
		while (curr->key < x)
		{
			pred = curr; curr = curr->next;
		}
		pred->Lock(); curr->Lock();
		if (validate(pred, curr)) {
			if (curr->key == x) {
				pred->next = curr->next;
				OPT.lock();
				curr->next = freelist;
				freelist = curr;
				OPT.unlock();
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
	bool contains(int x)																				//리스트 확인
	{
		NODE* pred = &head;
		NODE* curr = pred->next;
		while (curr->key < x) {
			pred = curr; curr = curr->next;
		}
		pred->Lock(); curr->Lock();
		if (validate(pred, curr)) {
			pred->unLock();
			curr->unLock();
			return (curr->key == x);
		}
		curr->unLock();
		pred->unLock();

	}
	void display20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i)
		{
			if (&tail == ptr) break;
			cout << ptr->key << ",";
			ptr = ptr->next;
		}
		cout << endl;
	}
	bool validate(NODE* pred, NODE* curr) {
		NODE* node = &head;
		while (node->key <= pred->key) {
			if (node == pred)
				return pred->next == curr;
			node = node->next;
		}
		return false;
	}
	void del_freelist()
	{
		NODE* node = freelist;
		while (node != &freetail)
		{
			NODE* change_node = node->next;
			delete node;
			node = change_node;
		}
		freelist = &freetail;
	}

};


constexpr int NUM_TEXT = 4000000;

constexpr int KEY_RANGE = 1000;

OLIST my_set;

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
		my_set.del_freelist();
		
	}
}