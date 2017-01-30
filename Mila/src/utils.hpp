//
//  utils.hpp
//  Mila
//
//  Created by Damian Malarczyk on 14.12.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>
#include <iostream>
#include <cstdio>
#include "ast.hpp"

class ASTDump {
public:
    ASTDump(std::ostream& os) : os_(os), curIndentLevel_(0) {}
    void increaseLevel() { ++curIndentLevel_; }
    void decreaseLevel() { --curIndentLevel_; }
    ASTDump& operator++();
    ASTDump& operator--();
    
    void dump(const std::string &name, const AST::NodeExpr& expr, const AST::ASTContext& ctx);
    
private:
    template<typename T> friend std::ostream& operator<<(ASTDump&, T);
    
    std::ostream& os_;
    int curIndentLevel_;
};

template<typename T>
std::ostream& operator<<(ASTDump& log, T op) {
    for(int i = 0; i < log.curIndentLevel_ * 2; ++i) {
        log.os_ << ' ';
    }
    log.os_ << op;
    return log.os_;
}

extern ASTDump astDump;

std::string hexToString(const std::string& input);
_Noreturn void formattedError(const char* src, ...);


template<typename T>
class Range {
    class Iterator {
        T val;
    public:

        Iterator(T val): val(val) {};
        void operator++() {
            val++;
        }
        
        const T& operator*() const {
            return val;
        }
        
        bool operator !=(const Iterator& other) const {
            return val <= other.val;
        }
    };
    
    T startVal;
    T endVal;
public:
    
    Iterator begin() const {
        return Iterator(startVal);
    }
    
    Iterator end() const {
        return Iterator(endVal);
    }
    
    Range(T start, T end): startVal(start), endVal(end) {
        
    }
    
    bool contains(T val) const {
        return val >= startVal && val <= endVal;
    }
    
    bool overlapsWith(const Range<T>& other) const {
        return (contains(other.startVal) || contains(other.endVal));
    }
    
    bool verify() {
        return startVal <= endVal; 
    }
    
};
#endif /* utils_hpp */
