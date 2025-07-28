# Binary File Transfer System

This project is a high-performance binary file transfer system implemented using C and C#.  
The main goal is to send large binary files (e.g. 4GB+) from a sender application written in C to a receiver written in C#, ensuring speed, integrity, and correct ordering of data chunks.

## Features

- Chunk-based file transfer (default: 64MB per chunk)
- MD5 and xxHash checksum validation for data integrity
- ZeroMQ-based messaging (PUSH-PULL pattern)
- Memory-mapped file writing on receiver side for performance
- Simple logging and progress feedback in terminal

## Technologies Used

- C (GCC)
- C# (.NET Core / .NET 6+)
- ZeroMQ
- xxHash
- MemoryMappedFile (System.IO.MemoryMappedFiles)

## Project Structure

```
BinaryFileTransferProject/
│
├── sender/                  # C project - reads and sends binary chunks
│   ├── main.c
│   ├── Makefile
│   └── xxHash/              # Integrated xxHash source code
│
├── receiver/                # C# project - receives and writes chunks
│   ├── Program.cs
│   └── BinaryReceiver.csproj
│
├── .gitignore
├── README.md
└── BinaryFileTransferProject.sln
```

## How It Works

1. **Sender** reads the file in chunks (64MB default), calculates hash, and sends the chunk over ZeroMQ.
2. **Receiver** listens to incoming chunks and writes them to the correct position in the target file using a memory-mapped file.
3. Both sides validate integrity with hash comparison.
4. Once all chunks are received and validated, the receiver finalizes the file.

## Build & Run Instructions

### Prerequisites

- `libzmq` installed (for C sender)
- `xxHash` source included (no external dependency)
- .NET SDK 6 or later (for C# receiver)

### Build Sender (C)

```bash
cd sender
make
./sender /path/to/your/file.bin
```

### Run Receiver (C#)

```bash
cd receiver
dotnet run
```

The receiver waits for chunks and reconstructs the binary file once all parts arrive.

## Notes

- Currently designed for local testing; network configurations (e.g. TCP addresses) can be adjusted easily.
- Chunk size can be modified from `main.c`.
- Performance and reliability optimized for large files (e.g. 4.7GB test successful in under ~5 seconds total transfer).
