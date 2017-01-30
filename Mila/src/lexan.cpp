//
//  lexan.c
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 12.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//


#include <string.h>
#include <assert.h>
#include "utils.hpp"
#include "lexan.hpp"
#include "lexan_utils.hpp"
#include "lexan_printing.hpp"

Token buildTokenWithLetter(char ch, FILE* input) {
    char expr[MAX_EXPRESSION_LENGTH] = { 0 };
    expr[0] = ch;
    int pos = 0;
    Token found;
    CharacterType currentType;
    do {
        expr[++pos] = getc(input);
        currentType = getCharacterType(expr[pos]);
        if (pos >= (MAX_EXPRESSION_LENGTH - 1)) {
            
            setTokenDataWithError(&found, ErrorType::tooLongExpression);
            return found;
        }
    } while(currentType != CharacterType::symbol || isAllowedIdentifierSymbol(expr[pos]));
    expr[pos] = 0;
    
    fseek(input, -1, SEEK_CUR);
    
    auto keyword = getKeywordForLiteral(expr);
    if (keyword) {
        SpecialSymbol symbol;
        setSpecialSymbolDataWithKeyword(&symbol, *keyword);
        setTokenDataWithSpecialSymbol(&found, symbol);
        return found;
    }
    
    found.type = TokenType::expression;
    setTokenDataWithExpression(&found, expr);
    return found;
}

Token buildTokenWithDigit(char ch, FILE* input) {
    Token found;
    int base = 10;
    int dataNumber = ch - '0';

    if (ch == '0') {
        char nextCh = getc(input);
        CharacterType type = getCharacterType(nextCh);
        if (type == CharacterType::letter ) {
            if (nextCh == 'x' || nextCh == 'X') {
                base = 16;
            }
        } else if (type == CharacterType::symbol) {
            fseek(input, -1, SEEK_CUR);
            found.type = TokenType::number;
            found.data.number = 0;
            return found;
        } else {
            base = 8;
            dataNumber = base * dataNumber + nextCh - '0';
        }
    }

    char current = getc(input);
    CharacterType currentType = getCharacterType(current);
    while (currentType == CharacterType::digit || currentType == CharacterType::letter) {
        int currentNumber;
        if (current >= 'a') {
            currentNumber = current - 'a' + 10;
        } else if (current >= 'A') {
            currentNumber = current - 'A' + 10;
        } else {
            currentNumber = current - '0';
        }
        if (currentNumber >= base) {
            while (getCharacterType(getc(input)) != CharacterType::symbol);
            fseek(input, -1, SEEK_CUR);
            setTokenDataWithError(&found, ErrorType::incorrectNumberLiteral);
            return found;
        }
        dataNumber = base * dataNumber + currentNumber;
        current = getc(input);
        currentType = getCharacterType(current);
    }
    
    fseek(input, -1, SEEK_CUR);
    setTokenDataWithNumber(&found, dataNumber);
    
    return found;
}

