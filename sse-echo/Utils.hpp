//
//  Utils.hpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#ifndef Utils_h
#define Utils_h

// For POSIX sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>


// Basic URL decoding for %XX
std::string url_decode(const std::string& encoded_string) {
    std::string decoded_string = "";
    char ch;
    int ii; // Use int for sscanf
    for (size_t current_idx = 0; current_idx < encoded_string.length(); ++current_idx) {
        if (encoded_string[current_idx] == '%') {
            if (current_idx + 2 < encoded_string.length() &&
                isxdigit(static_cast<unsigned char>(encoded_string[current_idx+1])) &&
                isxdigit(static_cast<unsigned char>(encoded_string[current_idx+2]))) {
                // sscanf is C-style, be careful with buffer overflows if source string wasn't fixed size.
                // Here substr is safer.
                if (sscanf(encoded_string.substr(current_idx + 1, 2).c_str(), "%x", &ii) == 1) {
                    ch = static_cast<char>(ii);
                    decoded_string += ch;
                    current_idx = current_idx + 2;
                } else { // sscanf failed
                     decoded_string += encoded_string[current_idx]; // Pass through '%'
                }
            } else { // Invalid % sequence
                decoded_string += encoded_string[current_idx]; // Pass through '%'
            }
        } else if (encoded_string[current_idx] == '+') {
            decoded_string += ' ';
        } else {
            decoded_string += encoded_string[current_idx];
        }
    }
    return decoded_string;
}

// Extracts a query parameter value by name
std::string extract_query_param(const std::string& query_string_with_qmark, const std::string& param_name) {
    if (query_string_with_qmark.empty() || query_string_with_qmark.length() < 2) { // Need at least "?a=b"
        return "";
    }

    std::string actual_query = query_string_with_qmark;
    // query_string_with_qmark is expected to be like "message=Hello" or "?message=Hello"
    // The parsing in handle_client gives "message=Hello" to query_string
    // if path_query = /events?message=Hello, query_string = message=Hello

    std::istringstream iss(actual_query); // actual_query should not start with '?' here based on usage
    std::string segment;
    while (std::getline(iss, segment, '&')) {
        size_t eq_pos = segment.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = segment.substr(0, eq_pos);
            if (key == param_name) {
                return url_decode(segment.substr(eq_pos + 1));
            }
        }
    }
    return ""; // Parameter not found
}

// Helper function to ensure all data is written
void full_write(int fd, const std::string& data) {
    ssize_t total_bytes_written = 0;
    const char* p_data = data.c_str();
    size_t data_len = data.length();

    while (total_bytes_written < static_cast<ssize_t>(data_len)) {
        ssize_t bytes_written_this_call = write(fd, p_data + total_bytes_written, data_len - total_bytes_written);
        if (bytes_written_this_call < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            throw SocketException("Failed to write to socket (fd: " + std::to_string(fd) + "): " + strerror(errno));
        }
        if (bytes_written_this_call == 0) { // Should not happen with blocking sockets unless error or non-blocking
            throw SocketException("Write to socket (fd: " + std::to_string(fd) + ") returned 0, connection may be closed prematurely.");
        }
        total_bytes_written += bytes_written_this_call;
    }
}


void handle_client(int client_socket_fd) {
    SocketGuard client_socket_guard(client_socket_fd); // RAII for client socket

    char buffer[BUFFER_SIZE] = {0};
    long bytes_read = read(client_socket_guard.get(), buffer, BUFFER_SIZE - 1);

    if (bytes_read < 0) {
        throw SocketException("Failed to read from socket (fd: " + std::to_string(client_socket_guard.get()) + "): " + strerror(errno));
    }
    if (bytes_read == 0) {
        std::cout << "Client (fd: " << client_socket_guard.get() << ") disconnected before sending data." << std::endl;
        return; // SocketGuard will close
    }
    buffer[bytes_read] = '\0';
    std::string request_str(buffer);
    std::cout << "Received request from fd " << client_socket_guard.get() << ":\n" << request_str << std::endl;

    std::istringstream request_stream(request_str);
    std::string method, path_query, http_version;
    request_stream >> method >> path_query >> http_version;

    std::string path;
    std::string query_params_str; // This string will be "message=Hello" not "?message=Hello"
    size_t query_pos = path_query.find('?');
    if (query_pos != std::string::npos) {
        path = path_query.substr(0, query_pos);
        if (path_query.length() > query_pos + 1) {
            query_params_str = path_query.substr(query_pos + 1);
        }
    } else {
        path = path_query;
    }

    if (method == "GET" && path == SSE_ENDPOINT_PATH) {
        std::string message_to_echo = extract_query_param(query_params_str, "message");

        if (message_to_echo.empty() && !query_params_str.empty() && query_params_str.find("message=") == std::string::npos) {
            // query param was present but not 'message' or 'message' was empty
             message_to_echo = "[Query param 'message' not found or empty in '" + query_params_str + "']";
        } else if (message_to_echo.empty() && query_params_str.empty()){
            message_to_echo = "[No query parameters provided]";
        }


        std::cout << "Client (fd: " << client_socket_guard.get() << ") connected to SSE endpoint. Echoing: " << message_to_echo << std::endl;

        std::ostringstream sse_response_header;
        sse_response_header << "HTTP/1.1 200 OK\r\n";
        sse_response_header << "Content-Type: text/event-stream\r\n";
        sse_response_header << "Cache-Control: no-cache\r\n";
        sse_response_header << "Connection: keep-alive\r\n";
        sse_response_header << "Access-Control-Allow-Origin: *\r\n";
        sse_response_header << "\r\n";
        full_write(client_socket_guard.get(), sse_response_header.str());

        std::ostringstream sse_event;
        sse_event << "id: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
        sse_event << "event: messageEcho\n";
        sse_event << "data: " << message_to_echo << "\n\n";
        full_write(client_socket_guard.get(), sse_event.str());
        std::cout << "Sent SSE event to fd " << client_socket_guard.get() << ": " << sse_event.str();

        std::cout << "Keeping connection open for 5 seconds for fd " << client_socket_guard.get() << " (iterative server behavior)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Implicitly closing client connection for fd " << client_socket_guard.get() << " via SocketGuard." << std::endl;

    } else {
        std::cout << "Received non-SSE request or invalid path from fd " << client_socket_guard.get() << ". Sending 404." << std::endl;
        std::string response_404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";
        full_write(client_socket_guard.get(), response_404);
    }
    // SocketGuard automatically closes client_socket_fd when handle_client exits
}


#endif /* Utils_h */
