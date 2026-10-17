# 🌐 HTTP Server in C

A lightweight HTTP server built from scratch in C using raw POSIX sockets. No frameworks, no external libraries. Just system calls, a parser, and a lot of learning about how the web actually works under the hood. 🛠️

---

## ✨ Features

- 🔌 **Raw TCP sockets** using `socket`, `bind`, `listen`, and `accept`
- 📨 **HTTP request parsing** for the method, path, and version
- 📤 **Valid HTTP responses** with a status line, headers, and body
- 📁 **Static file serving** from the `public/` directory
- 🧾 **Content-Type detection** based on file type
- 🚫 **404 handling** for missing paths
- 🧵 **Concurrent connections** with a thread per connection, each handler reading from and writing to its own client socket

---

## 🧱 How It Works

```
Client (browser / curl)
        │
        ▼
  accept() connection
        │
        ▼
  spawn thread ──► read raw request
                        │
                        ▼
                parse method, path, version
                        │
                        ▼
              map path to file in public/
                        │
              ┌─────────┴─────────┐
              ▼                   ▼
        file found           file missing
        200 OK + body        404 Not Found
              │                   │
              └─────────┬─────────┘
                        ▼
                send response, close socket
```

---

## 📂 Project Structure

```
.
├── src/            # Server source code
├── public/         # Static files served by the server
│   └── index.html
├── Makefile        # Build instructions (if applicable)
└── README.md
```

> 📝 Adjust this tree to match your actual layout.

---

## 🚀 Getting Started

### Prerequisites

- 🐧 Linux or WSL (developed on Ubuntu 22.04 under WSL)
- 🔧 `gcc`
- 🧶 POSIX threads (`-pthread`)

### Build

```bash
gcc -Wall -Wextra -pthread -o server src/*.c
```

### Run

```bash
./server
```

Then open your browser at:

```
http://localhost:<PORT>
```

> 📝 Replace `<PORT>` with the port your server listens on.

---

## 🧪 Testing

Try it out with a few different tools:

```bash
# See the full request and response
curl -v http://localhost:<PORT>/

# Request a file that does not exist (expect a 404)
curl -v http://localhost:<PORT>/nope.html

# Send a raw request by hand
nc localhost <PORT>
GET / HTTP/1.1
Host: localhost
```

🧠 Memory bugs are easy to introduce in C, so it is worth running the server under Valgrind too:

```bash
valgrind --leak-check=full ./server
```

---

## 📚 What I Learned

- 🔩 How sockets work at the system call level
- 📖 What an HTTP request and response actually look like on the wire
- 🧮 Manual memory management and buffer handling in C
- 🧵 Handling multiple clients at once with threads
- 🐛 Debugging with `curl -v`, `nc`, and Valgrind
