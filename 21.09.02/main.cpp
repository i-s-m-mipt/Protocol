#include <chrono>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>

namespace solution
{
	class Controller
	{
	public:

		using endpoint_t = boost::asio::ip::tcp::endpoint;

		using ip_address_t = boost::asio::ip::address;

		using socket_t = boost::asio::ip::tcp::socket;

		using sockets_container_t = std::vector < socket_t > ;

	private:

		struct Command
		{
			std::uint8_t  control_sum      = 0x00;
			std::uint8_t  protocol_version = 0x02; // default
			std::uint8_t  command_type     = 0x00;
			std::uint8_t  command_id       = 0x00;
			std::uint16_t data_length      = 0x00;

			std::uint8_t  data[1024] = {}; // max 1024
		};

	public:

		explicit Controller(const std::initializer_list < 
			std::pair < std::string, unsigned int > > & endpoints)
		{
			m_sockets.reserve(std::size(endpoints));

			for (const auto & [ip_address, port] : endpoints)
			{
				endpoint_t enpoint(ip_address_t::from_string(ip_address), port);

				socket_t socket(m_io_service, enpoint.protocol());

				socket.connect(enpoint);

				std::cout << "Socket on [" << ip_address << ":" << port <<
					"] successfully created" << std::endl;

				Command command;

				send_command(command, socket);

				command.data_length = 0x08;

				command.data[7] = 0xEF;
				command.data[6] = 0xCD;
				command.data[5] = 0xAB;
				command.data[4] = 0x89;
				command.data[3] = 0x67;
				command.data[2] = 0x45;
				command.data[1] = 0x23;
				command.data[0] = 0x01;

				send_command(command, socket);

				std::cout << "Controller on [" << ip_address << ":" << port <<
					"] ready to work" << std::endl;

				m_sockets.push_back(std::move(socket));
			}

			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		~Controller() noexcept = default;

	public:

		void move_forward()
		{
			Command command_1;
			Command command_2;

			// TODO

			send_command(command_1, m_sockets[0]);
			send_command(command_2, m_sockets[1]);
		}

		void move_backward()
		{
			Command command_1;
			Command command_2;

			// TODO

			send_command(command_1, m_sockets[0]);
			send_command(command_2, m_sockets[1]);
		}

		void stop()
		{
			Command command_1;
			Command command_2;

			// TODO

			send_command(command_1, m_sockets[0]);
			send_command(command_2, m_sockets[1]);
		}

	private:

		void send_command(Command & command, socket_t & socket)
		{
			update_control_sum(command);

			boost::asio::write(socket, boost::asio::buffer(
				&command, sizeof(command)));
		}

		void update_control_sum(Command & command)
		{
			command.control_sum = get_control_sum(
				(uint8_t *)&command.control_sum, sizeof(command));
		}

		std::uint8_t get_control_sum(std::uint8_t * data, std::uint16_t length)
		{
			std::uint8_t s = 0xFF;

			while (length--) 
			{ 
				s += *(data++);
			}

			return (s ^ 0xFF);
		}

	public:

		const auto & sockets() const noexcept
		{
			return m_sockets;
		}

	private:

		boost::asio::io_service m_io_service;

		sockets_container_t m_sockets;
	};

} // namespace solution

using Controller = solution::Controller;

int main(int argc, char ** argv)
{
	try
	{
		{
			Controller controller({
				{ "192.168.1.2", 5000U}, 
				{ "192.168.1.3", 5000U} });

			char command = ' ';

			while (command != 'e' && command != 'E')
			{
				std::cout << "Enter command (B - backward, S - stop, F - forward, E - exit) : ";

				std::cin >> command;

				switch (command)
				{
				case 'b': case 'B':
				{
					controller.move_backward();

					break;
				}
				case 's': case 'S':
				{
					controller.stop();

					break;
				}
				case 'f': case 'F':
				{
					controller.move_forward();

					break;
				}
				default:
				{
					break;
				}
				}
			}
		}

		system("pause");

		return EXIT_SUCCESS;
	}
	catch (const std::exception & exception)
	{
		std::cerr << exception.what() << std::endl;

		system("pause");

		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown exception" << std::endl;

		system("pause");

		return EXIT_FAILURE;
	}
}