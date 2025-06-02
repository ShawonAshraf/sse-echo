//
//  SocketGuard.cpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#include "SocketGuard.hpp"
#include "Exceptions.hpp"

SocketGuard::SocketGuard(int fd) : _fd(fd) {
    if (_fd < 0) { // If an invalid FD is passed (e.g. from a failed socket() call)
        // This guard should ideally only be constructed with a valid FD.
        // The caller should check for fd < 0 before constructing.
        // However, as a safeguard:
        throw SocketException("SocketGuard constructed with invalid file descriptor: " + std::to_string(fd));
    }
}


SocketGuard::~SocketGuard() {
    if (_fd >= 0) { // Check if fd is valid before closing
        // std::cout << "Closing socket fd: " << _fd << std::endl; // For debugging
        if (close(_fd) < 0) {
            // Cannot throw from destructor easily, so just report error
            std::cerr << "Error closing socket fd " << _fd << ": " << strerror(errno) << std::endl;
        }
    }
}

SocketGuard::SocketGuard(SocketGuard&& other) noexcept : _fd(other._fd) {
    other._fd = -1; // Mark the moved-from object as not owning the fd
}

SocketGuard& SocketGuard::operator=(SocketGuard&& other) noexcept {
    if (this != &other) {
        if (_fd >= 0) {
            if (close(_fd) < 0) {
                 std::cerr << "Error closing socket fd " << _fd << " during move: " << strerror(errno) << std::endl;
            }
        }
        _fd = other._fd;
        other._fd = -1;
    }
    return *this;
}


int SocketGuard::get() const { return _fd; }
