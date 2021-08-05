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
	mutex n_lock;																	//Key 값과 Link

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
class FLIST {
	NODE head, tail;
	mutex glock;
public:
	FLIST()
	{
		head.key = 0x80000000;																//head 리스트 중 최솟값
		tail.key = 0x7FFFFFFF;																//tail 리스트 중 최댓값
		head.next = &tail;
	}
	~FLIST() {}
	void clear()																			//리스트 초기화
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
	bool Add(int x)																			//리스트 추가
	{

		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x)
		{
			pred->unLock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}
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
	bool remove(int x)																				//리스트 삭제
	{
		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x)
		{
			pred->unLock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}
		if (curr->key == x) {
			pred->next = curr->next;
			curr->unLock();
			pred->unLock();
			delete curr;
			return true;
		}
		else {
			curr->unLock();
			pred->unLock();
			return false;

		}


	}
	bool contains(int x)																				//리스트 확인
	{
		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x)
		{
			pred->unLock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}
		if (curr->key == x) {
			curr->unLock();
			pred->unLock();
			return true;
		}
		else {
			curr->unLock();
			pred->unLock();
			return false;
		}

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

};

constexpr int NUM_TEXT = 4000000;

constexpr int KEY_RANGE = 1000;

FLIST my_set;

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