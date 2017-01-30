//
//  parser.hpp
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 11.11.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <set>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include "parser_structure.hpp"
#include "lexan_printing.hpp"
#include "lexan_utils.hpp"
#include "utils.hpp"
#include "ast_op.hpp"
#include "ast_ctx.hpp"
using namespace llvm;
using namespace AST;

int parserRead(FILE* fp, const std::string& outputname, bool dumpIR);

std::experimental::optional<int64_t>  computeNumberNode(const AST::NodeExpr* node, const AST::ASTContext& ctx);
std::experimental::optional<int64_t>  computeOperation(const AST::OperationExpr* op, const AST::ASTContext& ctx);

class Parser {
private:
    
    std::unique_ptr<ProgramExpr> program;
    Token current;
    FILE* input;
    AST::ASTContext astCtx;
    
    _Noreturn void error();
    _Noreturn void error(TokenType t);
    _Noreturn void error(SpecialSymbolType ss);
    _Noreturn void error(Keyword k);
    _Noreturn void errorMessage(const char* format, ...);
    int64_t extractArrayIndex(const NodeExpr* expr);

    void next();
    
    bool compareOptional(SpecialSymbolType ssType);
    bool compareKeywordOptional(Keyword k);
    bool compareOptionalExtractFollow();
    bool compareOptionalStartingKeywords(); 
    bool compareOptionalMath();
    bool compareOptionalComparison();
    bool compareOptionalOperation();
    bool compareOptionalRangeKeyword(); 
    void compare(SpecialSymbolType ssType);
    void compareToken(TokenType tt);
    void compareKeyword(Keyword k);
    bool compareTokenOptional(TokenType tt);
    bool compareOptionalGlobalExpressionStart();

    std::experimental::optional<const ExpressionType*> _getType(const std::string& str);

    void _entry();
    std::unique_ptr<NodeExpr> _entryPrime();
    void _fcnAttributes(std::set<FunctionAttr>& store);
    
    void _let(std::vector<std::unique_ptr<NodeExpr>>& store);
    void _letRepeat(std::vector<std::unique_ptr<NodeExpr>>& store);
    
    std::vector<std::unique_ptr<NodeExpr>>_newVariable(bool isGlobal);
    void _newVariablePrime(bool isGlobal, std::vector<std::unique_ptr<NodeExpr>>& store);
    void _newVariableRepeat(bool isGlobal, std::vector<std::unique_ptr<NodeExpr>>& store);
    std::tuple<std::vector<std::string>, const ExpressionType*> _d();
    std::tuple<std::experimental::optional<std::string>, std::experimental::optional<const ExpressionType*>> _dPrime();
    
    std::unique_ptr<NodeExpr> _exprSemicolon();
    std::unique_ptr<NodeExpr> _expr();
    std::unique_ptr<NodeExpr> _command(const std::string& expr);
    std::unique_ptr<AssignmentExpr> _assignment(std::unique_ptr<ValueAssignmentReferenceExpr> lhs);
    std::vector<std::unique_ptr<NodeExpr>>  _functionCall();
    std::vector<std::unique_ptr<NodeExpr>>  _functionParams();
    void _callParameters(std::vector<std::unique_ptr<NodeExpr>>& store);
    std::unique_ptr<NodeExpr> _k(bool consumeExpressionSemicolon);
    void _kSemicolon();
    std::vector<std::unique_ptr<NodeExpr>> _kPrime(std::vector<std::unique_ptr<NodeExpr>> exprs);
    const ExpressionType* _type();
    void _n();
    std::unique_ptr<ValueExpr> _subscript(std::unique_ptr<ValueExpr> value);
    std::unique_ptr<NodeExpr> _value();
    std::unique_ptr<NodeExpr> _call(const std::string& callee);
    std::unique_ptr<BlockExpr> _newScope();
    std::unique_ptr<BlockExpr> _scope();
    void _scopePrime();
    void _scopePrimePrime();
    void _scopeTerm();
    
    std::unique_ptr<NodeExpr> _if();
    std::tuple<std::unique_ptr<BlockExpr>, std::unique_ptr<BlockExpr>> _ifPrime();
    std::unique_ptr<BlockExpr> _ifPrimePrime();
    std::unique_ptr<BlockExpr> _else();
    std::unique_ptr<BlockExpr> _block(bool singleConsumeSemicolon);
    
    std::tuple<LoopMode, LoopRange> _range();
    std::unique_ptr<FunctionExpr> _functionDeclaration(std::unique_ptr<PrototypeExpr> prototype);
    void _parameters(std::vector<std::unique_ptr<ParameterDefinitionExpr>>&  collected);
    void _parametersPrime(std::vector<std::unique_ptr<ParameterDefinitionExpr>>&  collected);
    void _localVars(std::vector<std::unique_ptr<NodeExpr>>& store);
    
    std::unique_ptr<NodeExpr> _extract();
    std::unique_ptr<NodeExpr> _binary(std::unique_ptr<NodeExpr> left);
    
    std::unique_ptr<NodeExpr> _comparison();
    std::unique_ptr<NodeExpr> _comparisonPrime(std::unique_ptr<NodeExpr> left);
    void _cmp();
    
    std::unique_ptr<NodeExpr> _operation();
    std::unique_ptr<NodeExpr> _operationPrime(std::unique_ptr<NodeExpr> left);
    
    void _comparisonSign();
    void _sign();
    std::unique_ptr<NodeExpr> _tExtract();
    std::unique_ptr<NodeExpr> _tExtractPrime(std::unique_ptr<NodeExpr> left);
    std::unique_ptr<NodeExpr> _fExtract();
    std::unique_ptr<NodeExpr> _fExtractPrime();
    
    void _case(SwitchExpr& switchExpr);
    void _casePrime(SwitchExpr& switchExpr);

    int dumpCode(const std::string& outputName, const std::string& srcFile, bool dumpIR, bool optimization);
    
public:
    Parser(FILE* input): input(input) {
        
    }
    
    int generateCode(const std::string& outputname, const std::string& sourceFileName, bool dumpIR, bool dumpAST, bool optimization);
};


#endif /* parser_h */
