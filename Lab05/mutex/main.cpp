#include <windows.h>
#include <string>
#include <iostream>
#include <functional>
#include <fstream>

#include "tchar.h"

using namespace std;

constexpr size_t THREADS_COUNT = 50;

HANDLE FileLockingMutex;
function<void()> fnToRunInMutex;

void MutexActionRun() {
	fnToRunInMutex();
}

int DoInsideMutex() {
	DWORD dwCount = 0;
	WaitForSingleObject(FileLockingMutex, INFINITE);

	fnToRunInMutex();

	ReleaseMutex(FileLockingMutex);
	return TRUE;
}

int ReadFromFile() {
	int result;

	std::fstream myfile("balance.txt", std::ios_base::in);
	myfile >> result;
	myfile.close();

	return result;
}

void WriteToFile(int data) {
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
}

int GetBalance() {
	int balance = ReadFromFile();
	return balance;
}

void Deposit(int money) {
	cout << money << endl;
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money) {
	if (GetBalance() < money) {
		printf("Cannot withdraw money, balance lower than %d\n", money);
		return;
	}

	Sleep(20);
	int balance = GetBalance();

	balance -= money;
	WriteToFile(balance);
	printf("Balance after withdraw: %d\n", balance);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	fnToRunInMutex = [&lpParameter]() {
		Deposit((int)lpParameter);
	};
	return DoInsideMutex();
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	fnToRunInMutex = [&lpParameter]() {
		Withdraw((int)lpParameter);
	};
	return DoInsideMutex();
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE* handles = new HANDLE[THREADS_COUNT];

	FileLockingMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		L"file_locking_mutex");             // unnamed mutex

	if (FileLockingMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for (int i = 0; i < THREADS_COUNT; i++) {
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}


	// ожидание окончания работы двух потоков
	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	CloseHandle(FileLockingMutex);

	return 0;
}