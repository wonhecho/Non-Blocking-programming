#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <set>

using namespace std;
using namespace chrono;

constexpr int MAX_THREAD = 8;

constexpr int NUM_TEST = 400000;
constexpr int KEY_RANGE = 1000;


thread_local int thread_id;


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
	mutex m_lock;
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
			curr->Unlock();
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
				delete curr;
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
				delete curr;
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
		return reinterpret_cast<LFNODE*>(value & 0xFFFFFFFE);
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
		while (true) {
			LFNODE* pred, * curr;
			FIND(x, &pred, &curr);

			if (curr->key == x) {
				return false;
			}
			else {
				LFNODE* new_node = new LFNODE(x);
				new_node->set_next(curr, false);
				if (true == pred->CAS_NEXT(curr, new_node, false, false))
					return true;
			}
		}
	}

	bool Remove(int x)
	{
		while (true) {
			LFNODE* pred, * curr;
			FIND(x, &pred, &curr);

			if (curr->key != x) {
				return false;
			}
			else {
				LFNODE* succ = curr->get_next();
				if (false == curr->CAS_NEXT(succ, succ, false, true))
					continue;
				pred->CAS_NEXT(curr, succ, false, false);
				delete curr;
				return true;
			}
		}
	}

	bool Contains(int x)
	{
		LFNODE* curr = &head;
		while (curr->key < x)
			curr = curr->get_next();
		return (false == curr->is_removed()) && (curr->key == x);
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

class SET_LIST {
	set <int> m_set;
public:
	SET_LIST()
	{
	}
	~SET_LIST()
	{
	}

	void clear()
	{
		m_set.clear();
	}

	bool Add(int x)
	{
		if (0 != m_set.count(x)) return false;
		m_set.insert(x);
		return true;
	}

	bool Remove(int x)
	{
		if (0 == m_set.count(x)) return false;
		m_set.erase(x);
		return true;
	}

	bool Contains(int x)
	{
		return 0 != m_set.count(x);
	}

	void display20()
	{
		int counter = 20;
		for (auto v : m_set) {
			cout << v << ", ";
			if (counter-- <= 0) break;
		}
		cout << endl;
	}
};

class SET_CLIST {
	set <int> m_set;
	mutex m_l;
public:
	SET_CLIST()
	{
	}
	~SET_CLIST()
	{
	}

	void clear()
	{
		m_set.clear();
	}

	bool Add(int x)
	{
		lock_guard<mutex>L(m_l);
		if (0 != m_set.count(x)) return false;
		m_set.insert(x);
		return true;
	}

	bool Remove(int x)
	{
		lock_guard<mutex>L(m_l);
		if (0 == m_set.count(x)) return false;
		m_set.erase(x);
		return true;
	}

	bool Contains(int x)
	{
		lock_guard<mutex>L(m_l);
		return 0 != m_set.count(x);
	}

	void display20()
	{
		int counter = 20;
		for (auto v : m_set) {
			cout << v << ", ";
			if (counter-- <= 0) break;
		}
		cout << endl;
	}
};

constexpr int ADD = 1;
constexpr int REMOVE = 2;
constexpr int CONTAINS = 3;
constexpr int CLEAR = 4;
constexpr int DISPLAY_20 = 5;

struct Invocation {
	int method_type;
	int parameter1;
};

struct Response {
	bool resp1;
};

struct SeqObject_SET {
	set <int> m_set;
public:
	Response apply(Invocation inv)
	{
		switch (inv.method_type) {
		case ADD:
			if (0 != m_set.count(inv.parameter1)) return Response{ false };
			m_set.insert(inv.parameter1);
			return Response{ true };
		case REMOVE:
			if (0 == m_set.count(inv.parameter1)) return Response{ false };
			m_set.erase(inv.parameter1);
			return Response{ true };
		case CONTAINS:
			return Response{ 0 != m_set.count(inv.parameter1) };
		case CLEAR:
			m_set.clear();
			return Response();
		case DISPLAY_20:
			int counter = 20;
			for (auto v : m_set) {
				cout << v << ", ";
				if (counter-- <= 0) break;
			}
			cout << endl;
			return Response();
		}
	}
};

class SEQ_SET_LIST {
	SeqObject_SET m_set;
public:
	SEQ_SET_LIST()
	{
	}
	~SEQ_SET_LIST()
	{
	}

	bool Add(int x)
	{
		return m_set.apply(Invocation{ ADD, x }).resp1;
	}

	bool Remove(int x)
	{
		return m_set.apply(Invocation{ REMOVE, x }).resp1;
	}

	bool Contains(int x)
	{
		return m_set.apply(Invocation{ CONTAINS, x }).resp1;
	}

	void clear()
	{
		m_set.apply(Invocation{ CLEAR, 0 });
	}

	void display20()
	{
		m_set.apply(Invocation{ DISPLAY_20, 0 });
		return;
	}
};

class UNODE;

class CONSENSUS {
	atomic_int next;
public:
	CONSENSUS() {
		next = -1;
	}
	UNODE* decide(UNODE* value)
	{
		int v = reinterpret_cast<int> (value);
		int ov = -1;
		atomic_compare_exchange_strong(&next, &ov, v);
		v = next;
		return reinterpret_cast<UNODE*>(v);
	}
};

class UNODE {
public:
	Invocation inv;
	UNODE* next;
	int	seq;
	CONSENSUS decide_next;
public:
	UNODE(Invocation pinv)
	{
		inv = pinv;
		next = nullptr;
		seq = 0;
	}
	UNODE()
	{
		next = nullptr;
		seq = 0;
	}

};

class LFUniversal_SET {
	UNODE tail;
	UNODE* head[MAX_THREAD];
	UNODE* get_new()
	{
		UNODE* new_node = head[0];
		for (auto p : head) {
			if (new_node->seq < p->seq) new_node = p;
		}
		return new_node;
	}


public:
	LFUniversal_SET()
	{
		for (auto& p : head) p = &tail;
	}

	Response apply(Invocation inv)
	{
		UNODE* prefer = new UNODE(inv);
		while (0 == prefer->seq) {
			UNODE* before = get_new();
			UNODE* after = before->decide_next.decide(prefer);
			before->next = after;
			after->seq = before->seq + 1;
			head[thread_id] = after;
		}
		SeqObject_SET s_set;
		UNODE* p = tail.next;
		while (p != prefer) {
			s_set.apply(p->inv);
			p = p->next;
		}
		return s_set.apply(inv);
	}
	
	void clear()
	{
		UNODE *newtail=get_new();
		newtail->inv.method_type = 0; newtail->inv.parameter1 = 0;
		tail.next = newtail;
	}

};

class LF_SET_LIST {
	LFUniversal_SET m_set;
public:
	LF_SET_LIST()
	{
	}
	~LF_SET_LIST()
	{
	}

	bool Add(int x)
	{
		return m_set.apply(Invocation{ ADD, x }).resp1;
	}

	bool Remove(int x)
	{
		return m_set.apply(Invocation{ REMOVE, x }).resp1;
	}

	bool Contains(int x)
	{
		return m_set.apply(Invocation{ CONTAINS, x }).resp1;
	}

void clear()
{
	m_set.apply(Invocation{ CLEAR, 0 });

}

void display20()
{
	m_set.apply(Invocation{ DISPLAY_20, 0 });
	return;
}
void logclear()
{
	m_set.clear();
}
};

constexpr int MAX_LEVEL = 10;
class SKNODE {
public:
	int key;
	SKNODE* volatile next[MAX_LEVEL + 1];
	int toplevel;

	SKNODE()
	{
		key = 0;
		for (auto& p : next)p = nullptr;
		toplevel = MAX_LEVEL;
	}

	~SKNODE()
	{
	}
	SKNODE(int x, int top)
	{
		key = x;
		for (auto& p : next)p = nullptr;
		toplevel = top;
	}
};


class SKCLIST {
	SKNODE head, tail;
	mutex m_lock;
public:
	SKCLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.toplevel = tail.toplevel = MAX_LEVEL;
		for (auto& n : head.next)n = &tail;
	}
	~SKCLIST()
	{
		clear();

	}

	void clear()
	{
		SKNODE* ptr = head.next[0];
		while (ptr != &tail) {
			SKNODE* to_delete = ptr;
			ptr = ptr->next[0];
			delete to_delete;
		}
		for (auto& n : head.next)n = &tail;
	}
	void Find(int x, SKNODE* preds[MAX_LEVEL+1], SKNODE* currs[MAX_LEVEL+1])
	{
		
		preds[MAX_LEVEL] = &head;
		for (int cl = MAX_LEVEL; cl >= 0; --cl) {
			if (cl != MAX_LEVEL)
				preds[cl] = preds[cl + 1];
			currs[cl] = preds[cl]->next[cl];
			while (x > currs[cl]->key) {
				preds[cl] = currs[cl];
				currs[cl] = currs[cl]->next[cl];
			}
		}
		
	}
	bool Add(int x)
	{
		SKNODE* pred[MAX_LEVEL + 1];
		SKNODE* curr[MAX_LEVEL + 1];
		m_lock.lock();
		Find(x, pred, curr);
		if (curr[0]->key == x) {
			m_lock.unlock();
			return false;
		}
		else {
			int toplevel = 0;
			while ((rand() % 2) == 0){
			toplevel++;
			if (toplevel == MAX_LEVEL)
				break;
			}
			SKNODE* new_node = new SKNODE(x,toplevel);
			for (int i = 0; i < toplevel; ++i)
				new_node->next[i] = curr[i];
			for (int i = 0; i < toplevel; ++i)
				pred[i]->next[i] = new_node;
			m_lock.unlock();
			return true;
		}
	}

	bool Remove(int x)
	{

		SKNODE* pred[MAX_LEVEL+1];
		SKNODE* curr[MAX_LEVEL + 1];
		m_lock.lock();
		Find(x, pred, curr);
		if (curr[0]->key != x) {
			m_lock.unlock();
			return false;
		}
		else {
			for (int i = 0; i < curr[0]->toplevel; ++i)
			{
				pred[i]->next[i] = curr[i]->next[i];
			}
			delete curr[0];
			m_lock.unlock();
			return true;
		}
	}

	bool Contains(int x)
	{

		SKNODE* pred[MAX_LEVEL + 1];
		SKNODE* curr[MAX_LEVEL + 1];
		m_lock.lock();
		Find(x, pred, curr);
		if (curr[0]->key != x) {
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
		SKNODE* ptr = head.next[0];
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next[0];
		}
		cout << endl;
	}
};
class LSKNODE {
public:
	int key;
	LSKNODE* volatile next[MAX_LEVEL + 1];
	int toplevel;
	volatile bool is_removed;
	volatile bool is_fully_linked;
	recursive_mutex n_lock;

