using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;

class async_replace
{
    static async Task Main(string[] args)
    {
        Console.WriteLine("enter file path:");
        string filePath = Console.ReadLine();

        try
        {
            string text = await ReadTextFromFileAsync(filePath);

            Console.WriteLine("enter chars to remove:");
            string charsToRemove = Console.ReadLine();


            string newText = await RemoveCharsAsync(text, charsToRemove);

            await SaveTextToFileAsync(filePath, newText);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"error: {ex.Message}");
        }
    }

    static async Task<string> ReadTextFromFileAsync(string filePath)
    {
        using (StreamReader reader = new StreamReader(filePath))
        {
            return await reader.ReadToEndAsync();
        }
    }

    static async Task<string> RemoveCharsAsync(string text, string charsToRemove)
    {
        StringBuilder newText = new StringBuilder(text);

        foreach (char c in charsToRemove)
        {
            newText.Replace(c.ToString(), string.Empty);
        }

        return await Task.FromResult(newText.ToString());
    }

    static async Task SaveTextToFileAsync(string filePath, string text)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            await writer.WriteAsync(text);
        }
    }
}
