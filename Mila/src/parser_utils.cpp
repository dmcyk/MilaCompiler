//
//  parser_utils.cpp
//  Mila
//
//  Created by Damian Malarczyk on 30.12.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#include "parser_utils.hpp"
#include "lexan_printing.hpp"

using namespace AST;

static const char* OPERATIONTYPE_NAME[] = {
    "Undefined",
    
    "Addition",
    "Substraction",
    "Division",
    "Multiplication",
    
    "Less",
    "Greater",
    "Equal",
    "Less or equal",
    "Greater or equal",
    "Not equal",
    
    "Binary And",
    "Binary Or",
    "Binary Negation",
    "Unary minus",
    "Modulo"
};

static const char* PROTOTYPETYPE_NAME[] = {
    "Local",
    "Extern"
};

static const char* EXPRESSION_BULTIN_TYPE_DESC[] = {
    "int_8",
    "int_32",
    "int_64",
    "void",
    "pointer",
    "array" 
};

const char* operationTypeString(OperationType op) {
    return OPERATIONTYPE_NAME[op + 1];
}
const char* prototypetypeName(PrototypeType pt) {
    return PROTOTYPETYPE_NAME[(int)pt];
}

const char* expressionBuiltinTypeDescription(ExpressionBuiltinType type) {
    return EXPRESSION_BULTIN_TYPE_DESC[type];
}



