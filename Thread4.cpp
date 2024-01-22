#include <process.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "winmm")

using namespace std;

int Money = 0;
int Money2 = -100;

// 멀티 스레드 동기화를 위해 사용되는 동기화 객체를 정의 및 초기화 합니다.
// 이를 통해 공유 데이터에 안전하게 접근하도록 해줍니다.
// CRITICAL_SECTION 은 "임계 영역" 으로 하나의 스레드만이 진입할 수 있음을 의미합니다.
// 다만 멀티 프로세서의 각 프로세서가 데이터를 공휴하기에 수행될 때 프로세서는 동시 접근이 가능합니다.

// 이 과정에서 대기 중인 스레드가 다른 스레드에게 CPU 점유를 넘기는 경우 Context Switching 이 발생합니다.
// 이렇게 되면 이후 대기시간동안 CPU 자원을 점유하지 않습니다.
// 락을 먼저 선점한 스레드가 있을 경우 그 뒤의 스레드는 락이 풀릴 때 까지 대기해야합니다.
// 이 과정에서 작업 스레드 교환에 대한 컨텍스트 스위칭 비용이 발생하게 됩니다.
// 락이 해제되어 대기중이던 스레드가 깨어나면 다시 한 번 컨텍스트 스위칭이 일어나게 됩니다.
CRITICAL_SECTION MoneySection;
CRITICAL_SECTION MoneySection2;

// Mutex 를 사용하기 위한 핸들입니다.
// 이는 "상호 배제" 의 줄임말 입니다. 스레드에서 어떠한 데이터를 사용하고 있는 동안 다른 스레드는
// 해당 데이터를 못 건드리게 하며, 만약 건드리려면 현재 스레드가 이 데이터를 다 사용할 때까지 기다리게 합니다.
// 이는 주로 하나의 프로스세 내에서 여러 스레드를 사용할 때 사용합니다.
HANDLE MoneyMutex;

//race condition
unsigned Increase(void* Arg)
{
	//EnterCriticalSection(&MoneySection);

	// 뮤텍스를 사용해 MoneyMutex 에 대한 락을 획득합니다.
	// 다른 스레드가 해당 뮤텍스를 획득할 때까지 무한정 대기하게 됩니다.
	WaitForSingleObject(MoneyMutex, INFINITE);

	for (int i = 0; i < 1000000; ++i)
	{
		Money = Money + 1;
		Money2++;

		// 다른 스레드에게 CPU 를 양보하고, 그 스케줄러가 현재 스레드의 실행을 일시 중단하고
		// 다른 스레드에게 CPU 를 양보하도록 합니다.
		Sleep(0);
	}
	//LeaveCriticalSection(&MoneySection);

	// 뮤텍스의 락을 해제합니다. 이로써 다른 스레드가 해당 뮤텍스의 획득이 가능합니다.
	ReleaseMutex(MoneyMutex);

	return 0;
}

// 위와 아래의 방식을 통해서 Money 와 Money2 에 대한 동시 접근을 제어하고,
// 각각의 스레드가 안전하게 값을 증가 또는 감소시킬 수 있도록 합니다.
unsigned Decrease(void* Arg)
{
	EnterCriticalSection(&MoneySection);
	//WaitForSingleObject(MoneyMutex, INFINITE);

	for (int i = 0; i < 1000000; ++i)
	{
		Money2--;
		Money = Money - 1;
		Sleep(0);
	}
	LeaveCriticalSection(&MoneySection);
	//ReleaseMutex(MoneyMutex);

	return 0;
}


