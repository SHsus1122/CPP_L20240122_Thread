#include <process.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "winmm")

using namespace std;

int Money = 0;
int Money2 = -100;

// ��Ƽ ������ ����ȭ�� ���� ���Ǵ� ����ȭ ��ü�� ���� �� �ʱ�ȭ �մϴ�.
// �̸� ���� ���� �����Ϳ� �����ϰ� �����ϵ��� ���ݴϴ�.
// CRITICAL_SECTION �� "�Ӱ� ����" ���� �ϳ��� �����常�� ������ �� ������ �ǹ��մϴ�.
// �ٸ� ��Ƽ ���μ����� �� ���μ����� �����͸� �����ϱ⿡ ����� �� ���μ����� ���� ������ �����մϴ�.

// �� �������� ��� ���� �����尡 �ٸ� �����忡�� CPU ������ �ѱ�� ��� Context Switching �� �߻��մϴ�.
// �̷��� �Ǹ� ���� ���ð����� CPU �ڿ��� �������� �ʽ��ϴ�.
// ���� ���� ������ �����尡 ���� ��� �� ���� ������� ���� Ǯ�� �� ���� ����ؾ��մϴ�.
// �� �������� �۾� ������ ��ȯ�� ���� ���ؽ�Ʈ ����Ī ����� �߻��ϰ� �˴ϴ�.
// ���� �����Ǿ� ������̴� �����尡 ����� �ٽ� �� �� ���ؽ�Ʈ ����Ī�� �Ͼ�� �˴ϴ�.
CRITICAL_SECTION MoneySection;
CRITICAL_SECTION MoneySection2;

// Mutex �� ����ϱ� ���� �ڵ��Դϴ�.
// �̴� "��ȣ ����" �� ���Ӹ� �Դϴ�. �����忡�� ��� �����͸� ����ϰ� �ִ� ���� �ٸ� �������
// �ش� �����͸� �� �ǵ帮�� �ϸ�, ���� �ǵ帮���� ���� �����尡 �� �����͸� �� ����� ������ ��ٸ��� �մϴ�.
// �̴� �ַ� �ϳ��� ���ν��� ������ ���� �����带 ����� �� ����մϴ�.
HANDLE MoneyMutex;

//race condition
unsigned Increase(void* Arg)
{
	//EnterCriticalSection(&MoneySection);

	// ���ؽ��� ����� MoneyMutex �� ���� ���� ȹ���մϴ�.
	// �ٸ� �����尡 �ش� ���ؽ��� ȹ���� ������ ������ ����ϰ� �˴ϴ�.
	WaitForSingleObject(MoneyMutex, INFINITE);

	for (int i = 0; i < 1000000; ++i)
	{
		Money = Money + 1;
		Money2++;

		// �ٸ� �����忡�� CPU �� �纸�ϰ�, �� �����ٷ��� ���� �������� ������ �Ͻ� �ߴ��ϰ�
		// �ٸ� �����忡�� CPU �� �纸�ϵ��� �մϴ�.
		Sleep(0);
	}
	//LeaveCriticalSection(&MoneySection);

	// ���ؽ��� ���� �����մϴ�. �̷ν� �ٸ� �����尡 �ش� ���ؽ��� ȹ���� �����մϴ�.
	ReleaseMutex(MoneyMutex);

	return 0;
}

// ���� �Ʒ��� ����� ���ؼ� Money �� Money2 �� ���� ���� ������ �����ϰ�,
// ������ �����尡 �����ϰ� ���� ���� �Ǵ� ���ҽ�ų �� �ֵ��� �մϴ�.
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
// �� ����� ���ؽ��� ������ ������ �մϴ�.
// �̸� ���� ���� �ڿ��� ���� ���� ������ �����ϰ�, ������ ���� �����ϰ� ���� �ڿ��� ����ϰ� �մϴ�.
unsigned Decrease(void* Arg)
{
	// MoneySection �Ӱ� ������ ���� ���� ȹ���մϴ�.
	// �ٸ� �����忡�� ������ ��� �� ���� �ϳ��� �����常�� ������ �����մϴ�.
	// ��, ������ ��������� �� �������� ��� ���·� ���� �˴ϴ�.
	EnterCriticalSection(&MoneySection);

	for (int i = 0; i < 1000000; ++i)
	{
		Money2--;
		Money = Money - 1;
		Sleep(0);
	}

	// MoneySection �Ӱ� ������ ���� �����մϴ�.
	// �̷��� �Ǹ� �ٸ� ��������� �ش� �Ӱ� ������ ������ �� �ְ� �˴ϴ�.
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

	// ����� ���� �Ӱ� ���� �ʱ�ȭ �۾�
	InitializeCriticalSection(&MoneySection);
	InitializeCriticalSection(&MoneySection2);

	// �̴� Double World �� ���ڷ�, 32��Ʈ ��ȣ ���� ������ ������ Ÿ���Դϴ�.
	// StratTime ��(�и� �� ����) timeGetTime, ���� �ý��� �ð��� �����մϴ�.
	DWORD StartTime = timeGetTime();

	// Mutex ����
	// ù ��° ���� : ���ȼӼ� �������� nullptr �� ��� ���� �ʴ°��� �ǹ��մϴ�.
	// �� ��° ���� : �ʱ⿡ ���� ���� ���� ���·� �����մϴ�.(�ٸ� �����尡 ���� �����Ϸ��� ���� ������ ������ ����ؾ� �մϴ�)
	// �� ��° ���� : �̴� Mutex �� �̸����� ���⼭�� �������̱⿡ �⺻���� ����մϴ�.
	MoneyMutex = CreateMutex(nullptr, FALSE, nullptr);
	HANDLE ThreadHandle[2];		// HANDLE �迭

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

	// ù ��° ������� �� ��° �������� ������ �簳�մϴ�.
	ResumeThread(ThreadHandle[0]);
	ResumeThread(ThreadHandle[1]);

	// ù ��° ������� �� ��° �������� ������ �Ͻ� ������ŵ�ϴ�.
	// �ڵ� �۵��� ����� ���� ���ؼ��� �ּ� ó���� �մϴ�.
	//SuspendThread(ThreadHandle[0]);
	//SuspendThread(ThreadHandle[1]);

	//Sleep(5000);

	// �� ���� �����尡 ���� �� �� ���� ��ٸ��ϴ�.
	// ù ��° ���� : ����� HANDLE �� ���� �Դϴ�.
	// �� ��° ���� : ����� �ڵ��� ��Ÿ���� �迭�Դϴ�.
	// �� ��° ���� : ��� �ڵ��� �ñ׳� ���°� �� ������ ����մϴ�.
	//				�ñ׳� ���¶�, �ش� �ڵ��� ���� �Ǵ� ������ �̺�Ʈ�� �߻������� ��Ÿ���� ���¸� �ǹ��մϴ�.
	// �� ��° ���� : �� ���� �����尡 ����� �� ���� ������ ����մϴ�.
	WaitForMultipleObjects(2, ThreadHandle, TRUE, INFINITE);

	std::cout << "ProcessTime : " << timeGetTime() - StartTime << endl;

	// ����� �� �� �Ӱ� ������ �����ϴ� ����(���� ����, �޸� ����)
	DeleteCriticalSection(&MoneySection);
	DeleteCriticalSection(&MoneySection2);

	// �ڵ� ����
	CloseHandle(MoneyMutex);

	CloseHandle(ThreadHandle[0]);
	CloseHandle(ThreadHandle[1]);

	std::cout << Money << endl;

	return 0;
}