#include <process.h>
#include <Windows.h>	// HANDLE ����� ���ؼ� �߰�
#include <iostream>

#pragma comment(lib, "winmm")l

using namespace std;

int Money = 0;
int Money2 = -100;

// �Ӱ� ����
CRITICAL_SECTION MoneySection;
CRITICAL_SECTION MoneySection2;

HANDLE MoneyMutex;

// ���� Thread ������� �Լ� �ۼ��ϴ� ��� ����
// unsigned(__stdcall* _beginthreadex_proc_type)(void*); <- �⺻ ���
// �Ʒ�ó�� ���� ���� �������� ����Դϴ�.

// race condition
// ���� �޸� ������ ���ÿ� �ǵ���� �߻��ϴ� ����(���ڼ�)
unsigned __stdcall Increase(void* Arg)
{
	// Money = Money + 1; �� �۾��� ������ �� ���� �� �޸� ������ ���ǵ帮�� �մϴ�.
	for (int i = 0; i < 1000000; i++)
	{
		//EnterCriticalSection(&MoneySection);	// �� ���
		//EnterCriticalSection(&MoneySection2);
		WaitForSingleObject(MoneyMutex, INFINITE);
		Money = Money + 1;
		Money2++;
		//Sleep(0);	// �̷��� �ϸ� Conetxt Switching �� �߻��մϴ�.
		ReleaseMutex(MoneyMutex);
		//LeaveCriticalSection(&MoneySection);	// �� �ݱ�
		//LeaveCriticalSection(&MoneySection2);
	}

	return 0;
}

unsigned __stdcall Decrease(void* Arg)
{
	// Money = Money - 1; �� �۾��� ������ �� ���� �� �޸� ������ ���ǵ帮�� �մϴ�.
	for (int i = 0; i < 1000000; i++)
	{
		//EnterCriticalSection(&MoneySection2);	// �� ���
		//EnterCriticalSection(&MoneySection);
		WaitForSingleObject(MoneyMutex, INFINITE);
		Money2--;
		Money = Money - 1;
		ReleaseMutex(MoneyMutex);
		//LeaveCriticalSection(&MoneySection);
		//LeaveCriticalSection(&MoneySection2);	// �� �ݱ�
	}

	return 0;
}

// Main Thread
int main()
{
	// Thread Polling
	// 
	// 
	// ������ �޸� ������ �Ҵ��� �� �̶�� �����ϸ� �˴ϴ�.(�ʱ�ȭ �۾�)
	InitializeCriticalSection(&MoneySection);
	InitializeCriticalSection(&MoneySection2);

	DWORD StartTime = timeGetTime();
	MoneyMutex = CreateMutex(nullptr, FALSE, nullptr);
	HANDLE ThreadHandle[2];

	// �̻��, �ִ�뷮, ... ,initflag �� 0�� �ٷ� ����
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

	//Sleep(1000);	// 1��
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