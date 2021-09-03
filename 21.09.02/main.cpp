#include <chrono>
#include <cstdlib>
#include <initializer_list>
#include <iomanip>
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

		struct Command_REQUEST
		{
			std::uint8_t  control_sum      = 0x00;
			std::uint8_t  protocol_version = 0x03;
			std::uint8_t  command_type     = 0x00;
			std::uint8_t  command_id       = 0x00;
			std::uint16_t data_length      = 0x00;
		};

		struct Command_PASSWORD
		{
			std::uint8_t  control_sum      = 0x00;
			std::uint8_t  protocol_version = 0x03;
			std::uint8_t  command_type     = 0x00;
			std::uint8_t  command_id       = 0x00;
			std::uint16_t data_length      = 0x08;

			std::uint8_t  data[8] = {};
		};

		struct Command_RESPONSE
		{
			std::uint8_t  control_sum      = 0x00;
			std::uint8_t  protocol_version = 0x03;
			std::uint8_t  command_type     = 0x01;
			std::uint8_t  command_id       = 0x00;
			std::uint16_t data_length      = 0x07;

			std::uint8_t  data[7] = {};
		};

		struct Command_MOVE
		{
			std::uint8_t  control_sum      = 0x00;
			std::uint8_t  protocol_version = 0x03;
			std::uint8_t  command_type     = 0x02;
			std::uint8_t  command_id       = 0x00;
			std::uint16_t data_length      = 0x04;

			std::uint8_t  data[4] = {};
		};

		struct SMSD_Command
		{
			std::uint32_t reserve : 3;
			std::uint32_t action  : 1;
			std::uint32_t command : 6;
			std::uint32_t data    : 22;
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

				const auto length_request = sizeof(Command_REQUEST);

				char buffer_request[length_request];

				boost::asio::read(socket, boost::asio::buffer(buffer_request, length_request));

				print_data(buffer_request, length_request);

				Command_PASSWORD command_password;
				
				command_password.data[0] = 0xEF;
				command_password.data[1] = 0xCD;
				command_password.data[2] = 0xAB;
				command_password.data[3] = 0x89;
				command_password.data[4] = 0x67;
				command_password.data[5] = 0x45;
				command_password.data[6] = 0x23;
				command_password.data[7] = 0x01;

				send_command(command_password, socket);

				const auto length_response = sizeof(Command_RESPONSE) - 1;

				char buffer_response[length_response];

				boost::asio::read(socket, boost::asio::buffer(buffer_response, length_response));

				print_data(buffer_response, length_response);

				std::cout << "Controller on [" << ip_address << ":" << port <<
					"] ready to work" << std::endl;

				m_sockets.push_back(std::move(socket));
			}

			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		~Controller() noexcept = default;

	public:

		void move(char direction, std::uint32_t speed = 0)
		{
			SMSD_Command smsd_command;

			smsd_command.reserve = 0;
			smsd_command.action  = 0;

			const std::uint32_t min_speed = 15;
			const std::uint32_t max_speed = 15600;

			smsd_command.data = std::min(std::max(speed, min_speed), max_speed);

			switch (direction)
			{
			case 'f': case 'F':
			{
				smsd_command.command = 0x0E;

				break;
			}
			case 'b': case 'B':
			{
				smsd_command.command = 0x0F;

				break;
			}
			case 's': case 'S':
			{
				smsd_command.command = 0x0E;
				smsd_command.data    = 0;

				break;
			}
			default:
			{
				return;
			}
			}

			Command_MOVE command_move;

			memcpy(command_move.data, &smsd_command, sizeof(smsd_command));

			const auto length = sizeof(Command_MOVE);

			char buffer[length];

			memcpy(buffer, &command_move, length);

			print_data(buffer, length);

			//send_command(command_move, m_sockets[0]);
			//send_command(command_move, m_sockets[1]);
		}

	private:

		void print_data(char * buffer, std::size_t length)
		{
			for (auto i = 0U; i < length; ++i)
			{
				auto c = buffer[i];

				unsigned char mask = 128;

				for (auto j = 0U; j < 8U; ++j) 
				{
					std::cout << ((mask & c) ? "1" : "0");

					mask >>= 1;
				}

				std::cout << std::endl;
			}

			std::cout << std::endl;
		}

		template < typename T >
		void send_command(T & command, socket_t & socket)
		{
			update_control_sum(command);

			boost::asio::write(socket, boost::asio::buffer(&command, sizeof(command)));
		}

		template < typename T >
		void update_control_sum(T & command)
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
	system("chcp 1251");

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
				case 'f': case 'F': case 'b': case 'B': case 's': case 'S':
				{
					controller.move(command, 100);

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