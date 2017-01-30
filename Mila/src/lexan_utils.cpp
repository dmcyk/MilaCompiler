//
//  lexan_utils.c
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 12.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#include <string.h>
#include <cstdlib>
#include <iostream>
#include "lexan_utils.hpp"

static const std::string KEYWORD_DATA[] = {
    "if",
    "then",
    "else",
    "switch",
    "case",
    "default",
    "program",
    "function",
    "procedure",
    "extern",
    "const",
    "var",
    "begin",
    "end",
    "for",
    "to",
    "until",
    "downto",
    "downuntil",
    "do",
    "while",
    "exit",
    "and",
    "or",
    "of",
    "mod",
    "div",
    "forward",
    "array"
};

const std::string* getKeywordLiteral(Keyword k) {
    return &KEYWORD_DATA[(int)k];
}

std::experimental::optional<Keyword> getKeywordForLiteral(std::string str) {
    for (int i = 0; i < (int)Keyword::Count; i++) {
        if (str == KEYWORD_DATA[i]) {
            return Keyword(i);
        }
    }
    return std::experimental::nullopt;
}
static const char ALLOWED_IDENTIFIER_SYMBOLS[] = {
    '_', // at start or end of expr 
    '*', '&' // only at the end of the expression
};
static const int ALLOWED_IDENTIFIER_COUNT = 3;
static const int ALLOWED_IDENTIFIER_START_COUNT = 1;


CharacterType getCharacterType(char ch) {
    switch (ch) {
        case 'A'...'Z':
        case 'a'...'z':
            return CharacterType::letter;
        case '0'...'9':
            return CharacterType::digit;
        default:
            return CharacterType::symbol;
    }
}

bool isAllowedIdentifierStartSymbol(char ch) {
    for (int i = 0; i < ALLOWED_IDENTIFIER_START_COUNT; i++) {
        if (ch == ALLOWED_IDENTIFIER_SYMBOLS[i]){
            return true;
        }
    }
    return false;
}

bool isAllowedIdentifierSymbol(char ch) {
    for (int i = 0; i < ALLOWED_IDENTIFIER_COUNT; i++) {
        if (ch == ALLOWED_IDENTIFIER_SYMBOLS[i]){
            return true;
        }
    }
    return false;
}


static const std::string ERROR_DESCRIPTION[] = {
    "Expression was too long",
    "Unrecognized token found" ,
    "Incorrect number literal"
};
const std::string* getErrorLiteral(ErrorType err) {
    switch (err) {
        case ErrorType::tooLongExpression:
            return &ERROR_DESCRIPTION[0];
        case ErrorType::unrecognizedToken:
            return &ERROR_DESCRIPTION[1];
        case ErrorType::incorrectNumberLiteral:
            return &ERROR_DESCRIPTION[2];
    }
}


SeparatorType getSeparatorType(char expr) {
    switch (expr) {
        case ' ':
            return SeparatorType::whitespace;
        case '\r':
            return SeparatorType::carriage_return;
        case '\n':
            return SeparatorType::line_feed;
        case '\t':
            return SeparatorType::tab;
    }
    std::cout << "Lexan error: separator unknown: \"" << expr << "\"" << std::endl;
    exit(1);
    
}
void setSpecialSymbolDataWithKeyword(SpecialSymbol* ss, Keyword k) {
    SpecialSymbolData data;
    data.keyword = k;
    ss->data = data;
    ss->type = SpecialSymbolType::keyword;
}
void setSpecialSymbolDataWithSeparator(SpecialSymbol* ss, SeparatorType s) {
    SpecialSymbolData data;
    data.separator = s;
    ss->data = data;
    ss->type = SpecialSymbolType::separator;
}


void setTokenDataWithError(Token* token, ErrorType err) {
    TokenData data;
    data.error = err;
    token->data = data;
    token->type = TokenType::incorrect;
}


void setTokenDataWithSpecialSymbol(Token* token, SpecialSymbol specialSymbol) {
    TokenData data;
    data.specialSymbol = specialSymbol;
    token->data = data;
    token->type = TokenType::specialsymbol;
}
void setTokenDataWithNumber(Token* token, int n) {
    TokenData data;
    data.number = n;
    token->type = TokenType::number;
    token->data = data;
}
void setTokenDataWithExpression(Token* token, const char expression[]) {
    TokenData data;
    strcpy(data.expression, expression);
    token->data = data;
}
