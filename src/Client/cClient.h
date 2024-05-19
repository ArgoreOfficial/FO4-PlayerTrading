#pragma once

#include <WinSock2.h>

#include <string>
#include <vector>

enum ePacketType
{
	PacketType_Connect = 1,
	PacketType_Send = 2,
	PacketType_ConfirmTrade = 3
};

union uAddressCode {
	uint8_t a[ 4 ];
	uint32_t c = 0;

	std::string dump()
	{
		std::string ip; 
		ip += std::to_string( a[ 0 ] );
		ip += "." + std::to_string( a[ 1 ] );
		ip += "." + std::to_string( a[ 2 ] );
		ip += "." + std::to_string( a[ 3 ] );

		return ip;
	}
};

class cClient
{
public:
	cClient();
	~cClient();

	void create( WORD _port );
	void destroy();

	void connect( const char* _ip, WORD _port );
	void handleConnection( SOCKADDR_IN _addr );

	void sendData( const char* _ip, WORD _port, void* _buffer, int _size );
	void sendData( const char* _ip, WORD _port );
	void sendDataToConnected( void* _buffer, int _size );

	std::vector<int> getReceivedData();

	SOCKET makeSocket( WORD _port );
	uAddressCode getExternalIP();

	uAddressCode m_external_ip{};

	SOCKET m_socket;
	HANDLE m_thread;

	ULONG m_connected_addr = 0;
	USHORT m_connected_port = 0;

	bool m_received_data = false;
	int m_data_size = 0;
	char m_recv_buffer[ 1024 ];
	char m_send_buffer[ 1024 ];

};
