using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using NetMQ;
using NetMQ.Sockets;
using K4os.Hash.xxHash;

class Program
{
    const string OutputPath = "received_output.bin";

    static void Main(string[] args)
    {
        Console.WriteLine("🚀 Receiver hazır, chunk bekleniyor...");

        var receivedChunks = new List<byte[]>();
        var stopwatch = System.Diagnostics.Stopwatch.StartNew();
        int chunkCounter = 0;

        using var socket = new PullSocket(">tcp://localhost:5555");

        while (true)
        {
            var message = socket.ReceiveMultipartBytes();

            if (message.Count == 1 && Encoding.UTF8.GetString(message[0]) == "END")
                break;

            if (message.Count != 2)
            {
                Console.WriteLine($"⚠️ Beklenmeyen mesaj yapısı! Chunk {chunkCounter} atlandı.");
                continue;
            }

            var chunkData = message[0];
            var receivedHashBytes = message[1];
            ulong receivedHash = BitConverter.ToUInt64(receivedHashBytes);
            ulong calculatedHash = XXH64.DigestOf(chunkData);

            bool match = (receivedHash == calculatedHash);
            string status = match ? "✅ Doğrulandı" : "❌ HATALI";

            Console.WriteLine($"📥 Chunk {chunkCounter} alındı ({chunkData.Length / (1024.0 * 1024.0):F2} MB) - xxHash: {status}");
            Console.WriteLine($"     🔍 Gelen   : {receivedHash:x16}");
            Console.WriteLine($"     🔄 Hesaplanan: {calculatedHash:x16}");

            if (match)
                receivedChunks.Add(chunkData);
            else
                Console.WriteLine($"⛔ Chunk {chunkCounter} diske yazılmadı (Hash uyuşmazlığı).");

            chunkCounter++;
        }

        using var fs = new FileStream(OutputPath, FileMode.Create, FileAccess.Write);
        foreach (var chunk in receivedChunks)
            fs.Write(chunk, 0, chunk.Length);

        stopwatch.Stop();
        Console.WriteLine("🕓 END sinyali alındı. Tüm chunklar işlendi.");
        Console.WriteLine($"📊 Toplam chunk: {receivedChunks.Count}");
        Console.WriteLine($"✅ Chunklar diske yazıldı.");
        Console.WriteLine($"⏱ Toplam süre: {stopwatch.Elapsed.TotalSeconds:F2} saniye");
    }
}
