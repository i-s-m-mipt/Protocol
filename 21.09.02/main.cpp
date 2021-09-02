#include <iostream>
#include <boost/asio.hpp>

void write_data(boost::asio::ip::tcp::socket & socket, std::string comand)
{
	boost::asio::write(socket, boost::asio::buffer(comand));
}

struct LAN_COMMAND_0
{
	uint8_t XOR_SUM = 0x00;
	uint8_t Ver = 0x02;
	uint8_t CMD_TYPE = 0x00;
	uint8_t CMD_IDENTIFICATION = 0x00;
	uint16_t LENGTH_DATA = 0x00;
	uint8_t DATA[0];
};

uint8_t xor_sum(uint8_t * data, uint16_t length)
{
	uint8_t xor_temp = 0xFF;
	while (length--) { xor_temp += *data; data++; }
	return(xor_temp ^ 0xFF);
}


int main()
{
	LAN_COMMAND_0 lan_command;

	lan_command.XOR_SUM = xor_sum((uint8_t *)&lan_command.XOR_SUM, sizeof(lan_command));

	std::string raw_ip_address_1 = "192.168.1.2";
	std::string raw_ip_address_2 = "192.168.1.3";
	auto port = 5000;

	boost::asio::ip::tcp::endpoint endpoint_1(boost::asio::ip::address::from_string(raw_ip_address_1), port);
	boost::asio::ip::tcp::endpoint endpoint_2(boost::asio::ip::address::from_string(raw_ip_address_2), port);

	boost::asio::io_service io_service;

	boost::asio::ip::tcp::socket socket_1(io_service, endpoint_1.protocol());
	socket_1.connect(endpoint_1);

	std::cout << "192.168.1.2 connected" << std::endl;

	boost::asio::ip::tcp::socket socket_2(io_service, endpoint_2.protocol());
	socket_2.connect(endpoint_2);

	std::cout << "192.168.1.3 connected" << std::endl;

	//boost::asio::write(socket_1, boost::asio::buffer(&lan_command, sizeof(lan_command)));

	//boost::asio::write(socket_2, boost::asio::buffer(&lan_command, sizeof(lan_command)));

	//std::cout << "lan_command_0 write" << std::endl;

	/*const std::size_t size = 30;

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), port);
	boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint.protocol());
	acceptor.bind(endpoint);
	acceptor.listen(size);
	boost::asio::ip::tcp::socket socket(io_service);
	acceptor.accept(socket);*/

	int a = 0;

	while (true)
	{
		std::cin >> a;

		if (a == 0)
		{
			std::string comand = ""; //остановка

		}

		if (a == 1)
		{
			std::string comand = ""; //крутить вправо
			write_data(socket_1, comand);
		}

		if (a == 2)
		{
			std::string comand = ""; //крутить влево
			write_data(socket_1, comand);
		}
	}
	system("pause");
	return 0;
}