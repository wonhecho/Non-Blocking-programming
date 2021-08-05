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
	NODE* volatile next;
	volatile bool is_removed;
	mutex n_lock;

	NODE()
	{
		is_removed = false;
		next = nullptr;
	}

	NODE(int x)
	{
		key = x;
		is_removed = false;
		next = nullptr;
	}
	~NODE()
	{
	}

	void Lock()
	{
		n_lock.lock();
	}

	void Unlock()
	{
		n_lock.unlock();
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

class CLIST {
	NODE head, tail;
	null_mutex m_lock;
public:
	CLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~CLIST()
	{

	}

	void clear()
	{
		NODE* ptr = head.next;
		while (ptr != &tail) {
			NODE* to_delete = ptr;
			ptr = ptr->next;
			delete to_delete;
		}
		head.next = &tail;
	}

	bool Add(int x)
	{
		NODE* pred = &head;
		m_lock.lock();
		NODE* curr = pred->next;
		while (curr->key < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == x) {
			m_lock.unlock();
			return false;
		}
		else {
			NODE* new_node = new NODE(x);
			new_node->next = curr;
			pred->next = new_node;
			m_lock.unlock();
			return true;
		}
	}

	bool Remove(int x)
	{

		NODE* pred = &head;
		m_lock.lock();
		NODE* curr = pred->next;
		while (curr->key < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key != x) {
			m_lock.unlock();
			return false;
		}
		else {
			pred->next = curr->next;
			delete curr;
			m_lock.unlock();
			return true;
		}
	}

	bool Contains(int x)
	{

		NODE* pred = &head;
		m_lock.lock();
		NODE* curr = pred->next;
		while (curr->key < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key != x) {
			m_lock.unlock();
			return false;
		}
		else {
			m_lock.unlock();
			return true;
		}
	}
	void display20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};

class FLIST {
	NODE head, tail;
public:
	FLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~FLIST()
	{

	}

	void clear()
	{
		NODE* ptr = head.next;
		while (ptr != &tail) {
			NODE* to_delete = ptr;
			ptr = ptr->next;
			delete to_delete;
		}
		head.next = &tail;
	}

	bool Add(int x)
	{
		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x) {
			pred->Unlock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}

		if (curr->key == x) {
			pred->Unlock();
			curr->Unlock();
			return false;
		}
		else {
			NODE* new_node = new NODE(x);
			new_node->next = curr;
			pred->next = new_node;
			pred->Unlock();
			curr->Unlock();
			return true;
		}
	}

	bool Remove(int x)
	{
		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x) {
			pred->Unlock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}

		if (curr->key != x) {
			pred->Unlock();
			curr->Unlock();
			return false;
		}
		else {
			pred->next = curr->next;
			delete curr;
			pred->Unlock();
			// curr->Unlock();
			return true;
		}
	}

	bool Contains(int x)
	{
		head.Lock();
		NODE* pred = &head;
		NODE* curr = pred->next;
		curr->Lock();
		while (curr->key < x) {
			pred->Unlock();
			pred = curr;
			curr = curr->next;
			curr->Lock();
		}

		if (curr->key != x) {
			pred->Unlock();
			curr->Unlock();
			return false;
		}
		else {
			pred->Unlock();
			curr->Unlock();
			return true;
		}
	}
	void display20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};

class OLIST {
	NODE head, tail;
public:
	OLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~OLIST()
	{

	}

	void clear()
	{
		NODE* ptr = head.next;
		while (ptr != &tail) {
			NODE* to_delete = ptr;
			ptr = ptr->next;
			delete to_delete;
		}
		head.next = &tail;
	}

	bool is_valid(NODE* pred, NODE* curr)
	{
		NODE* p = &head;
		while (p->key <= pred->key) {
			if (p == pred) {
				return pred->next == curr;
			}
			p = p->next;
		}
		return false;
	}

	bool Add(int x)
	{
		while (true) {
			NODE* pred = &head;
			NODE* curr = pred->next;
			while (curr->key < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key == x) {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				NODE* new_node = new NODE(x);
				new_node->next = curr;
				pred->next = new_node;
				pred->Unlock();
				curr->Unlock();
				return true;
			}
		}
	}

	bool Remove(int x)
	{
		while (true) {
			NODE* pred = &head;
			NODE* curr = pred->next;
			while (curr->key < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key != x) {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				pred->next = curr->next;
				pred->Unlock();
				curr->Unlock();
				// delete curr;
				return true;
			}
		}
	}

