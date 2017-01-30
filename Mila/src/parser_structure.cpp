//
//  parser_structure.cpp
//  Mila
//
//  Created by Damian Malarczyk on 02.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include "parser_structure.hpp"
#include "parser_utils.hpp"

using namespace llvm;
using namespace AST;

int ExpressionType::size() const {
    switch (builtin) {
        case vtype_int32:
            return 32;
        case vtype_int:
            return 64;
        case vtype_int8:
            return 8;
        default:
            report_fatal_error("Requested size for non static type");
    }
}

OperationType loopCMPOperation(const LoopMode mode, const LoopRange range) {
    if (range == LoopRange::until) {
        return OperationType::op_equal;
    }
    
    if (mode == LoopMode::increasing) {
        return OperationType::op_less;
    } else {
        return OperationType::op_greater;
    }
}

void ExpressionType::printDescription() const {
    std::cout << expressionBuiltinTypeDescription(builtin);
}
int PointerExpressionType::size() const {
    report_fatal_error("requested constant size on pointer type");
}

void PointerExpressionType::printDescription() const {
    std::cout << "pointer(" << expressionBuiltinTypeDescription(getStoredType()) << ")";
}

void ArrayExpressionType::printDescription() const {
    std::cout << "array(" << expressionBuiltinTypeDescription(getStoredType()) << ")";
}

int ArrayExpressionType::size() const {
    report_fatal_error("array llvm type find out...");
}

Type* ArrayExpressionType::llvmType(llvm::LLVMContext &ctx) const {
    return ArrayType::get(ExpressionType::llvmType(ctx), endIndex - startIndex + 1); // all indexes with 0
}


Type* ExpressionType::llvmType(llvm::LLVMContext &ctx) const {
    switch (builtin) {
        case vtype_int32:
            return Type::getInt32Ty(ctx);
        case vtype_int:
            return Type::getInt64Ty(ctx);
        case vtype_void:
            return Type::getVoidTy(ctx);
        case vtype_int8:
            return Type::getInt8Ty(ctx);
        default:
            report_fatal_error("Requesting specialized type on generic one");
    }
}

Type* PointerExpressionType::llvmType(llvm::LLVMContext &ctx) const {
    auto builtinLLVM = this->ExpressionType::llvmType(ctx);
    return PointerType::getUnqual(builtinLLVM);
}

