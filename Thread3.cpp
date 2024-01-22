#include <process.h>
#include <Windows.h>	// HANDLE 사용을 위해서 추가
#include <iostream>

#pragma comment(lib, "winmm")

using namespace std;

int Money = 0;
int Money2 = -100;

// 임계 영역
CRITICAL_SECTION MoneySection;
CRITICAL_SECTION MoneySection2;

HANDLE MoneyMutex;

// ↓↓↓ Thread 방식으로 함수 작성하는 방법 ↓↓↓
// unsigned(__stdcall* _beginthreadex_proc_type)(void*); <- 기본 양식
// 아래처럼 쓰는 것은 윈도우의 양식입니다.

// race condition
// 같은 메모리 영역을 동시에 건드려서 발생하는 문제(원자성)
//unsigned __stdcall Increase(void* Arg)
unsigned Increase(void* Arg)
{
	// Money = Money + 1; 이 작업이 끝나기 전 까지 이 메모리 영역은 못건드리게 합니다.
	for (int i = 0; i < 1000000; i++)
	{
		//EnterCriticalSection(&MoneySection);	// 문 사용
		//EnterCriticalSection(&MoneySection2);
		WaitForSingleObject(MoneyMutex, INFINITE);
		Money = Money + 1;
		Money2++;
		//Sleep(0);	// 이렇게 하면 Conetxt Switching 이 발생합니다.
		ReleaseMutex(MoneyMutex);
		//LeaveCriticalSection(&MoneySection);	// 문 닫기
		//LeaveCriticalSection(&MoneySection2);
	}

	return 0;
}

//unsigned __stdcall Decrease(void* Arg)
unsigned Decrease(void* Arg)
{
	// Money = Money - 1; 이 작업이 끝나기 전 까지 이 메모리 영역은 못건드리게 합니다.
	for (int i = 0; i < 1000000; i++)
	{
		//EnterCriticalSection(&MoneySection2);	// 문 사용
		//EnterCriticalSection(&MoneySection);
		WaitForSingleObject(MoneyMutex, INFINITE);
		Money2--;
		Money = Money - 1;
		ReleaseMutex(MoneyMutex);
		//LeaveCriticalSection(&MoneySection);
		//LeaveCriticalSection(&MoneySection2);	// 문 닫기
	}

	return 0;
}

// Main Thread
int main()
{
	// Thread Polling
	// Kernel Mode Lock object
	// UserMode Lock Object
	// 나만의 메모리 영역에 할당한 문 이라고 생각하면 됩니다.(초기화 작업)
	InitializeCriticalSection(&MoneySection);
	InitializeCriticalSection(&MoneySection2);

	DWORD StartTime = timeGetTime();
	MoneyMutex = CreateMutex(nullptr, FALSE, nullptr);
	HANDLE ThreadHandle[2];

	// 미사용, 최대용량, ... ,initflag 는 0은 바로 실행
	//HANDLE ThreadHandle1 = 
	//	(HANDLE)_beginthreadex(nullptr, 0, Increase, nullptr, 0, nullptr);

	//HANDLE ThreadHandle2 =
	//	(HANDLE)_beginthreadex(nullptr, 0, Decrease, nullptr, 0, nullptr

	ThreadHandle[0] =
		(HANDLE)_beginthreadex(nullptr, 0, Increase, nullptr, 0, nullptr);

	ThreadHandle[1] =
		(HANDLE)_beginthreadex(nullptr, 0, Decrease, nullptr, 0, nullptr);

	ResumeThread(ThreadHandle[0]);
	ResumeThread(ThreadHandle[1]);

	SuspendThread(ThreadHandle[0]);
	SuspendThread(ThreadHandle[1]);

	//Sleep(1000);	// 1초
	WaitForMultipleObjects(2, ThreadHandle, TRUE, INFINITE);

	cout << "ProcessTime : " << timeGetTime() - StartTime << endl;
	
	DeleteCriticalSection(&MoneySection);
	DeleteCriticalSection(&MoneySection2);

	CloseHandle(MoneyMutex);
	CloseHandle(ThreadHandle[0]);
	CloseHandle(ThreadHandle[1]);

	cout << Money << endl;

	return 0;
}