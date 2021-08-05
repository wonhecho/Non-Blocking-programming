/****************
***2015154035****
*****������******/

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
using namespace std;
using namespace chrono;
mutex sum_lock;

volatile int sum;
const int MAX_THREAD = 16;
volatile bool Flag[MAX_THREAD] = { false, };
volatile int Label[MAX_THREAD] = { 0, };

int Compare(int i, int j)								//Flag���¸� Ȯ���Ͽ� Ž������ �������� �󺧰��� �ڽ� �������� �󺧰��� ��.
{
	if (Label[i] < Label[j])						//�ڽ� �������� �󺧰��� ũ�� true����
		return 1;
	else if (Label[i] > Label[j])					//Ž�� �������� �󺧰��� ũ�� false����
		return 0;
	else                                            //�󺧰��� ���� ��� ���̵� ���� ���ڸ� ã�Ƴ���.
		if (i < j)									//�ڽ� �������� ���̵� Ŭ��� true����
			return 1;
		else if(i>j)								//Ž�� �������� ���̵� Ŭ��� false�� �����մϴ�
			return 0;
}

void bakery_lock(int my_id, int thread_num)
{
	/*atomic_thread_fence(memory_order_seq_cst);*/		//atomic fence
	Flag[my_id] = true;								//�ڱ� ���̵��� flag�� true ���·� �д�.
	int m = 0;										//max�Լ��� ����� �̹� �ִ� Label�߿��� ���� ���� ���� ã��
	for (int j = 1; j <= thread_num; j++)
	{
		int jn = Label[j];
		m = jn > m ? jn : m;
	}					

	Label[my_id] = 1 + m;							//�ڱ� ���̵� Label�� 1�� ���Ͽ� ����
	
	for (int k = 1; k <= thread_num; k++)
	{
		/*atomic_thread_fence(memory_order_seq_cst);*/							//automic fence
		if (k == my_id)continue;											//Ž������ k�� �ڽ��� ���̵� ������ ����
		/*atomic_thread_fence(memory_order_seq_cst);*/
		while ((Flag[k] != 0) && (Compare(k,my_id))){ this_thread::yield(); }					
	}

}

void bakery_unlock(int my_id)
{
	
	Flag[my_id] = false;

}
void thread_func(int t_id,int num_thread)
{
	
	for (int i = 0; i < 50000000/num_thread; ++i)
	{
		bakery_lock(t_id,num_thread);
		/*sum_lock.lock();*/
		sum += 2;
		/*sum_lock.unlock();*/
		bakery_unlock(t_id);
	}
}

int main()
{
	for (int count = 1; count <= 16; count *= 2)
	{
		vector<thread> threads;
		sum = 0;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < count; i++)
			threads.emplace_back(thread_func, i,count);
		for (auto& t : threads)t.join();
		auto end_t = high_resolution_clock::now();
		auto exec_time = end_t - start_t;
		cout << "bakery  :" << endl;
		cout << "Label of threads = " << count << ", ";
		cout << "Exec Time = " << duration_cast<milliseconds>(exec_time).count() << "ms , ";
		cout << "SUM = " << sum << endl;

	}
}