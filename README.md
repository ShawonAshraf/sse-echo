# SSE Echo Server

A lightweight C++ Server-Sent Events (SSE) echo server that accepts messages via query parameters and streams them back to connected clients. Perfect for testing SSE functionality and real-time web applications.


## Project Structure

```
sse-echo/
├── CMakeLists.txt          # Build configuration
├── README.md              # This file
├── client.html            # Test HTML client
├── .gitignore            # Git ignore rules
└── sse-echo/             # Source code directory
    ├── main.cpp          # Server entry point
    ├── Utils.hpp         # Utility functions and request handling
    ├── SocketGuard.hpp   # RAII socket wrapper (header)
    ├── SocketGuard.cpp   # RAII socket wrapper (implementation)
    ├── Exceptions.hpp    # Custom exception classes (header)
    └── Exceptions.cpp    # Custom exception classes (implementation)
```

## Building the Project

### 1. Create Build Directory
```bash
mkdir build
cd build
```

### 2. Configure with CMake
```bash
cmake ..
```

### 3. Build the Project
```bash
cmake --build .
# or alternatively:
make
```

### Build Types

```bash
# For debug build:
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# For release build:
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Running the Server

### Start the Server

```bash
./sse-echo
```

The server will start and display:

```
SSE Echo Server listening on port 8080
Try connecting from a browser with: http://localhost:8080/events?message=Hello
```

### Server Configuration

- **Port**: 8080 (configured in `SocketGuard.hpp`)
- **Endpoint**: `/events`
- **Connection timeout**: 5 seconds per client

## API Documentation

### SSE Endpoint

**Endpoint**: `GET /events`

**Query Parameters**:
- `message` (optional): The message to echo back to the client

**Response**:
- **Content-Type**: `text/event-stream`
- **Headers**: CORS-enabled, no-cache
- **Event Type**: `messageEcho`

### Examples

#### Basic Request
```bash
curl -N "http://localhost:8080/events?message=Hello%20World"
```

#### Response
```
HTTP/1.1 200 OK
Content-Type: text/event-stream
Cache-Control: no-cache
Connection: keep-alive
Access-Control-Allow-Origin: *

id: 1704067200123456789
event: messageEcho
data: Hello World

```

#### No Message Parameter
```bash
curl -N "http://localhost:8080/events"
```
Returns: `[No query parameters provided]`

#### Invalid Endpoint
```bash
curl "http://localhost:8080/invalid"
```
Returns: `HTTP/1.1 404 Not Found`

## Testing with Browser Client

The project includes a test HTML client (`client.html`) for easy testing:

### 1. Start the Server
```bash
./sse-echo
```

### 2. Open the HTML Client
Open `client.html` in your web browser, or serve it via a local web server:

```bash
# Option 1: Direct file open
open client.html  # macOS
xdg-open client.html  # Linux

# Option 2: Simple HTTP server
python3 -m http.server 3000
# Then visit: http://localhost:3000/client.html
```

### 3. Test the Connection
1. Enter a message in the input field
2. Click "Connect SSE"
3. Watch real-time messages appear

### 4. Debug Mode

For detailed debugging, build in debug mode:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## License

This project is provided as-is for educational and development purposes.