Token buildTokenWithSymbol(char symbol, FILE* input) {
    Token found;
    SpecialSymbol ss;
    SeparatorType separator;
    char nextChar = 0;
    switch (symbol) {
        case '+':
            ss.type = SpecialSymbolType::plus;
            break;
        case '-':
            ss.type = SpecialSymbolType::minus;
            break;
        case ':': {
            nextChar = getc(input);
            if (nextChar == '=') {
                ss.type = SpecialSymbolType::assignment;
                break;
            }
            fseek(input, -1, SEEK_CUR);
            ss.type = SpecialSymbolType::colon;
            break;
        }
        case '@':
            ss.type = SpecialSymbolType::at;
            break; 
        case '(':
            ss.type = SpecialSymbolType::opening_bracket;
            break;
        case ')':
            ss.type = SpecialSymbolType::closing_bracket;
            break;
        case '[':
            ss.type = SpecialSymbolType::opening_sq_bracket;
            break;
        case ']':
            ss.type = SpecialSymbolType::closing_sq_bracket;
            break;
        case '.':
            ss.type = SpecialSymbolType::point;
            break;
        case ',':
            ss.type = SpecialSymbolType::coma;
            break;
        case '\'': {
            found.type = TokenType::string_constant;

            int pos = 0;
            nextChar = getc(input);
            while (nextChar != '\'') {
                found.data.expression[pos++] = nextChar;
                if (pos >= (MAX_EXPRESSION_LENGTH - 1)) {
                    setTokenDataWithError(&found, ErrorType::tooLongExpression);
                    return found;
                }
                nextChar = getc(input);
            }
            found.data.expression[pos] = '\0';
            return found;
        }
        case ';':
            ss.type = SpecialSymbolType::semicolon;
            break;
        case '/':
            nextChar = getc(input);
            if (nextChar == '*') {
            skipComment: while ((nextChar = getc(input)) != '*' && nextChar != EOF);
                if (nextChar == EOF) {
                    formattedError("Incorrect input, comment not closed");
                }
                while ((nextChar = getc(input)) == '*');
                if (nextChar != symbol) {
                    goto skipComment;
                }
                ss.type = SpecialSymbolType::comment;
                break;
            } else {
                setTokenDataWithError(&found, ErrorType::unrecognizedToken);
                return found;
            }
        case '*':
            ss.type = SpecialSymbolType::multiplication;
            break;
        case '<':
            nextChar = getc(input);
            if (nextChar == '>') {
                ss.type = SpecialSymbolType::not_equal;
                break;
            } else if (nextChar == '=') {
                ss.type = SpecialSymbolType::less_or_equal;
                break;
            }
            fseek(input, -1, SEEK_CUR);
            ss.type = SpecialSymbolType::less;
            break;
        case '>':
            nextChar = getc(input);
            if (nextChar == '=') {
                ss.type = SpecialSymbolType::greater_or_equal;
                break;
            }
            fseek(input, -1, SEEK_CUR);
            ss.type = SpecialSymbolType::greater;
            break;
        case '=':
            ss.type = SpecialSymbolType::equal;
            break;
        case '"': {
            char expr[MAX_EXPRESSION_LENGTH] = { 0 };
            int pos = 0;
            nextChar = getc(input);
            while (nextChar != '"' && pos < MAX_EXPRESSION_LENGTH) {
                if (nextChar == '\\') {
                    char upcomming = getc(input);
                    switch (upcomming) {
                        case 'a':
                            nextChar = '\a'; break;
                        case 'b':
                            nextChar = '\b'; break;
                        case 'f':
                            nextChar = '\f'; break;
                        case 'n':
                            nextChar = '\n'; break;
                        case 'r':
                            nextChar = '\r'; break;
                        case 't':
                            nextChar = '\t'; break;
                        case 'v':
                            nextChar = '\v'; break;
                        case '\'':
                            nextChar = '\''; break;
                        case '\"':
                            nextChar = '\"'; break;
                        case '\?':
                            nextChar = '\?'; break;
                        default:
                            nextChar = '\\'; break;
                    }
                }
                expr[pos++] = nextChar;
                nextChar = getc(input);
            }
            
            if (pos >= MAX_EXPRESSION_LENGTH) {
                printf("Too long expression\n");
                exit(-1);
            }
            found.type = TokenType::string_constant;
            strncpy(found.data.expression, expr, MAX_EXPRESSION_LENGTH);
            return found;
        }
        case EOF:
            ss.type = SpecialSymbolType::eof;
            break;
        case ' ':
        case '\r':
        case '\n':
        case '\t':
            separator = getSeparatorType(symbol);
            setSpecialSymbolDataWithSeparator(&ss, separator);
            break;
        default:
            setTokenDataWithError(&found, ErrorType::unrecognizedToken);
            return found;
            
    }
    setTokenDataWithSpecialSymbol(&found, ss);
    return found;
    
}

Token nextToken(FILE* input) {
    char current = getc(input);
    CharacterType currentType = getCharacterType(current);
    Token found;
    found.type = TokenType::incorrect;
    switch (currentType) {
        case CharacterType::letter:
            found = buildTokenWithLetter(current, input);
            break;
        case CharacterType::digit:
            found = buildTokenWithDigit(current, input);
            break;
        case CharacterType::symbol:
            if (isAllowedIdentifierStartSymbol(current))
                found = buildTokenWithLetter(current, input);
            else
                found = buildTokenWithSymbol(current, input);
            if (found.type == TokenType::specialsymbol) {
                if (found.data.specialSymbol.type == SpecialSymbolType::separator || found.data.specialSymbol.type == SpecialSymbolType::comment) {
                    return nextToken(input);
                }
            }
            break;
    }
    return found;
    
};
