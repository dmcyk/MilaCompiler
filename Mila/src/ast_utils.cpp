//
//  ast_utils.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include "ast_utils.hpp"
#include "utils.hpp"
namespace AST {
    _Noreturn void errorMessage(const char* format, ...) {
        std::cout << "AST Error" << std::endl;
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        va_end(argptr);
        std::cout << std::endl;
        exit(1);
    }

}
