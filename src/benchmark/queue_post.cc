#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>

typedef std::function<void()> Functor;
std::vector<Functor> functors;
std::mutex functors_mutex;

uint64_t msg_count = 0;
static uint64_t count = 0;
uint64_t total_count = 0;

void Task()
{
}

void Consumer()
{
	while (1) {
		std::vector<Functor> vec;
		{
			std::lock_guard<std::mutex> lock(functors_mutex);
			vec.swap(functors);
		}

		for (auto& f : vec) {
			f();
			++count;
		}

		if (count >= total_count) {
			break;
		}
	}
}

void Producer()
{
	for (uint64_t i = 0; i < msg_count; ++i) {
		std::lock_guard<std::mutex> lock(functors_mutex);
		functors.emplace_back(Task);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Usage post_test <producer_count> <msg_count>\n";
		return 1;
	}

	uint64_t producer_count = atoll(argv[1]);
	msg_count = atoll(argv[2]);
	total_count = producer_count * msg_count;

	std::vector<std::thread*> threads;
	for (uint16_t i = 0; i < producer_count; ++i){
		std::thread* t = new std::thread(Producer);
		threads.emplace_back(t);
	}

	Consumer();

	for (auto& t: threads) {
		t->join();
	}

	return 0;
}