#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>


using namespace std;
using namespace chrono;

constexpr int MAX_THREAD = 8;
int num_threads;
atomic_int el_success;

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
class CStack {
	NODE* top;
	mutex s_lock;
public:
	CStack()
	{
		top = nullptr;
	}
	~CStack()
	{
		clear();
	}

	void clear()
	{
		while (nullptr != top) {
			NODE* to_delete = top;
			top = top->next;
			delete to_delete;
		}
	}

	void Push(int x)
	{
		NODE* e = new NODE(x);
		s_lock.lock();
		e->next = top;
		top = e;
		s_lock.unlock();
	}

	int pop()
	{
		s_lock.lock();
		if (nullptr == top)
		{
			s_lock.unlock();
			return -2;
		}
	
		NODE* p = top;
		top = top->next;
		int value = p->key;
		s_lock.unlock();
		delete p;
		return value;

	}


	void display20()
	{
		NODE* ptr = top->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};
class LFStack {
public:
	NODE* volatile top;
	LFStack()
	{
		top = nullptr;
	}
	~LFStack()
	{
		clear();
	}
	bool CAS(NODE* volatile* ptr, NODE* o_node, NODE* n_node)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_int*>(ptr), reinterpret_cast<int*>(&o_node), reinterpret_cast<int>(n_node));
	}

	void clear()
	{
		while (nullptr != top) {
			NODE* to_delete = top;
			top = top->next;
			delete to_delete;
		}
	}

	void Push(int x)
	{
		NODE* e = new NODE(x);
		while (true) {
			NODE* first = top;
			if (top != first)continue;
			e->next = first;
			if(true == CAS(&top, first, e))
			return;

		}
	
	
	}		

	int pop()
	{
		while (true) {
			NODE* first = top;
			if (nullptr == first)
			{
				return -2;
			}
			NODE* next = first->next;
			int value = first->key;
			if (top != first)continue;
			if (true == CAS(&top, first, next))
				return value;

		}

	}


	void display20()
	{
		NODE* ptr = top->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}

};
class BackOff {
		int minDelay, maxDelay;
		int limit;
	public:
		void init(int min, int max) {
			minDelay=min;
			maxDelay=max;
			limit=min;
		}
		void InterruptedException() {
			int delay = 0;
			if (limit != 0) delay = rand() % limit;
			limit *= 2;
			if (limit > maxDelay) limit = maxDelay;
			this_thread::sleep_for(chrono::microseconds(delay));;
		}
		void backoff() {
			int delay = 0;
			if (limit != 0)
				delay = rand() % limit;
			limit *= 2;
			if (limit > maxDelay)
				limit = maxDelay;
			int start, current;
			_asm RDTSC;
			_asm mov start, eax;
			do {
				_asm RDTSC;
				_asm mov current, eax;
			} while (current - start < delay);
		}
		void backoff2()
		{
			int delay = 0;
			if (0 != limit) delay = rand() % limit;
			if (0 == delay) return;
			limit += limit;
			if (limit > maxDelay) limit = maxDelay;
			_asm mov eax, delay;
		myloop:
			_asm dec eax
			_asm jnz myloop;																		
		}
	};
	
class LFBOStack {
public:
	BackOff bo;
	NODE* volatile top;
	LFBOStack()
	{
		top = nullptr;
		bo.init(1, 1000000);
	}
	~LFBOStack()
	{
		clear();
	}
	bool CAS(NODE* volatile* ptr, NODE* o_node, NODE* n_node)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_int*>(ptr), reinterpret_cast<int*>(&o_node), reinterpret_cast<int>(n_node));
	}

	void clear()
	{
		while (nullptr != top) {
			NODE* to_delete = top;
			top = top->next;
			delete to_delete;
		}
	}

	void Push(int x)
	{
		NODE* e = new NODE(x);
		while (true) {
			NODE* first = top;
			if (top != first)continue;
			e->next = first;
			if (true == CAS(&top, first, e)) {
				bo.backoff2();
				return;
			}

		}


	}

	int pop()
	{
		while (true) {
			NODE* first = top;
			if (nullptr == first)
			{
				return -2;
			}
			NODE* next = first->next;
			int value = first->key;
			if (top != first)continue;
			if (true == CAS(&top, first, next))
				return value;

		}
		bo.backoff2();

	}


	void display20()
	{
		NODE* ptr = top->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}

};


constexpr int EMPTY_ST = 0;
constexpr int WAITING_ST = 1;
constexpr int BUSY_ST = 2;

