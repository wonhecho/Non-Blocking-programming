/****************
***2015154035****
*****조원희******/

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

int Compare(int i, int j)								//Flag상태를 확인하여 탐색중인 쓰레드의 라벨값과 자신 쓰레드의 라벨값을 비교.
{
	if (Label[i] < Label[j])						//자신 쓰레드의 라벨값이 크면 true리턴
		return 1;
	else if (Label[i] > Label[j])					//탐색 쓰레드의 라벨값이 크면 false리턴
		return 0;
	else                                            //라벨값이 같은 경우 아이디가 낮은 숫자를 찾아낸다.
		if (i < j)									//자신 쓰레드의 아이디가 클경우 true리턴
			return 1;
		else if(i>j)								//탐색 쓰레드의 아이디가 클경우 false를 리턴합니다
			return 0;
}

void bakery_lock(int my_id, int thread_num)
{
	/*atomic_thread_fence(memory_order_seq_cst);*/		//atomic fence
	Flag[my_id] = true;								//자기 아이디의 flag를 true 상태로 둔다.
	int m = 0;										//max함수의 결과값 이미 있는 Label중에서 가장 높은 라벨을 찾음
	for (int j = 1; j <= thread_num; j++)
	{
		int jn = Label[j];
		m = jn > m ? jn : m;
	}					

	Label[my_id] = 1 + m;							//자기 아이디 Label에 1을 더하여 대입
	
	for (int k = 1; k <= thread_num; k++)
	{
		/*atomic_thread_fence(memory_order_seq_cst);*/							//automic fence
		if (k == my_id)continue;											//탐색중인 k와 자신의 아이디가 같을때 생략
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