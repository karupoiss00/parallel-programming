using System;
using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Threading.Tasks;
using Newtonsoft.Json;

class Program
{
    static async Task Main()
    {
        const string apiUrl = "https://dog.ceo/api/breeds/image/random";
        const int numberOfRequests = 10;

        Console.WriteLine("Асинхронные запросы:");

        Stopwatch asyncStopwatch = Stopwatch.StartNew();
        await MakeAsyncRequests(apiUrl, numberOfRequests);
        asyncStopwatch.Stop();
        Console.WriteLine($"Время выполнения асинхронных запросов: {asyncStopwatch.ElapsedMilliseconds} мс");

        Console.WriteLine("\nСинхронные запросы:");

        Stopwatch syncStopwatch = Stopwatch.StartNew();
        await MakeSyncRequests(apiUrl, numberOfRequests);
        syncStopwatch.Stop();
        Console.WriteLine($"Время выполнения синхронных запросов: {syncStopwatch.ElapsedMilliseconds} мс");
    }

    static async Task MakeAsyncRequests(string apiUrl, int numberOfRequests)
    {
        var httpClient = new HttpClient();

        var tasks = new Task[numberOfRequests];
        for (int i = 0; i < numberOfRequests; i++)
        {
            tasks[i] = FetchAndSaveImageAsync(apiUrl, httpClient, i + 1, "async");
        }

        await Task.WhenAll(tasks);
    }

    static async Task MakeSyncRequests(string apiUrl, int numberOfRequests)
    {
        var httpClient = new HttpClient();

        for (int i = 0; i < numberOfRequests; i++)
        {
            await FetchAndSaveImageAsync(apiUrl, httpClient, i + 1);
        }
    }

    static async Task FetchAndSaveImageAsync(string apiUrl, HttpClient httpClient, int requestNumber, string prefix = "")
    {
        try
        {
            HttpResponseMessage response = await httpClient.GetAsync(apiUrl);
            response.EnsureSuccessStatusCode();

            string jsonContent = await response.Content.ReadAsStringAsync();
            var jsonObject = JsonConvert.DeserializeObject<ApiResponse>(jsonContent);

            if (jsonObject?.Message != null)
            {
                byte[] imageData = await httpClient.GetByteArrayAsync(jsonObject.Message);

                Directory.CreateDirectory("downloads");

                string fileName = $"downloads\\image_{prefix + "_" + requestNumber}.jpg";

                using (FileStream fileStream = new FileStream(fileName, FileMode.Create, FileAccess.Write, FileShare.None, bufferSize: 4096, useAsync: true))
                {
                    await fileStream.WriteAsync(imageData, 0, imageData.Length);
                }

                Console.WriteLine($"Запрос {requestNumber}: Изображение сохранено в {fileName}");
            }
            else
            {
                Console.WriteLine($"Ошибка при выполнении запроса {requestNumber}: Не удалось получить URL изображения.");
            }
        }
        catch (HttpRequestException ex)
        {
            Console.WriteLine($"Ошибка при выполнении запроса {requestNumber}: {ex.Message}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Произошла ошибка при выполнении запроса {requestNumber}: {ex.Message}");
        }
    }

    class ApiResponse
    {
        public string Message { get; set; }
    }
}
