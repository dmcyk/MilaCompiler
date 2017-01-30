//
//  lexan_printing.c
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 12.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#include "lexan_printing.hpp"
#include "lexan_utils.hpp"
#include <iostream>

static const char* SPECIALSYMBOL_DESCRITPION[] = {
    "keyword",
    
    "plus",
    "minus",
    "multiplication",
    
    "less",
    "greater",
    "equal",
    "less_or_equal",
    "greater_or_equal",
    "not_equal",
    "negate !",
    
    "asignment",
    
    "colon",
    
    "opening_bracket",
    "closing_bracket",
    "opening_sq_bracket",
    "closing_sq_bracket",
    
    "semicolon",
    "point",
    "coma",
    "apostrophe",
    
    "eof",
    "separator",
    "comment",
    
    "func_attribute"
};

static const int SPECIALSYMBOL_COUNT = 25;

void printSpecialSymbolDescription(SpecialSymbolType type) {
    int topr = (int)type;
    if (topr < SPECIALSYMBOL_COUNT)
        printf("<%s>", SPECIALSYMBOL_DESCRITPION[topr]);
    else {
        printf("\n \tPrinting incorrect special symbol\n");
    }
}

void printToken(Token token) {
    switch (token.type) {
        case TokenType::specialsymbol: {
            SpecialSymbol data = token.data.specialSymbol;
            
            switch (data.type) {
                case SpecialSymbolType::keyword: {
                    printSpecialSymbolDescription(data.type);

                    std::cout << "\n\t " << *getKeywordLiteral(data.data.keyword) << std::endl;
                    break;
                }
                case SpecialSymbolType::comment:
                case SpecialSymbolType::separator:
                    break;
                default:
                    printSpecialSymbolDescription(data.type);
                    break;
                    
            }
            if (data.type != SpecialSymbolType::comment && data.type != SpecialSymbolType::separator)
                printf("\n");
            break;
        }
        case TokenType::number:
            printf("<number>\n\t %llu\n", token.data.number);
            break;
        case TokenType::expression:
            printf("<expression>\n\t %s\n", token.data.expression);
            break;
        case TokenType::incorrect:
            printf("<incorrect>\n\t");
            std::cout << "description: " << *getErrorLiteral(token.data.error) << std::endl;
            break;
        case TokenType::string_constant:
            std::cout << "<string_constant>\n\t" << token.data.expression << std::endl;
            break;
    }
}
