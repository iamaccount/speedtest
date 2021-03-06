
#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

int64_t g_data_count = 0;

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket) : socket_(std::move(socket)) {
	}

	void start() {
		std::cerr << socket_.remote_endpoint() << std::endl;
		do_read();
	}

private:
	void do_read() {
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, max_length), [this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec) {
				do_read();
			}
			g_data_count += length;
		});
	}

	tcp::socket socket_;
	enum { max_length = 4096 };
	char data_[max_length];
};

class server
{
public:
	server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
		do_accept();
	}

private:
	void do_accept() {
		acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
			if (!ec) {
				std::make_shared<session>(std::move(socket))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
	try {
		if (argc != 2) {
			std::cerr << "Usage: server <port>\n";
			return 1;
		}
		std::cerr << "start server port:" << argv[1] << std::endl;
		boost::asio::io_context ioc;
		server s(ioc, std::atoi(argv[1]));
		std::thread t([&]() {
			ioc.run();
		});
		for (;;) {
			auto last_data = g_data_count;
			std::this_thread::sleep_for(std::chrono::seconds(10));
			std::cout << boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time()) 
				<< " data:" << (g_data_count - last_data) / 10 << " byte/s\n";
		}
		t.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