	LSKNODE()
	{
		for (auto& p : next) p = nullptr;
		toplevel = MAX_LEVEL;
		is_removed = false;
		is_fully_linked = false;
	}
	~LSKNODE() {}
	LSKNODE(int x, int top)
	{
		key = x;
		for (auto& p : next) p = nullptr;
		toplevel = top;
		is_removed = false;
		is_fully_linked = false;
	}
};

class LSKLIST {
	LSKNODE head, tail;
public:
	LSKLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		for (auto& n : head.next) n = &tail;
		head.toplevel = tail.toplevel = MAX_LEVEL;
		head.is_fully_linked = tail.is_fully_linked = true;
	}
	~LSKLIST()
	{
		clear();
	}

	void clear()
	{
		LSKNODE* ptr = head.next[0];
		while (ptr != &tail) {
			LSKNODE* to_delete = ptr;
			ptr = ptr->next[0];
			delete to_delete;
		}
		for (auto& n : head.next) n = &tail;
	}

	int Find(int x, LSKNODE* preds[], LSKNODE* currs[])
	{
		int l_found = -1;
		preds[MAX_LEVEL] = &head;
		for (int cl = MAX_LEVEL; cl >= 0; --cl) {
			if (cl != MAX_LEVEL)
				preds[cl] = preds[cl + 1];
			currs[cl] = preds[cl]->next[cl];
			while (x > currs[cl]->key) {
				preds[cl] = currs[cl];
				currs[cl] = currs[cl]->next[cl];
			}
			if ((x == currs[cl]->key) && (-1 == l_found))
				l_found = cl;
		}
		return l_found;
	}

	bool Add(int x)
	{
		LSKNODE* preds[MAX_LEVEL + 1];
		LSKNODE* currs[MAX_LEVEL + 1];

		int toplevel = 0;
		while (0 == (rand() % 2)) {
			toplevel++;
			if (MAX_LEVEL == toplevel) break;
		}

		while (true) {
			int l_found = Find(x, preds, currs);

			if (-1 != l_found) {
				if (true == currs[l_found]->is_removed)
					continue;
				while (false == currs[l_found]->is_fully_linked);
				return false;
			}
			bool valid = true;
			int lock_index = 0;
			for (int i = 0; i <= toplevel; ++i) {
				preds[i]->n_lock.lock();
				lock_index = i;
				valid = (false == preds[i]->is_removed) &&
					(false == currs[i]->is_removed) &&
					(preds[i]->next[i] == currs[i]);
				if (false == valid) break;
			}
			if (false == valid) {
				for (int i = 0; i <= lock_index; ++i)
					preds[i]->n_lock.unlock();
				continue;
			}

			LSKNODE* new_node = new LSKNODE(x, toplevel);

			for (int i = 0; i <= toplevel; ++i)
				new_node->next[i] = currs[i];
			for (int i = 0; i <= toplevel; ++i)
				preds[i]->next[i] = new_node;

			for (int i = 0; i <= lock_index; ++i)
				preds[i]->n_lock.unlock();
			new_node->is_fully_linked = true;
			return true;
		}
	}

	bool Remove(int x)
	{
		LSKNODE* pred[MAX_LEVEL + 1];
		LSKNODE* curr[MAX_LEVEL + 1];
		LSKNODE* victim = nullptr;
		bool is_marked = false;
		int toplevel = 0;

		while (true) {
			int l_found = Find(x, pred, curr);
			if (-1 != l_found)
			{
				victim = curr[l_found];
			}
			if (is_marked || (l_found != -1 &&(victim->is_fully_linked && victim->toplevel == l_found && !(victim->is_removed))))
			{
				if (!is_marked)
				{
					toplevel = victim->toplevel;
					victim->n_lock.lock();

					if (victim->is_removed) {
						victim->n_lock.unlock();
						return false;
					}
					victim->is_removed = true;
					is_marked = true;
				}
				int highestLocked = -1;
				bool valid = true;
				for (int i = 0; (valid)&&(i <= toplevel); i++)
				{
					pred[i]->n_lock.lock();
					highestLocked = i;
					valid = (!pred[i]->is_removed) && (pred[i]->next[i]==victim);
				}
				if (!valid)
				{
					for (int i = 0; i <= highestLocked; i++)
						pred[i]->n_lock.unlock();
					continue;
				}
				for (int i = toplevel; i >= 0; i--)
				{
					pred[i]->next[i] = victim->next[i];
				}
				for (int i = 0; i <= highestLocked; i++)
					pred[i]->n_lock.unlock();
				victim->n_lock.unlock();
				return true;
			}
			else return false;
		}
	}

	bool Contains(int x)
	{
		LSKNODE* pred[MAX_LEVEL + 1];
		LSKNODE* curr[MAX_LEVEL + 1];
		int l_found = Find(x, pred, curr);
		return (l_found != -1 && curr[l_found]->is_fully_linked && !(curr[l_found]->is_removed));


	}
	void display20()
	{
		LSKNODE* ptr = head.next[0];
		for (int i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << ", ";
			ptr = ptr->next[0];
		}
		cout << endl;
	}
};
LSKLIST my_set;

void benchmark(int num_threads, int t_id)
{
	thread_id = t_id;
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

int main()
{
	for (int num = 1; num <= MAX_THREAD; num = num * 2) {
		vector <thread> threads;
		my_set.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num; ++i)
			threads.emplace_back(benchmark, num, i);
		for (auto& th : threads) th.join();
		auto end_t = high_resolution_clock::now();
		auto du = end_t - start_t;
		cout << num << " Threads,  ";
		cout << "Exec time " <<
			duration_cast<milliseconds>(du).count() << "ms  ";
		my_set.display20();
		/*my_set.logclear();*/
	}
}
