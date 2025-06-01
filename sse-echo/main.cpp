//
//  main.cpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For std::chrono::seconds
#include <stdexcept> // For std::runtime_error
#include <system_error> // For std::error_code, std::system_category (though strerror is often simpler)




// include headers
#include "Exceptions.hpp"
#include "SocketGuard.hpp"
#include "Utils.hpp"



int main() {
    int server_fd_raw = -1;
    // Using a raw fd first, then passing to SocketGuard once successfully created.
    // SocketGuard itself should not be in a state where it holds an invalid fd from construction if possible.

    try {
        server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_raw < 0) { // socket() returns -1 on error
            throw SocketException("Socket creation failed: " + std::string(strerror(errno)));
        }
        SocketGuard server_socket_guard(server_fd_raw); // Manages server_fd_raw from now on

        int opt = 1;
        // Use only SO_REUSEADDR for broader compatibility
        if (setsockopt(server_socket_guard.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            // It's still good to check for errors with SO_REUSEADDR, though it's more commonly available.
            throw SocketException("setsockopt SO_REUSEADDR failed: " + std::string(strerror(errno)));
        }

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_socket_guard.get(), (struct sockaddr *)&address, sizeof(address)) < 0) {
            throw SocketException("Bind failed: " + std::string(strerror(errno)));
        }
        if (listen(server_socket_guard.get(), 3) < 0) {
            throw SocketException("Listen failed: " + std::string(strerror(errno)));
        }

        std::cout << "SSE Echo Server listening on port " << PORT << std::endl;
        std::cout << "Try connecting from a browser with: http://localhost:" << PORT << "/events?message=Hello" << std::endl;

        while (true) {
            int new_socket_fd = -1;
            struct sockaddr_in client_address;
            socklen_t client_addrlen = sizeof(client_address);
            
            new_socket_fd = accept(server_socket_guard.get(), (struct sockaddr *)&client_address, &client_addrlen);
            
            if (new_socket_fd < 0) {
                if (errno == EINTR) { // Interrupted system call
                    std::cout << "accept() call interrupted, retrying..." << std::endl;
                    continue;
                }
                // For other accept errors, print error but server continues.
                // A more robust server might stop on certain critical accept errors.
                perror("accept failed");
                continue;
            }
            
            std::cout << "Accepted new connection, fd: " << new_socket_fd << std::endl;

            try {
                handle_client(new_socket_fd); // new_socket_fd is managed by SocketGuard inside handle_client
            } catch (const SocketException& e) {
                std::cerr << "Error handling client (fd: " << new_socket_fd << ") [SocketException]: " << e.what() << std::endl;
                // SocketGuard in handle_client should have closed new_socket_fd.
            } catch (const std::exception& e) {
                std::cerr << "Error handling client (fd: " << new_socket_fd << ") [std::exception]: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Error handling client (fd: " << new_socket_fd << ") [Unknown exception]." << std::endl;
            }
        }
        // server_socket_guard will close server_fd when main exits (e.g. if loop was breakable)
    } catch (const SocketException& e) {
        std::cerr << "Server critical setup error: " << e.what() << std::endl;
        // If server_fd_raw was opened but SocketGuard not constructed or error before guard took ownership
        // This case is mostly covered by throwing before guard construction or guard handling it.
        // If socket() call itself failed, server_fd_raw is < 0 and doesn't need closing.
        // If socket() succeeded but a later step failed before guard construction (not current logic), then manual close of server_fd_raw needed.
        // With current logic, if socket() fails, exception is thrown, server_fd_raw is not used by guard.
        // If socket() succeeds, guard takes over. If subsequent setup fails, guard cleans server_fd_raw.
        return EXIT_FAILURE;
    } catch (const std::exception& e) { // Catch any other unexpected std exceptions during setup
        std::cerr << "An unexpected server error occurred during setup: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; // Should not be reached in this infinite loop server
}
