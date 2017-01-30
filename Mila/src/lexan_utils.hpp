//
//  lexan_utils.h
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 12.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#ifndef lexan_utils_h
#define lexan_utils_h

#include <stdbool.h>
#include <cstdio>
#include <string>
#include <experimental/optional>
#include "lexan.hpp"

SeparatorType getSeparatorType(char expr);
const std::string* getKeywordLiteral(Keyword k);
const std::string* getErrorLiteral(ErrorType err);
std::experimental::optional<Keyword> getKeywordForLiteral(std::string);
CharacterType getCharacterType(char ch);
bool isAllowedIdentifierStartSymbol(char ch);
bool isAllowedIdentifierSymbol(char ch);
void setSpecialSymbolDataWithKeyword(SpecialSymbol* ss, Keyword k);
void setSpecialSymbolDataWithSeparator(SpecialSymbol* ss, SeparatorType s);

void setTokenDataWithError(Token* token, ErrorType err);

void setTokenDataWithErrorDescriptionIndex(Token* token, int index);
void setTokenDataWithSpecialSymbol(Token* token, const SpecialSymbol specialSymbol);
void setTokenDataWithNumber(Token* token, int n);
void setTokenDataWithExpression(Token* token, const char expression[]);
#endif /* lexan_utils_h */
