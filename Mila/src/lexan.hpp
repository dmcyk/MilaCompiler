//
//  lexan.h
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 12.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#ifndef lexan_h
#define lexan_h

#include <stdio.h>
#include <string>
#define MAX_EXPRESSION_LENGTH 128

enum class CharacterType {
    letter, digit, symbol
};

enum class TokenType {
    number = 1, expression, specialsymbol, incorrect, string_constant
};

enum class ErrorType {
    tooLongExpression, unrecognizedToken, incorrectNumberLiteral
};

enum class Keyword {
    _if, _then, _else, _switch, _case, _default, _program, _function, _procedure, _extern, _const, _var, _begin, _end, _for, _to, _until, _downto, _downuntil, _do,  _while, _exit, _and, _or, _of,
    _mod, _div, _forward, _array, Count
} ;

enum class SeparatorType {
    whitespace, line_feed, carriage_return, tab
};

//typedef struct {SeparatorType s; char literal;} SeparatorData;

enum class SpecialSymbolType {
    keyword, // 1
    plus, minus, multiplication, // 3
    less, greater, equal, less_or_equal, greater_or_equal, not_equal, negate, // 6
    assignment,  // 1
    colon, // 1
    opening_bracket, closing_bracket, opening_sq_bracket, closing_sq_bracket, // 4
    semicolon, point, coma, apostrophe, // 4
    eof, separator, comment, // 3
    at 
};

typedef union  {
    Keyword keyword;
    SeparatorType separator;
} SpecialSymbolData;


typedef struct {
    SpecialSymbolType type;
    SpecialSymbolData data;
} SpecialSymbol;

typedef union {
    ErrorType error;
    SpecialSymbol specialSymbol;
    char expression[MAX_EXPRESSION_LENGTH];
    uint64_t number;
} TokenData;

typedef struct {
    TokenType type;
    TokenData data;
} Token;

Token nextToken(FILE* input);

#endif /* lexan_h */
