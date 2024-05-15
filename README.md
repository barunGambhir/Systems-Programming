# Systems Programming

Welcome to my systems programming projects repository! This repository contains four projects that I developed using Linux, C, and some C++. Each project showcases different aspects of systems programming, from creating a simple shell to managing memory and synchronization, and building a group chat server with fuzzing clients. Below, you will find a detailed description of each project.

## Projects Overview

### 1. Simple Shell

**Overview:**
The Simple Shell project involves developing a basic command line shell that allows users to enter and execute commands. The shell supports executing commands in both the foreground and background, and includes some internal commands and a history feature.

**Features:**
- Executes user commands in separate processes.
- Supports background execution using the ampersand (&) symbol.
- Implements internal commands (e.g., `cd`, `exit`, etc.).
- Provides a history feature to recall previously executed commands.

**Files:**
- `simple_shell.c`: Main implementation of the shell.

### 2. Memory Management

**Overview:**
The Memory Management project involves implementing a memory allocator using the memory management techniques discussed in class. The allocator is tested using a provided `main.c` file, and all necessary definitions are included in `alloc.h`.

**Features:**
- Custom memory allocation and deallocation.
- Techniques such as first-fit, best-fit, or worst-fit can be implemented.

**Files:**
- `main.c`: Contains the main function to test the allocator.
- `alloc.h`: Contains definitions and function prototypes for the allocator.
- `alloc.c`: Implementation of the memory allocator.

### 3. Synchronization

**Overview:**
The Synchronization project implements the classic producer-consumer problem using threads. It models candy factories (producers) generating candies and kids (consumers) consuming candies from a bounded buffer.

**Features:**
- Multi-threaded implementation using pthreads.
- Synchronization using mutexes and condition variables.
- A bounded buffer to store candies.

**Files:**
- `producer_consumer.c`: Main implementation of the producer-consumer solution.

### 4. A Simple Group Chat Server with Fuzzing Clients

**Overview:**
This project involves creating a simple group chat server and client programs. The server accepts multiple clients and relays messages between them. The clients act as fuzzers, generating and sending random messages to the server.

**Features:**
- Multi-client support on the server.
- Clients generate and send random messages (fuzzing).
- Relays messages from clients to all connected clients.

**Files:**
- `chat_server.c`: Implementation of the chat server.
- `chat_client.c`: Implementation of the chat client with fuzzing.

## Getting Started

To get started with any of these projects, clone the repository and navigate to the project directory. Each project contains a `README.md` with specific instructions on how to compile and run the code.

```sh
git clone https://github.com/barungambhir/Systems-Programming.git
cd Systems-Programming