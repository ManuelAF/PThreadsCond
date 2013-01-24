#include <Windows.h>
#include <pthread.h>
#include <vector>
#include <queue>

/*
template<typename Data>
class concurrent_queue {

private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;

public:
    void push(Data const& data) {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    bool empty() const {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value) {
        boost::mutex::scoped_lock lock(the_mutex);
        if (the_queue.empty()) {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value) {
        boost::mutex::scoped_lock lock(the_mutex);
        while (the_queue.empty()) {
            the_condition_variable.wait(lock);
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
    }
};
	
*/

bool IsPrime(size_t N) {

	if (0 == (N % 2)) {
		return false;
	}

	if (0 == (N % 3)) {
		return false;
	}

	size_t K = 1;
	size_t T = (6 * K);
	
	while ( ((T - 1) * (T - 1)) <= N) {

		if (0 == (N % (T - 1))) {
			return false;
		}

		if (0 == (N % (T + 1))) {
			return false;
		}

		++K;
		T = (6 * K);
	}

	return true;
}

typedef struct TDataTAG {
	std::queue<size_t> Queue_;
	pthread_mutex_t Mutex_;
	pthread_cond_t Cond_;
} TData;

void* ProducerThread(void* D) {

	TData *Data = (TData*)D;

	for (size_t i = 0; i < 100; ++i) {

		pthread_mutex_lock(&Data->Mutex_);

		for (size_t j = 0; j < 10; ++j) {
			size_t t = (j + 1) * 1000 + i;
			printf("Produciendo: %d Queue Size: %d\n", t, Data->Queue_.size());
			Data->Queue_.push(t);
		}

		//pthread_cond_signal(&Data->Cond_);
		pthread_cond_broadcast(&Data->Cond_);

		pthread_mutex_unlock(&Data->Mutex_);

		Sleep(3 * 1000);
	} 

	return NULL;
}

void* ConsumerThread(void* D) {

	pthread_t Self = pthread_self();

	TData *Data = (TData*)D;

	while (1) {

		pthread_mutex_lock(&Data->Mutex_);
		while (Data->Queue_.empty()) {
			pthread_cond_wait(&Data->Cond_, &Data->Mutex_);
		}
		
		size_t N = Data->Queue_.front(); Data->Queue_.pop();
			
		pthread_mutex_unlock(&Data->Mutex_);

		printf("Procesando %p: %d\n", Self.p, N);
		//IsPrime(N);
		//Sleep(1 * 1000);
		//Sleep(25);

	}  

	return NULL;
}

int main(int argc, char *argv[]) {

	TData Data;

	pthread_mutex_init(&Data.Mutex_, NULL);
	pthread_cond_init(&Data.Cond_, NULL);

	std::vector<pthread_t> Consumers;
	const size_t NumberOfConsumers = 2;
	for (size_t N = 0; N < NumberOfConsumers; ++N) {
		pthread_t PThread;
		pthread_create(&PThread, NULL, ConsumerThread, (void*)&Data);
		Consumers.push_back(PThread);
	}

	pthread_t Producer;

	int Error = pthread_create(&Producer, NULL, ProducerThread, (void*)&Data);

	for (size_t N = 0; N < NumberOfConsumers; ++N) {
		pthread_join(Consumers[N], NULL);
	}
}