class EXCHANGER {
	atomic_int slot;
	int get_state()
	{
		int t = slot;
		return (t >> 30) & 0x3;
	}
	int get_value()
	{
		int t = slot;
		return t & 0x3FFFFFFF;
	}
	bool CAS(int old_st, int new_st, int old_v, int new_v)
	{
		int ov = (old_st << 30) + (old_v & 0x3FFFFFFF);
		int nv = (new_st << 30) + (new_v & 0x3FFFFFFF);
		bool ret = atomic_compare_exchange_strong(&slot, &ov, nv);
		old_v = ov & 0x3FFFFFFF;
		return ret;
	}
public:
	EXCHANGER()
	{
		slot = 0;
	}
	~EXCHANGER() {}
	int exchange(int value, bool* time_out, bool* is_busy)
	{
		for (int i = 0; i < 100; ++i) {
			switch (get_state()) {
			case EMPTY_ST:
				if (true == CAS(EMPTY_ST, WAITING_ST, 0, value)) {
					int counter = 0;
					while (BUSY_ST != get_state()) {
						counter++;
						if (counter > 1000) {
							if (true == CAS(WAITING_ST, EMPTY_ST, value, 0)) {
								*time_out = true;
								return 0;
							} break;
						}
					};
					int ret = get_value();
					slot = 0;
					return ret;
				}
				else continue;
			case WAITING_ST: {
				int ret = get_value();
				if (true == CAS(WAITING_ST, BUSY_ST, ret, value))
					return ret;
				else continue;
			}
			case BUSY_ST:
				*is_busy = true;
				continue;
			}
		}
		*is_busy = true;
		*time_out = true;
		return 0;
	}
};
class ELIMINATION_ARRAY {
	int range;
	EXCHANGER ex[1 + MAX_THREAD / 2];
public:
	ELIMINATION_ARRAY() : range(1) {}
	~ELIMINATION_ARRAY() {};
	int visit(int value, bool* time_out)
	{
		int index = rand() % range;
		bool is_busy = false;
		int ret = ex[index].exchange(value, time_out, &is_busy);
		if ((true == *time_out) && (range > 1)) {
			range--; 
			el_success++;
		}
		if ((true == is_busy) && (range < num_threads / 2))
		{
			range++;
		}
		return ret;
	}
};
class LFELStack {
public:
	NODE* volatile top;
	ELIMINATION_ARRAY el;
	LFELStack()
	{
		top = nullptr;
	}
	~LFELStack()
	{
		clear();
	}
	bool CAS(NODE* volatile* ptr, NODE* o_node, NODE* n_node)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_int*>(ptr), reinterpret_cast<int*>(&o_node), reinterpret_cast<int>(n_node));
	}

	void clear()
	{
		while (nullptr != top) {
			NODE* to_delete = top;
			top = top->next;
			delete to_delete;
		}
	}

	void Push(int x)
	{
		NODE* e = new NODE(x);
		while (true) {
			NODE* first = top;
			if (top != first)continue;
			e->next = first;
			if (true == CAS(&top, first, e))
				return;
			bool time_out = false;
			int ret = el.visit(x,&time_out);
			if ((false == time_out) && (ret == -1))
				return;
		}


	}

	int pop()
	{
		while (true) {
			NODE* first = top;
			if (nullptr == first)
			{
				return -2;
			}
			NODE* next = first->next;
			int value = first->key;
			if (top != first)continue;
			if (true == CAS(&top, first, next))
				return value;

		}

	}


	void display20()
	{
		NODE* ptr = top->next;
		for (int i = 0; i < 20; ++i) {
			if (nullptr == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}

};
LFELStack my_stack;
constexpr int NUM_TEST = 10000000;

void benchmark(int num_threads)
{
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		if ((rand() % 2 == 0) || (i < 2 / num_threads))
			my_stack.Push(i);
		else
			my_stack.pop();
	}
}

int main()
{
	for (num_threads = 1; num_threads <= MAX_THREAD; num_threads = num_threads * 2) {
		vector <thread> threads;
		my_stack.clear();
		el_success = 0;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads);
		for (auto& th : threads) th.join();
		auto end_t = high_resolution_clock::now();
		auto du = end_t - start_t;
		cout << num_threads << " Threads,  ";
		cout << "Exec time " <<
			duration_cast<milliseconds>(du).count() << "ms  ";
		cout << "el_success : " << el_success<<endl;
		my_stack.display20();
	}
}