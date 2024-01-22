#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
	// 이전 시간에 계속 사용했던 코드이기에 별 다른 설명은 없습니다.
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));

	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("192.168.3.118");
	ServerSockAddr.sin_port = htons(10880);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	while (true)
	{
		const char Buffer1[] =
			"\n"
			"⊂_＼\n"
			"  ＼＼Λ＿Λ\n"
			"    ＼('ㅅ') 두둠칫\n"
			"      > ⌒＼\n"
			"      /   へ＼\n"
			"     /　 /  ＼＼\n"
			"    /　ノ     ＼_つ\n"
			"   /　/두둠칫\n"
			"  /　/|\n"
			" (  ( ＼\n"
			" |　| 、＼\n"
			" | 丿  ＼⌒)\n"
			" | |   ) /\n"
			" `ノ ) L |\n";
		send(ServerSocket, Buffer1, (u_int)sizeof(Buffer1), 0);

		const char Buffer2[] =
			"  \n"
			"  ⊂_＼\n"
			"    ＼＼Λ＿Λ\n"
			"      ＼('ㅅ') 두둠칫\n"
			"        > ⌒＼\n"
			"        /   へ＼\n"
			"       /　 /  ＼＼\n"
			"      /　ノ     ＼_つ\n"
			"     /　/두둠칫\n"
			"    /　/|\n"
			"   (  ( ＼\n"
			"   |　| 、＼\n"
			"   | 丿  ＼⌒)\n"
			"   | |   ) /\n"
			"   `ノ ) L |\n";
		send(ServerSocket, Buffer2, (u_int)sizeof(Buffer2), 0);

		char Message[1024] = { 0, };
		recv(ServerSocket, Message, 1024, 0);

		cout << Message << endl;
	}

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}