//
//  utils.cpp
//  Mila
//
//  Created by Damian Malarczyk on 14.12.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#include "utils.hpp"
#include <cstdlib>

ASTDump astDump(std::cout);
ASTDump& ASTDump::operator++() {
    increaseLevel();
    return *this;
}

ASTDump& ASTDump::operator--() {
    decreaseLevel();
    return *this;
}

void ASTDump::dump(const std::string &name, const AST::NodeExpr& expr, const AST::ASTContext& ctx) {
    astDump << "(" << name << std::endl;
    ++astDump;
    expr.print(ctx);
    --astDump;
    astDump << ")\n";
    
}

// ref. http://stackoverflow.com/questions/3381614/c-convert-string-to-hexadecimal-and-vice-versa
std::string hexToString(const std::string& input) {
    static const char* const lut = "0123456789abcdef";
    size_t len = input.length();
    if (len & 1) throw std::invalid_argument("odd length");
    
    std::string output;
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");
        
        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");
        
        output.push_back(((p - lut) << 4) | (q - lut));
    }
    return output;
}

_Noreturn void formattedError(const char* src, ...) {
    va_list argptr;
    va_start(argptr, src);
    vfprintf(stderr, src, argptr);
    va_end(argptr);
    std::cout << std::endl;
    exit(1);
}