	bool Contains(int x)
	{
		while (true) {
			NODE* pred = &head;
			NODE* curr = pred->next;
			while (curr->key < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key == x) {
				pred->Unlock();
				curr->Unlock();
				return true;
			}
			else {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
		}
	}

	void display20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};

class LLIST {
	NODE head, tail;
public:
	LLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~LLIST()
	{

	}

	void clear()
	{
		NODE* ptr = head.next;
		while (ptr != &tail) {
			NODE* to_delete = ptr;
			ptr = ptr->next;
			delete to_delete;
		}
		head.next = &tail;
	}

	bool is_valid(NODE* pred, NODE* curr)
	{
		return (false == pred->is_removed) &&
			(false == curr->is_removed) &&
			pred->next == curr;
	}

	bool Add(int x)
	{
		while (true) {
			NODE* pred = &head;
			NODE* curr = pred->next;
			while (curr->key < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key == x) {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				NODE* new_node = new NODE(x);
				new_node->next = curr;
				pred->next = new_node;
				pred->Unlock();
				curr->Unlock();
				return true;
			}
		}
	}

	bool Remove(int x)
	{
		while (true) {
			NODE* pred = &head;
			NODE* curr = pred->next;
			while (curr->key < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key != x) {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				curr->is_removed = true;
				atomic_thread_fence(std::memory_order_seq_cst);
				pred->next = curr->next;
				pred->Unlock();
				curr->Unlock();
				// delete curr;
				return true;
			}
		}
	}

	bool Contains(int x)
	{
		NODE* curr = &head;
		while (curr->key < x)
			curr = curr->next;
		return (false == curr->is_removed) && (curr->key == x);
	}

	void display20()
	{
		NODE* ptr = head.next;
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};

class SPNODE {
public:
	int key;
	shared_ptr<SPNODE>  next;
	bool is_removed;
	mutex n_lock;

	SPNODE()
	{
		is_removed = false;
	}

	SPNODE(int x)
	{
		key = x;
		is_removed = false;
	}
	~SPNODE()
	{
	}

	void Lock()
	{
		n_lock.lock();
	}

	void Unlock()
	{
		n_lock.unlock();
	}
};

class SPLLIST {
	shared_ptr<SPNODE> head, tail;
public:
	SPLLIST()
	{
		head = make_shared<SPNODE>(0x80000000);
		tail = make_shared<SPNODE>(0x7FFFFFFF);
		head->next = tail;
	}
	~SPLLIST()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool is_valid(const shared_ptr<SPNODE>& pred, const shared_ptr<SPNODE>& curr)
	{
		return (false == pred->is_removed) &&
			(false == curr->is_removed) &&
			atomic_load(&pred->next) == curr;
	}

	bool Add(int x)
	{
		while (true) {
			shared_ptr<SPNODE> pred = head;
			shared_ptr<SPNODE> curr = atomic_load(&pred->next);
			while (curr->key < x) {
				pred = curr;
				curr = atomic_load(&curr->next);
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key == x) {
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				shared_ptr<SPNODE> new_node = make_shared<SPNODE>(x);
				new_node->next = curr;
				atomic_store(&pred->next, new_node);
				pred->Unlock();
				curr->Unlock();
				return true;
			}
		}
	}

	bool Remove(int x)
	{
		while (true) {
			shared_ptr<SPNODE> pred = head;
			shared_ptr<SPNODE> curr = atomic_load(&pred->next);
			while (curr->key < x) {
				pred = curr;
				curr = atomic_load(&curr->next);
			}

			pred->Lock();
			curr->Lock();

			if (false == is_valid(pred, curr))
			{
				pred->Unlock();
				curr->Unlock();
				continue;
			}

			if (curr->key == x) {
				curr->is_removed = true;
				atomic_thread_fence(memory_order_seq_cst);
				atomic_store(&pred->next, atomic_load(&curr->next));
				pred->Unlock();
				curr->Unlock();
				return false;
			}
			else {
				pred->Unlock();
				curr->Unlock();
				return true;
			}
		}
	}

	bool Contains(int x)
	{
		shared_ptr<SPNODE> curr = head;
		while (curr->key < x)
			curr = atomic_load(&curr->next);
		return (false == curr->is_removed) && (curr->key == x);
	}

	void display20()
	{
		shared_ptr<SPNODE> ptr = head->next;
		for (int i = 0; i < 20; ++i) {
			if (tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next;
		}
		cout << endl;
	}
};

class LFNODE {
	atomic_int next;
public:
	int key;
	


	LFNODE()
	{
		next = 0;
	}

	LFNODE(int x)
	{
		key = x;
		next = 0;
	}

	~LFNODE()
	{
	}

	void set_next(LFNODE* addr, bool is_removed)
	{
		int value = reinterpret_cast<int>(addr);
		if (true == is_removed) value = value | 1;
		next = value;
	}

