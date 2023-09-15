#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <numeric>
#include <functional>

#include <tchar.h>
#include <Windows.h>
#include <wingdi.h>

#include "BitmapPlusPlus.h"

using namespace std;
using namespace bmp;

struct Args
{
    string inputFileName;
    string outputFileName;
    int threadsCount;
    int coresCount;
};

struct ProcessBitmapInfo
{
    Bitmap* image;
    unsigned lineNumber;
    size_t lineHeight;
};

optional<Args> ParseArgs(int argc, _TCHAR* argv[]);
DWORD WINAPI BlurBitmap(CONST LPVOID lpParam);
unique_ptr<HANDLE[]> CreateThreads(size_t count, function<ProcessBitmapInfo*(int)> const& dataCreatorFn);

int _tmain(int argc, _TCHAR* argv[])
{
    auto args = ParseArgs(argc, argv);

    if (!args)
    {
        cout << "Params format: <input file name> <output file name> <cores count> <threads count>" << endl;
        return -1;
    }

    try
    {
        Bitmap* image = new Bitmap(args->inputFileName);
        unsigned lineHeight = image->height() / args->threadsCount;

        auto threads = CreateThreads(args->threadsCount, [lineHeight, image](unsigned threadNumber) {
            return new ProcessBitmapInfo{
                image,
                threadNumber,
                lineHeight,
            };
        });
       
        for (int i = 0; i < args->threadsCount; i++)
        {
            ResumeThread(threads[i]);
        }

        WaitForMultipleObjects(args->threadsCount, threads.get(), true, INFINITE);

        image->save(args->outputFileName);
    }
    catch (const bmp::Exception& e)
    {
        std::cerr << "[BMP ERROR]: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}

optional<Args> ParseArgs(int argc, _TCHAR* argv[])
{
    if (argc < 5)
    {
        return nullopt;
    }

    Args result;

    result.inputFileName = argv[1];
    result.outputFileName = argv[2];
    
    try
    {
        result.coresCount = stoi(argv[3]);
        result.threadsCount = stoi(argv[4]);
    }
    catch (...)
    {
        return nullopt;
    }

    return result;
}


unique_ptr<HANDLE[]> CreateThreads(size_t count, function<ProcessBitmapInfo* (int)> const& dataCreatorFn)
{
    auto threads = make_unique<HANDLE[]>(count);

    for (unsigned i = 0; i < count; i++)
    {
        threads[i] = CreateThread(
            NULL, 0, &BlurBitmap, dataCreatorFn(i), CREATE_SUSPENDED, NULL
        );
    }

    return threads;
}

Pixel Average(std::vector<optional<Pixel>> const& v) 
{
    auto const count = static_cast<int>(v.size());
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    int pixelsCount = 0;

    for (auto const& pixel : v) 
    {
        if (!pixel)
        {
            continue;
        }
        pixelsCount++;
        sumR += pixel->r;
        sumG += pixel->g;
        sumB += pixel->b;
    }

    return Pixel(sumR / pixelsCount, sumG / pixelsCount, sumB / pixelsCount);
}



Pixel GetAverageColor(Bitmap const& img, int x, int y)
{
    vector<optional<Pixel>> pixels = {
        img.get(x, y),
        img.get(x, y - 1),
        img.get(x, y + 1),
        img.get(x - 1, y),
        img.get(x - 1, y - 1),
        img.get(x - 1, y + 1),
        img.get(x + 1, y),
        img.get(x + 1, y - 1),
        img.get(x + 1, y + 1),
    };
   
    return Average(pixels);
}

DWORD WINAPI BlurBitmap(CONST LPVOID lpParam)
{
    auto data = reinterpret_cast<ProcessBitmapInfo*>(lpParam);
    
    int startY = data->lineNumber * data->lineHeight;

    auto image = data->image;
    for (std::int32_t y = startY; y < startY + data->lineHeight; ++y)
    {
        for (std::int32_t x = 0; x < image->width(); ++x)
        {
            image->set(x, y, GetAverageColor(*image, x, y));
        }
    }

    delete data;
    ExitThread(0);
}
