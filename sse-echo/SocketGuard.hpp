//
//  SocketGuard.hpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#ifndef SocketGuard_hpp
#define SocketGuard_hpp

#include <iostream>
#include <stdio.h>
#include <unistd.h> // For close(), read(), write()
#include <cerrno>  // For errno
#include <string> // For memset(), strlen(), strerror()

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const std::string SSE_ENDPOINT_PATH = "/events";


// RAII wrapper for socket file descriptor
class SocketGuard {
private:
    int _fd;
public:
    explicit SocketGuard(int fd);
    ~SocketGuard();

    // Prevent copying
    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;

    // Allow moving
    SocketGuard(SocketGuard&& other) noexcept;
    SocketGuard& operator=(SocketGuard&& other) noexcept;

    int get() const;
};





#endif /* SocketGuard_hpp */
