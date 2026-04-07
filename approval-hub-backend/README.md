# Approval Hub - Backend (C++)

## Prerequisites
- C++ compiler (g++ 11+)
- Crow library (header-only): https://crowcpp.org
- Asio library (header-only): https://think-async.com/Asio/

## Setup
1. Download `crow_all.h` from Crow releases
2. Download Asio headers
3. Place both in this directory (or adjust include paths)

## Compile & Run
```bash
g++ -std=c++17 -I. main.cpp -o server -lpthread
./server
```

Server runs on http://127.0.0.1:5000

## Seed Accounts
| Email              | Password  | Role   |
|--------------------|-----------|--------|
| admin@portal.com   | admin123  | admin  |
| client@acme.com    | client123 | client |

## API Endpoints
| Method | Endpoint         | Description          |
|--------|------------------|----------------------|
| POST   | /login           | Authenticate user    |
| GET    | /documents       | List all documents   |
| GET    | /document/<id>   | Get document detail  |
| POST   | /comment         | Add comment          |
| POST   | /status          | Change doc status    |
| POST   | /upload          | Upload new document  |
| GET    | /delete/<id>     | Delete document      |
