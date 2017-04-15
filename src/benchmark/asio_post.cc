#include <iostream>
#include <cstdint>
#include <thread>
#include <vector>
#include <boost/asio.hpp>

boost::asio::io_service io;
uint64_t msg_count = 0;
static uint64_t count = 0;
uint64_t total_count = 0;
void Task()
{
	++count;
	if (count >= total_count) {
		io.stop();
	}
}

void Consumer()
{
    boost::system::error_code ec;
	io.run(ec);
}

void Producer()
{
	
    for (uint64_t i = 0; i < msg_count; ++i) {
		io.post(Task);
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



