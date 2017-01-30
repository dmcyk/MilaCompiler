//
//  ast_context.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "ast_ctx.hpp"
#include "ast_utils.hpp"
using namespace AST; 

static int RuntimeErrorMessageCount = 1;
std::string RuntimeErrorMessage[] = {
    "Runtime error, array index overflow"
};
std::string RuntimeErrorKey[] = {
    "indx_overflow"
};

void ASTContext::initializateForCodeGeneration(LLVMContext &ctx, Module* m) {
    for (int i = 0; i < RuntimeErrorMessageCount; i++) {
        auto current = RuntimeErrorMessage[i];
        // have to initialize global strings 'by hand' builder can not be used on global scope without assigned function
        // thus it's necessary to add them straight to the module itself
        auto type = ArrayType::get(Type::getInt8Ty(ctx), current.size() + 1);
        auto val = m->getOrInsertGlobal(RuntimeErrorKey[i], type);
        if (auto glob = dyn_cast<GlobalVariable>(val)) {
            glob->setConstant(true);
            glob->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
            
            std::vector<Constant*> store;
            auto cStr = current.c_str();
            for (int i = 0; i < current.size() + 1; i++) {
                store.push_back(ConstantInt::get(ctx, APInt(8, cStr[i], true)));
            }
            auto agr = ConstantArray::get(type, store);
            glob->setInitializer(agr);
            glob->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
        }
        runtimeErrorStringMessages[RuntimeErrorKey[i]] = val;
    }
}

Value* ASTContext::runtimeErrorMessageForKey(RuntimeErrorMessageKey key) {
    std::string* mapKey;
    switch (key) {
        case RuntimeErrorMessageKey::arrayIndexOverflow: {
            mapKey = &RuntimeErrorKey[0];
        } break;
    }
    auto found = runtimeErrorStringMessages.find(*mapKey);
    if (found == runtimeErrorStringMessages.end()) {
        errorMessage("Internal error, AST not initialized");
    }
    return found->second;
}
Value* ASTContext::getValue(const std::string &name, Module *m) const {

    auto mutableVal = getMutableValueOpt(name, m, std::experimental::nullopt);
    if (mutableVal) {
        return mutableVal;
    }
    
    if (!stack.empty()) {
        auto current = stack.back();
        auto found = current.find(name);
        if (found != current.end()) {
            return found->second;
        }
    }
    auto foundConst = globalNamedConstants.find(name);
    if (foundConst != globalNamedConstants.end()) {
        return (Value*)(foundConst->second);
    }
    return nullptr;
}

Value* ASTContext::getUserMutableValue(const std::string& name, Module* m) const {
    return getMutableValueOpt(name, m, VariableMutation::userMutable);
}

Value* ASTContext::getPrivateMutableValue(const std::string &name, llvm::Module *m) const {
    return getMutableValueOpt(name, m, std::experimental::nullopt);

}

Value* ASTContext::getMutableValue(const std::string& name, Module* m, VariableMutation mutation) const {
    if (mutation == VariableMutation::innerMutable) {
        return getMutableValueOpt(name, m, std::experimental::nullopt);
    }
    return getMutableValueOpt(name, m, mutation);
}

Value* ASTContext::getMutableValueOpt(const std::string& name, Module* m,std::experimental::optional<VariableMutation> mutation) const {
    if (!mutableStack.empty()) {
        auto current = mutableStack.back();
        auto found = current.find(name);
        if (found != current.end()) {
            if (!mutation || *mutation == std::get<0>(found->second)) {
                return std::get<1>(found->second);
            }

        }
    }
    
    return m->getGlobalVariable(name);
}

const AllocaInst* ASTContext::getFromStack(const std::string &name) const {
    if (stack.empty()) {
        report_fatal_error("no stack defined");
    }
    auto& ref = stack[stack.size() - 1];
    auto check = ref.find(name);
    if (check == ref.end()) {
        return  nullptr;
    }
    return ref.at(name);
    
}
const ArrayExpressionType* ASTContext::getFromArrayTypeStack(const std::string& name) const {
    if (arrayTypeStack.empty()) {
        report_fatal_error("no stack defined");
    }
    auto& ref = arrayTypeStack[arrayTypeStack.size() - 1];
    auto check = ref.find(name);
    if (check == ref.end()) {
        return  nullptr;
    }
    return ref.at(name);
    
}

const ArrayExpressionType* ASTContext::getVariableArrayType(const std::string& name) {
    auto fromStack = getFromArrayTypeStack(name);
    if (fromStack) {
        return fromStack;
    }
    return globalArrayType[name];

}
const ExpressionType* ASTContext::getType(ExpressionBuiltinType type) const {
    switch (type) {
        case vtype_int:
            return &intType;
        case vtype_int32:
            return &int32Type;
        case vtype_void:
            return &voidType;
        case vtype_int8:
            return &int8Type;
        default:
            report_fatal_error("Requesting specialized type on generic one");
            
    }
}

const PointerExpressionType* ASTContext::getPointerTypeTo(ExpressionBuiltinType type) const {
    switch (type) {
        case vtype_int:
            return &pIntType;
        case vtype_int32:
            return &pInt32Type;
        case vtype_void:
            return &pVoidType;
        case vtype_int8:
            return &pInt8Type;
        case vtype_pointer:
            report_fatal_error("nested pointers aren't supported");
        default:
            report_fatal_error("Requesting specialized type on generic one");
    }
}

const ArrayExpressionType* ASTContext::getArrayType(ExpressionBuiltinType to, int64_t start, int64_t end) {
    for (auto& element: arrayTypeStore) {
        if (element->getStoredType() == to && element->startIndex == start && element->endIndex == end) {
            return element.get();
        }
    }
    arrayTypeStore.push_back(std::make_unique<ArrayExpressionType>(to, start, end));
    return arrayTypeStore[arrayTypeStore.size() - 1].get();
}


