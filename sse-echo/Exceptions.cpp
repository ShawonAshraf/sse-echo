//
//  Exceptions.cpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#include "Exceptions.hpp"


// Custom Exception for Socket Errors



SocketException::SocketException(const std::string& message) : std::runtime_error(message) {}
