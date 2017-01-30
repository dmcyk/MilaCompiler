//
//  stdlib.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//
#include "ast_ctx.hpp"

///// STDLIB

using namespace AST; 

std::vector<std::unique_ptr<ParameterDefinitionExpr>> _intArg(const ASTContext* astCtx) {
    std::vector<std::unique_ptr<ParameterDefinitionExpr>> args;
    args.push_back(std::make_unique<ParameterDefinitionExpr>("i", astCtx->getType(vtype_int)));
    return args;
}

std::vector<std::unique_ptr<ParameterDefinitionExpr>> _intPtrArg(const ASTContext* astCtx) {
    std::vector<std::unique_ptr<ParameterDefinitionExpr>> args;
    args.push_back(std::make_unique<ParameterDefinitionExpr>("i", astCtx->getPointerTypeTo(vtype_int)));
    return args;
}

std::unique_ptr<ParameterDefinitionExpr> _strArg(const ASTContext* astCtx, const std::string& name) {
    return std::make_unique<ParameterDefinitionExpr>(name, astCtx->getPointerTypeTo(vtype_int8));
}
std::vector<std::unique_ptr<ParameterDefinitionExpr>> _strArgs(const ASTContext* astCtx) {
    std::vector<std::unique_ptr<ParameterDefinitionExpr>> args;
    args.push_back(_strArg(astCtx, "i"));
    return args;
}

std::unique_ptr<FunctionExpr> _intFn(const std::string& name, const ASTContext* astCtx) {
    return std::make_unique<FunctionExpr>(std::make_unique<PrototypeExpr>(name, _intArg(astCtx), PrototypeType::externd, astCtx->getType(vtype_void)), nullptr);
}

std::unique_ptr<NodeExpr> _intPtrFn(const std::string& name, const ASTContext* astCtx) {
    return std::make_unique<FunctionExpr>(std::make_unique<PrototypeExpr>(name,_intPtrArg(astCtx), PrototypeType::externd, astCtx->getType(vtype_void)), nullptr);
}

std::unique_ptr<FunctionExpr> _strFn(const std::string& name, const ASTContext* astCtx) {
    return std::make_unique<FunctionExpr>(std::make_unique<PrototypeExpr>(name,_strArgs(astCtx), PrototypeType::externd, astCtx->getType(vtype_void)), nullptr);
}

std::unique_ptr<NodeExpr> _emptyFn(const std::string& name, const ASTContext* astCtx) {
    std::vector<std::unique_ptr<ParameterDefinitionExpr>> empty;
    return std::make_unique<FunctionExpr>(std::make_unique<PrototypeExpr>(name, std::move(empty), PrototypeType::externd, astCtx->getType(vtype_void)), nullptr);
}


std::vector<std::unique_ptr<AST::NodeExpr>> ASTContext::milaStdLib() const {
    std::vector<std::unique_ptr<NodeExpr>> stdlib;
    
    stdlib.push_back(_intFn("writeln", this));
    stdlib.push_back(_intFn("write", this));
    stdlib.push_back(_strFn("print", this));
    stdlib.push_back(_strFn("echo", this));
    stdlib.push_back(_intPtrFn("readln", this));
    stdlib.push_back(_intPtrFn("inc", this));
    stdlib.push_back(_intPtrFn("dec", this));
    stdlib.push_back(_emptyFn("newLine", this));
    auto _extFn = _intFn("_exit", this);
    auto _extMsgFn = _intFn("_exitMessage", this);
    _extMsgFn->prototype->args.push_back(_strArg(this, "msg"));
    _extFn->attributes.insert(FunctionAttr::noreturn);
    _extMsgFn->attributes.insert(FunctionAttr::noreturn);
    stdlib.push_back(std::move(_extFn));
    stdlib.push_back(std::move(_extMsgFn)); 
    return stdlib;
}
