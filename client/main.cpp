
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;

enum { max_length = 4096 };

int main(int argc, char* argv[])
{
	try {
		if (argc != 3) {
			std::cerr << "Usage: test <host> <port>\n";
			return 1;
		}
		std::cerr << "connect to " << argv[1] << ":" << argv[2] << std::endl;
		boost::asio::io_context ioc;
		tcp::socket s(ioc);
		tcp::resolver resolver(ioc);
		boost::asio::connect(s, resolver.resolve(argv[1], argv[2]));

		int64_t data_count = 0;
		std::thread t([&]() {
			for (;;) {
				auto last_data = data_count;
				std::this_thread::sleep_for(std::chrono::seconds(10));
				std::cout << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time())
					<< " data:" << (data_count - last_data) / 10 << " byte/s\n";
			}
		});

		char request[max_length];
		for (;;) {
			boost::asio::write(s, boost::asio::buffer(request, max_length));
			data_count += max_length;
		}
		t.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
