//
//  parser_structure.h
//  Mila
//
//  Created by Damian Malarczyk on 02.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef parser_structure_h
#define parser_structure_h
#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;


typedef enum {
    op_undefined = -1,
    op_plus, op_minus, op_division, op_multiplication,
    op_less, op_greater, op_equal, op_less_or_equal, op_greater_or_equal, op_not_equal,
    op_and, op_or, op_negate, op_unary_minus,
    op_modulo
} OperationType;

typedef enum {
    vtype_int8, vtype_int32, vtype_int, vtype_void, vtype_pointer, vtype_array
} ExpressionBuiltinType;

enum class PrototypeType {
    local, externd
};

enum class LoopMode {
    increasing, decreasing
};

enum class LoopRange {
    to, until
};

OperationType loopCMPOperation(const LoopMode mode, const LoopRange range);
class ExpressionType {
protected:
    ExpressionBuiltinType builtin;
    
public:
    ExpressionType(ExpressionBuiltinType builtin): builtin(builtin) {};
    virtual Type* llvmType(LLVMContext& ctx) const;
    virtual ExpressionBuiltinType getBuiltin() const { return builtin; }
    virtual int size() const;
    virtual void printDescription() const;
    inline friend bool operator==(const ExpressionType& lhs, const ExpressionType& rhs);
    ExpressionBuiltinType getStoredType() const {
        return builtin;
    }
    
};
inline bool operator==(const ExpressionType& lhs, const ExpressionType& rhs) {
    return lhs.builtin == rhs.builtin;
}

class PointerExpressionType: public ExpressionType {
public:
    
    virtual ExpressionBuiltinType getBuiltin() const override { return vtype_pointer; }
    
    PointerExpressionType(ExpressionBuiltinType to): ExpressionType(to) {
    };
    
    virtual int size() const override;
    virtual void printDescription() const override;
    virtual Type* llvmType(LLVMContext& ctx) const override;
};

class ArrayExpressionType: public ExpressionType {
public:
    
    virtual ExpressionBuiltinType getBuiltin() const override { return vtype_array; }
    virtual int size() const override;
    virtual void printDescription() const override;
    virtual Type* llvmType(LLVMContext& ctx) const override;
    int64_t startIndex;
    int64_t endIndex;
    
    ArrayExpressionType(ExpressionBuiltinType of, int64_t startIndex, int64_t endIndex)
    :
    ExpressionType(of),
    startIndex(startIndex),
    endIndex(endIndex) {
        
    };
};




#endif /* parser_structure_h */
