#include <iostream>
#include <cstdlib>

extern "C" {
    void writeln(int64_t i) {
        printf("%lld\n", i);
    }
    void write(int64_t i) {
        printf("%lld", i);
    }
    
    void readln(int64_t* ptr) {
        std::string buff;
        std::cin >> buff;
        *ptr = std::stoi(buff);
    }
    
    void inc(int64_t* ptr) {
        (*ptr)++;
    }
    
    void dec(int64_t* ptr) {
        (*ptr)--;
    }

    void print(char* ptr) {
        std::cout << ptr;
    }

    void echo(char* ptr) {
        std::cout << ptr << std::endl;
    }
    void newLine() {
        std::cout << std::endl;
    }

    _Noreturn 
    void _exit(int64_t i) {
        exit(i);
    }

    _Noreturn 
    void _exitMessage(int64_t i, char* ptr) {
        echo(ptr);
        exit(i);
    }

}