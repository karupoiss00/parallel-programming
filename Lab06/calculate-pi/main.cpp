#include <iostream>
#include <functional>
#include <chrono>

#include <omp.h>

using namespace std;

using GetEllapsedTimeFn = function<chrono::duration<double>()>;

constexpr unsigned ITERATIONS_COUNT = 1000000000;

GetEllapsedTimeFn StartTimer();
double syncCalculatePi(int iterations);
double parallelForCalculatePi(int iterations);
double atomicCalculatePi(int iterations);
double reductionCalculatePi(int iterations);

int main()
{
    double syncPi = syncCalculatePi(ITERATIONS_COUNT);
    cout << syncPi << endl;

    double parallelPi = parallelForCalculatePi(ITERATIONS_COUNT);
    cout << parallelPi << endl;

    double atomicPi = atomicCalculatePi(ITERATIONS_COUNT / 10);
    cout << atomicPi << endl;

    double reductionPi = reductionCalculatePi(ITERATIONS_COUNT);
    cout << reductionPi << endl;

    return 0;
}


GetEllapsedTimeFn StartTimer()
{
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    return [start]()
    {
        return chrono::steady_clock::now() - start;
    };
}

double syncCalculatePi(int iterations)
{
    auto getEllapsedTime = StartTimer();

    double result = 0.0;
    for (int i = 0; i < iterations; ++i)
    {
        result += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1);
    }
    result *= 4;

    cout << "syncCalculatePi time: " << getEllapsedTime().count() << endl;

    return result;
}

double parallelForCalculatePi(int iterations)
{
    auto getEllapsedTime = StartTimer();

    double result = 0.0;
    #pragma omp parallel for
    for (int i = 0; i < iterations; ++i)
    {
        result += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1);
    }
    result *= 4;

    cout << "parallelForCalculatePi time: " << getEllapsedTime().count() << endl;

    return result;
}

double atomicCalculatePi(int iterations) 
{
    auto getEllapsedTime = StartTimer();

    double result = 0.0;
    #pragma omp parallel for
    for (int i = 0; i < iterations; ++i)
    {
        double term = (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1);
        #pragma omp atomic
        result += term;
    }
    result *= 4;

    cout << "atomicCalculatePi time: " << getEllapsedTime().count() * 10 << endl;

    return result;
}

double reductionCalculatePi(int iterations)
{
    auto getEllapsedTime = StartTimer();

    double result = 0.0;
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < iterations; ++i)
    {
        result += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1);
    }
    result *= 4;

    cout << "reductionCalculatePi time: " << getEllapsedTime().count() << endl;

    return result;
}
