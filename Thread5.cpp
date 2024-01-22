#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	//ListenSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(10880);

	bind(ListenSocket, (struct sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	fd_set ReadSocketList;
	fd_set CopyReadSocketList;

	FD_ZERO(&ReadSocketList);
	FD_SET(ListenSocket, &ReadSocketList);

	struct timeval TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 10;

	while (true)
	{
		CopyReadSocketList = ReadSocketList;
		int SocketEventCount = select(0, &CopyReadSocketList, 0, 0, &TimeOut);

		if (SocketEventCount == 0)
		{
			//other process
			continue;
		}
		else if (SocketEventCount < 0)
		{
			//error
		}

		for (int i = 0; i < (int)ReadSocketList.fd_count; ++i)
		{
			if (FD_ISSET(ReadSocketList.fd_array[i], &CopyReadSocketList))
			{
				if (ReadSocketList.fd_array[i] == ListenSocket)
				{
					//connect
					struct sockaddr_in ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockAddrSize = sizeof(ClientSockAddr);
					SOCKET ClientSocket = accept(ListenSocket, (struct sockaddr*)&ClientSockAddr, &ClientSockAddrSize);
					FD_SET(ClientSocket, &ReadSocketList);
				}
				else
				{
					//process data
					char Buffer[1024] = { 0, };
					int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, 1024, 0);
					if (RecvLength <= 0)
					{
						//정상, 비정상 종료
						closesocket(ReadSocketList.fd_array[i]);
						FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
					}
					else
					{
						int SendLength = send(ReadSocketList.fd_array[i], Buffer, RecvLength, 0);
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();
	return 0;
}