	LFNODE* get_next()
	{
		return reinterpret_cast<LFNODE*>(next & 0xFFFFFFFE);
	}

	LFNODE* get_next(bool* is_removed)
	{
		int value = next;
		*is_removed = (0 != (value & 1));
		return reinterpret_cast<LFNODE*>(next & 0xFFFFFFFE);
	}

	bool is_removed()
	{
		return 1 == (next & 1);
	}

	bool CAS_NEXT(LFNODE* old_addr, LFNODE* new_addr, bool old_mark, bool new_mark)
	{
		int old_value = reinterpret_cast<int>(old_addr);
		if (true == old_mark) old_value = old_value | 1;
		int new_value = reinterpret_cast<int>(new_addr);
		if (true == new_mark) new_value = new_value | 1;
		return atomic_compare_exchange_strong(&next, &old_value, new_value);
	}
	bool AttemptMarking(LFNODE* old_node, bool new_removed)
	{
		int old_value, new_value;
		old_value = reinterpret_cast<int>(old_node);
		old_value = old_value & 0xFFFFFFFE;
		new_value = old_value;
		if (true == new_removed)new_value = new_value | 0x01;
		else new_value = new_value & 0xFFFFFFFE;
		return atomic_compare_exchange_strong(reinterpret_cast<atomic_int*>(&next), &old_value, new_value);
	}
};


class LFLIST {
	LFNODE head, tail;

public:
	LFLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.set_next(&tail, false);
	}
	~LFLIST()
	{
		clear();
	}

	void clear()
	{
		LFNODE* ptr = head.get_next();
		while (ptr != &tail) {
			LFNODE* to_delete = ptr;
			ptr = ptr->get_next();
			delete to_delete;
		}
		head.set_next(&tail, false);
	}

	void FIND(int key, LFNODE** pred, LFNODE** curr)
	{
	retry:
		LFNODE* pr = &head;
		LFNODE* cu = pr->get_next();

		while (true) {
			// cu�� ��ŷ�Ǿ� ������ �����ϰ� cu�� �ٽ� ����
			bool is_removed;
			LFNODE* su = cu->get_next(&is_removed);
			while (true == is_removed) {
				if (false == pr->CAS_NEXT(cu, su, false, false))
					goto retry;
				cu = su;
				su = cu->get_next(&is_removed);
			}

			if (cu->key >= key) {
				*pred = pr; *curr = cu;
				return;
			}
			pr = cu;
			cu = su;
		}
	}

	bool Add(int x)
	{
		LFNODE* pred = NULL;
		LFNODE* curr = NULL;


		while (true) {
			FIND(x, &pred, &curr);
			if (curr->key == x)return false;
			else {
				LFNODE* node = new LFNODE(x);
				node->set_next(curr, false);
				if (pred->CAS_NEXT(curr, node, false, false))
					return true;
			}
		}
		return true;
	}

	bool Remove(int x)
	{
		LFNODE* pred, * curr;
		bool snlp;
		while (true) {
			FIND(x, &pred, &curr);
			if (curr->key != x)return false;
			else {
				LFNODE* succ = curr->get_next();
				snlp = curr->AttemptMarking(succ, true);
				if (!snlp)
					continue;
				pred->CAS_NEXT(curr, succ, false, false);
				return true;
			}
		}
	}

	bool Contains(int x)
	{
		LFNODE* pred, * curr;
		bool marked = false;
		curr = &head;
		while (curr->key < x)
		{
			curr = curr->get_next();
			LFNODE* succ = curr->get_next(&marked);

		}
		return curr->key == x && !marked;
	}


	void display20()
	{
		LFNODE* ptr = head.get_next();
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->get_next();
		}
		cout << endl;
	}
};



constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

LFLIST my_set;

void benchmark(int num_threads)
{
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		switch (rand() % 3) {
		case 0:
			my_set.Add(rand() % KEY_RANGE);
			break;
		case 1:
			my_set.Remove(rand() % KEY_RANGE);
			break;
		case 2:
			my_set.Contains(rand() % KEY_RANGE);
			break;
		}
	}
}

constexpr int MAX_THREAD = 8;

int main()
{
	for (int num = 1; num <= MAX_THREAD; num = num * 2) {
		vector <thread> threads;
		my_set.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num; ++i)
			threads.emplace_back(benchmark, num);
		for (auto& th : threads) th.join();
		auto end_t = high_resolution_clock::now();
		auto du = end_t - start_t;

		cout << num << " Threads,  ";
		cout << "Exec time " <<
			duration_cast<milliseconds>(du).count() << "ms  ";
		my_set.display20();
	}
}