/*
// 이 방식은 뮤텍스와 유사한 역할을 합니다.
// 이를 통해 공유 자원에 대한 동시 접근을 제어하고, 스레드 간에 안전하게 공유 자원을 사용하게 합니다.
unsigned Decrease(void* Arg)
{
	// MoneySection 임계 영역에 대한 락을 획득합니다.
	// 다른 스레드에서 접근할 경우 한 번에 하나의 스레드만이 접근이 가능합니다.
	// 즉, 나머지 스레드들은 이 시점에서 대기 상태로 들어가게 됩니다.
	EnterCriticalSection(&MoneySection);

	for (int i = 0; i < 1000000; ++i)
	{
		Money2--;
		Money = Money - 1;
		Sleep(0);
	}

	// MoneySection 임계 영역의 락을 해제합니다.
	// 이렇게 되면 다른 스레드들이 해당 임계 영역에 진입할 수 있게 됩니다.
	LeaveCriticalSection(&MoneySection);

	return 0;
}
*/


//main thread
int main()
{
	//Thread Pooling, IOCP
	//Kernel Mode Lock object
	//UserMode Lock Object

	// 사용을 위해 임계 영역 초기화 작업
	InitializeCriticalSection(&MoneySection);
	InitializeCriticalSection(&MoneySection2);

	// 이는 Double World 의 약자로, 32비트 부호 없는 정수형 데이터 타입입니다.
	// StratTime 에(밀리 초 단위) timeGetTime, 현재 시스템 시간을 저장합니다.
	DWORD StartTime = timeGetTime();

	// Mutex 생성
	// 첫 번째 인자 : 보안속성 설정으로 nullptr 은 사용 하지 않는것을 의미합니다.
	// 두 번째 인자 : 초기에 락이 되지 않은 상태로 생성합니다.(다른 스레드가 락을 해제하려면 락이 해제될 때까지 대기해야 합니다)
	// 세 번째 인자 : 이는 Mutex 의 이름으로 여기서는 미지정이기에 기본값을 사용합니다.
	MoneyMutex = CreateMutex(nullptr, FALSE, nullptr);
	HANDLE ThreadHandle[2];		// HANDLE 배열

	ThreadHandle[0] = (HANDLE)_beginthreadex(nullptr,
		0,
		Increase,
		nullptr,
		CREATE_SUSPENDED,
		nullptr);

	ThreadHandle[1] = (HANDLE)_beginthreadex(nullptr,
		0,
		Decrease,
		nullptr,
		CREATE_SUSPENDED,
		nullptr);

	// 첫 번째 스레드와 두 번째 스레드의 실행을 재개합니다.
	ResumeThread(ThreadHandle[0]);
	ResumeThread(ThreadHandle[1]);

	// 첫 번째 스레드와 두 번째 스레드의 실행을 일시 중지시킵니다.
	// 코드 작동의 결과를 보기 위해서는 주석 처리를 합니다.
	//SuspendThread(ThreadHandle[0]);
	//SuspendThread(ThreadHandle[1]);

	//Sleep(5000);

	// 두 개의 스레드가 종료 될 때 까지 기다립니다.
	// 첫 번째 인자 : 대기할 HANDLE 의 갯수 입니다.
	// 두 번째 인자 : 대기할 핸들을 나타내는 배열입니다.
	// 세 번째 인자 : 모든 핸들이 시그널 상태가 될 때까지 대기합니다.
	//				시그널 상태란, 해당 핸들이 종료 또는 지정된 이벤트가 발생했음을 나타내는 상태를 의미합니다.
	// 네 번째 인자 : 두 개의 스레드가 종료될 때 까지 무한정 대기합니다.
	WaitForMultipleObjects(2, ThreadHandle, TRUE, INFINITE);

	std::cout << "ProcessTime : " << timeGetTime() - StartTime << endl;

	// 사용을 다 한 임계 영역을 삭제하는 역할(누수 방지, 메모리 해제)
	DeleteCriticalSection(&MoneySection);
	DeleteCriticalSection(&MoneySection2);

	// 핸들 종료
	CloseHandle(MoneyMutex);

	CloseHandle(ThreadHandle[0]);
	CloseHandle(ThreadHandle[1]);

	std::cout << Money << endl;

	return 0;
}