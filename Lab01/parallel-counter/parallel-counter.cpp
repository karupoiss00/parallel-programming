#include <iostream>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <optional>

using namespace std;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    int* param = reinterpret_cast<int*>(lpParam);
    printf("Thread %d\n", *param);
    delete param;
    ExitThread(0); // функция устанавливает код завершения потока в 0
}

struct Args
{
    int threadsCount;
};

optional<Args> ParseArgs(int argc, _TCHAR* argv[]);

int _tmain(int argc, _TCHAR* argv[])
{
    auto args = ParseArgs(argc, argv);

    if (!args || args->threadsCount <= 0)
    {
        cout << "incorrect params" << endl;
        return -1;
    }

    // создание потоков
    HANDLE* handles = new HANDLE[args->threadsCount];

    for (int i = 0; i < args->threadsCount; i++)
    {
        int* newNumber = new int;
        *newNumber = i + 1;
        handles[i] = CreateThread(NULL, 0, &ThreadProc, newNumber, CREATE_SUSPENDED, NULL);
    }

    // запуск потоков
    for (int i = 0; i < args->threadsCount; i++)
    {
        ResumeThread(handles[i]);
    }

    // ожидание окончания работы потоков
    WaitForMultipleObjects(args->threadsCount, handles, true, INFINITE);

    delete[] handles;
    return 0;
}

optional<Args> ParseArgs(int argc, _TCHAR* argv[])
{
    if (argc < 2)
    {
        return nullopt;
    }

    Args result;

    try
    {
        result.threadsCount = stoi(argv[1]);
    }
    catch (...)
    {
        return nullopt;
    }

    return result;
}