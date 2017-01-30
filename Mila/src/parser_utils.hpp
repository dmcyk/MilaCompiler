//
//  parser_utils.hpp
//  Mila
//
//  Created by Damian Malarczyk on 30.12.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#ifndef parser_utils_hpp
#define parser_utils_hpp

#include <stdio.h>
#include <stdlib.h>
#include "parser.hpp"
#include "lexan.hpp"
#include <string.h>

const char* operationTypeString(OperationType op);
const char* prototypetypeName(PrototypeType pt);
const char* expressionBuiltinTypeDescription(ExpressionBuiltinType type);
#endif /* parser_utils_hpp */
