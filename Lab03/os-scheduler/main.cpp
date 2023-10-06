#include <iostream>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <optional>
#include <fstream>
#include <functional>
#include <chrono>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

constexpr size_t THREADS_COUNT = 2;

struct ThreadParams
{
    int number;
    int opsCount;
};

DWORD WINAPI ThreadProc(CONST LPVOID lpParam);

DWORD START_TIME = 0;


int _tmain(int argc, _TCHAR* argv[])
{
    unsigned opsCount;

    cin >> opsCount;

    HANDLE* handles = new HANDLE[THREADS_COUNT];

    START_TIME = timeGetTime();

    for (int i = 0; i < THREADS_COUNT; i++)
    {
        ThreadParams* params = new ThreadParams;
        params->number = i + 1;
        params->opsCount = opsCount;
        handles[i] = CreateThread(NULL, 0, &ThreadProc, params, CREATE_SUSPENDED, NULL);
    }

    for (int i = 0; i < THREADS_COUNT; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(THREADS_COUNT, handles, true, INFINITE);

    delete[] handles;
    return 0;
}

void PrimeNumbersSearch(int N)
{
    bool* primesArr = new bool[N + 1];
    int i, j;
    for (i = 1; i <= N; i++)
    {
        primesArr[i] = true;
    }
    i = 1; j = 1;
    while ((2 * i * j + i + j) <= N)
    {
        while (j <= (N - i) / (2 * i + 1))
        {
            primesArr[2 * i * j + i + j] = false;
            j++;
        }
        i++;
        j = i;
    }
    for (i = 1; i <= N; i++)
    {
        if (primesArr[i]) cout << 2 * i + 1 << " ";
    }
    delete[] primesArr;
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    ThreadParams* param = reinterpret_cast<ThreadParams*>(lpParam);
    ofstream output;
    output.open("thread" + to_string(param->number) + ".txt");

    for (size_t i = 0; i < param->opsCount; i++)
    {
        PrimeNumbersSearch(1000);
        float ellapsedTime = (float(timeGetTime()) - float(START_TIME)) / 1000.0f;
        output << ellapsedTime << endl;
    }

    output.close();
   
    delete param;
    ExitThread(0);
}