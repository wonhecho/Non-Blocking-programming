///****************
//***2015154035****
//*****조원희******/
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

using namespace std;
using namespace chrono;

class NODE {																		//Node 클래스 선언
public:
	int key;																		
	NODE* next;																		//Key 값과 Link

NODE() { next = NULL; }
NODE(int key_value) {
		next = NULL;
		key = key_value;

	}
~NODE() {}
};

class CLIST {
	NODE head, tail;
	mutex glock;
public:
	CLIST()
	{
		head.key = 0x80000000;																//head 리스트 중 최솟값
		tail.key = 0x7FFFFFFF;																//tail 리스트 중 최댓값
		head.next = &tail;
	}
~CLIST() {}
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
		
		NODE* pred = &head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < x)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == x) {
			glock.unlock();
			return false;
		}
		else {
			NODE* new_node = new NODE(x);
			new_node->next = curr;
			pred->next = new_node;
			glock.unlock();
			return true;
		}


	}
	bool remove(int x)																				//리스트 삭제
	{
		
		NODE* pred = &head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < x)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == x) {
			pred->next = curr->next;
			delete curr;
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
			
		}

	}
	bool contains(int x)																				//리스트 확인
	{	
		NODE* pred = &head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < x)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == x) {
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}
	}

};
constexpr int NUM_TEXT = 4000000;

constexpr int KEY_RANGE = 1000;

CLIST my_set;

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

int main()
{
	for (int count = 1; count <= 16; count *= 2)
	{
		vector<thread> threads;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < count; i++) {
			threads.emplace_back(benchmark,count);
		}
		for (auto& t : threads)t.join();
		auto end_t = high_resolution_clock::now();
		auto exec_time = end_t - start_t;
		cout << "number of threads = " << count << ", ";
		cout << "Exec Time = " << duration_cast<milliseconds>(exec_time).count() << "ms" << endl;
		my_set.clear();																						//작업 후 리스트 초기화
	}

}
