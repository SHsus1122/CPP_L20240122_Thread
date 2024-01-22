#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>

using namespace std;

#pragma comment(lib,"ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));

	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(10880);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	// fd_set : 소켓 집합(구조체)를 나타내는 자료형입니다.
	//  - ReadSocketList : 서버에 요청을 보낸 소켓 입출력 관련 이벤트를 모니터링할 소켓 집합입니다.
	fd_set ReadSocketList;
	FD_ZERO(&ReadSocketList);	// 초기화 작업

	// FD_SET : 이는 매크로로, 특정 소켓을 소켓 집합에 추가흐는 역할을 합니다.
	//  - 첫 번째 인자 : "fd", 확인할 소켓의 파일 디스크립터
	//  - 두 번째 인자 : "set", 확인할 fd_set 만약 소켓이 set 에 속해 있지 않다면 0 을 반환
	// ListenSocket 이라는 소켓을 ReadSocketList 에 추가해 해당 소켓에서 발생하는 이벤트를
	// 모니티렁 한다는 의미입니다.
	FD_SET(ListenSocket, &ReadSocketList);

	struct timeval TimeOut;		// 얼마나 기다릴지에 대한 시간을 정의한 구조체
	TimeOut.tv_sec = 0;			  // 초 단위
	TimeOut.tv_usec = 100;		// 마이크로 초 단위(100 은 0.1초)
	// select 함수가 여기서 설정한 시간동안 대기 후 반환됩니다.

	// 복사 한 소켓 리스트를 담기 위한 구조체 선언 및 초기화
	fd_set CopySocketLists;
	FD_ZERO(&CopySocketLists);

	// "비동기 소켓 입출력 처리"를 위해 select 함수 사용
	while (true)
	{
		// 아래 select 에서 소켓 리스트를 날려버리기 때문에 원본을 복사해주도록 합니다.
		// 즉, select 함수를 사용하면 원본 소켓 집합의 값을 변경하게 됩니다.
		CopySocketLists = ReadSocketList;

		// 아래와 같은 방법을 polling 이라고 합니다.
		// 이를 위에서 정의한 0.1초마다 계속해서 맞게 왔는지 원본과 비교해서 물어보는 작업을 합니다.
		// 항상 구조체들은 포인터로 전달하도록 합니다.

		// 1. 가장 먼저 이벤트가 발생했는지(최대 기다리는 시간, 0.1초 마다) 물어봅니다.
		//    이 때, 발생한 소켓의 개수가 EventSocketCount 변수에 저장됩니다.
		//  - 첫 번째 인자   : 소켓의 개수 지정으로 윈도우에서는 무시되므로 0 을 사용합니다.
		//  - 두 번째 인자   : 소켓 이벤트를 감지할 소켓 집합 입니다.
		//  - 세 번째 인자   : 소켓의 쓰기 이벤트를 감지할 소켓 집합입니다.
		//  - 네 번째 인자   : 예외 상황을 감지할 소켓 집합 입니다.
		//  - 다섯 번째 인자 : (구조체)지정된 대기시간 입니다. 즉, 여기선 0.1초간 대기합니다.
		int EventSocketCount = select(0, &CopySocketLists, nullptr, nullptr, &TimeOut);
		if (EventSocketCount == 0)
		{
			// no event - 이벤트가 발생하지 않았을 경우의 처리는 여기서
			//cout << "done ?" << endl;
		}
		else if (EventSocketCount < 0)
		{
			// error - 함수 호출의 결과가 오류를 반환했을 경우 처리는 여기서
		}
		else
		{
			// process
			// 2. 이후 만약 이벤트가 발생했다면..
			// 현재 소켓 집합에 포함된 소켓의 갯수만큼 소켓 집합을 반복적으로 순회합니다.
			for (int i = 0; i < (int)ReadSocketList.fd_count; i++)
			{
				// FD_ISSET 를 통해서 ReadSocketList.fd_array[i] 번째의 소켓이
				// CopySocketLists 소켓 집합에서 이벤트가 발생한 소켓인지 확인합니다.
				if (FD_ISSET(ReadSocketList.fd_array[i], &CopySocketLists))
				{
					// 3. 연결 처리에 대한 작업입니다.
					// 이벤트가 발생한 소켓이 ListenSocket 인지 확인해서 맞다면, 서버 소켓은 
		  // 클라이언트의 요청을 기다리게 됩니다.
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						// 이제 검증을 통과한 소켓을 새로이 생성해서 accept 를 클라이언트와 연결을 수락합니다.
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockAddrLength = sizeof(ClientSockAddr);

						SOCKET NewClientSocket = accept(ReadSocketList.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

						// 새로운 클라이언트 소켓을 기존의 소켓 집합에 추가하게 됩니다.
						// 이렇게 해야 새로운 클라이언트 소켓 추가 및 select 함수가 해당 소켓에서 발생하는
						// 이벤트를 감지할 수 있게 됩니다.
						FD_SET(NewClientSocket, &ReadSocketList);
						// 연결된 클라이언트의 IP 를 확인하기 위한 출력문입니다.
						cout << "connect client : " << inet_ntoa(ClientSockAddr.sin_addr) << endl;
					}
					else
					{
						// recv
						// 클라이언트의 데이터를 받아서 빈 배열 Buffer 에 담아줍니다.
						char Buffer[1024] = { 0, };
						int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, 1024, 0);

						if (RecvLength == 0)
						{
							// Disconnected
							cout << "Disconnected client : " << ReadSocketList.fd_array[i] << endl;

							// 해당 순번의 소켓을 ReadSocketList 이 리스트에서 제거 합니다.
							// 이를 통해 select 에서 더 이상 이벤트를 감지하지 않게 해줍니다.
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else if (RecvLength < 0)
						{
							// Disconnected and Lan cut Error
							cout << "Error Disconnected client : " << ReadSocketList.fd_array[i] << endl;
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else
						{
							// 모두 통과한 즉, 정상적인 경우 이제 메시지를 받아서 출력해주도록 합니다.
							cout << "recv client : " << ReadSocketList.fd_array[i] << ", " << Buffer << endl;
							send(ReadSocketList.fd_array[i], Buffer, RecvLength, 0);
						}
					}
				}
			}
		}
	}


	WSACleanup();

	return 0;
}