//
//  Exceptions.hpp
//  sse-echo
//
//  Created by Shawon Ashraf on 1/6/25.
//

#ifndef Exceptions_hpp
#define Exceptions_hpp

#include <string>

class SocketException : public std::runtime_error {
public:
    SocketException(const std::string& message);
};

#endif /* Exceptions_hpp */
