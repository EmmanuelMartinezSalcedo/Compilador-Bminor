#ifndef ERROR_DETECTION_H
#define ERROR_DETECTION_H

#include <iostream>
#include <stdlib.h>

#include <string>
#include <fstream>
#include <utility>
#include <cctype>
#include <algorithm>

#include <tuple>
#include <stack>
#include <vector>
#include <map>


using namespace std; 

stack<string> token_stack;
vector<int> error_pos_detected;


struct Error{
    string message;
    string token;
    int line;
    int column;
};

class ErrorDetection {
    private:
        vector<Error> errors;
    public:
        
        void addError(const string& message, const string& token, int line, int column) {
            errors.push_back({message, token, line, column});
        }

        const  vector<Error>& getErrors() const {
            return errors;
        }
        
};
#endif