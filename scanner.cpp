#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <cctype>
#include <algorithm>
#include <tuple>
#include <string.h>
#include <string>
using namespace std;

int mostrar_solo_errores = 0;

vector<tuple<string,string,int,int>> errores_encontrados;

class Scanner {
public:
    Scanner() {
        keywords = {"array", "boolean", "char", "else", "false", "for", "function", "if", 
                    "integer","map", "print", "return", "string", "true", "void", "while"};
        symbols = {",", ";", ":", "(", ")", "[", "]", "{", "}","\"","\'"};
        operators = {"++", "--", "+", "-", "*", "/", "%", "^", "&&", "||", "!", "=", "<", ">", "<=", ">=", "==", "!="};
    }

    ~Scanner() {}

    vector<vector<char>> readBMMFile(const string& filename) {
        vector<vector<char>> data;
        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "Error al abrir el archivo: " << filename << endl;
            return data;
        }

        string line;
        while (getline(file, line)) {
            vector<char> row(line.begin(), line.end());
            row.push_back('\n');
            data.push_back(row);
        }

        file.close();
        return data;
    }

    char getchar(vector<vector<char>>& buffer, int& row, int& col) {
        char c = buffer[row][col];
        col++;
        if (col >= buffer[row].size()) {
            row++;
            col = 0;
        }
        return c;
    }

    char peekchar(vector<vector<char>>& buffer, int row, int col) {
        if (row >= buffer.size() || col >= buffer[row].size()) return '\0';
        return buffer[row][col];
    }

    bool isSymbol(char c) {
        return find(symbols.begin(), symbols.end(), string(1, c)) != symbols.end();
    }

    bool isOperator(string str) {
        return find(operators.begin(), operators.end(), str) != operators.end();
    }

    pair<string, string> gettoken(vector<vector<char>>& buffer, int& row, int& col) {
        if (row >= buffer.size()) return {"EOF", ""};

        char c = getchar(buffer, row, col);

        // Ignorar espacios en blanco
        while (isspace(c)) {
            if (row >= buffer.size()) return {"EOF", ""};
            c = getchar(buffer, row, col);
        }

        // Verificar comentarios
        if (c == '/') {
            char next = peekchar(buffer, row, col);
            if (next == '/') {
                // Comentario de una línea
                while (c != '\n' && row < buffer.size()) {
                    c = getchar(buffer, row, col);
                }
                return {"COMMENT_LINE", "//"};
            } else if (next == '*') {
                // Comentario de múltiples líneas
                getchar(buffer, row, col); // avanzar
                while (!(c == '*' && peekchar(buffer, row, col) == '/') && row < buffer.size()) {
                    c = getchar(buffer, row, col);
                }

                if(c != '*' && peekchar(buffer,row,col) != '/')
                {
                    errores_encontrados.push_back(tuple("Error: No se cerro el comentario: '","/*",row+1,col+1));
                    return {"ERROR","/*"};
                }
                getchar(buffer, row, col); // avanzar para cerrar el comentario
                return {"COMMENT_LARGE", "/* */"};
            }
        }

        int tokenStartRow = row;
        int tokenStartCol = col - 1; // Columna inicial del token

        // Verificar identificadores y palabras clave
        if (isalpha(c) || c == '_') {
            string value(1, c);
            while (isalnum(peekchar(buffer, row, col)) || peekchar(buffer, row, col) == '_') {
                value += getchar(buffer, row, col);
            }
            if (find(keywords.begin(), keywords.end(), value) != keywords.end()) {
                if(value == "array")
                {
                    return {"ARRAY",value};
                }
                else if(value =="boolean")
                {
                    return {"BOOL_TYPE",value};
                }
                else if(value =="char")
                {
                    return {"CHAR_TYPE",value};
                }
                else if(value =="else")
                {
                    return {"ELSE",value};
                }
                else if(value =="false")
                {
                    return {"FALSE",value};
                }
                else if(value =="for")
                {
                    return {"FOR",value};
                }
                else if(value =="function")
                {
                    return {"FUNCTION",value};
                }
                else if(value =="if")
                {
                    return {"IF",value};
                }
                else if(value =="integer")
                {
                    return {"INT_TYPE",value};
                }
                else if(value =="map")
                {
                    return {"MAP",value};
                }
                else if(value =="print")
                {
                    return {"PRINT",value};
                }
                else if(value =="return")
                {
                    return {"RETURN",value};
                }
                else if(value =="string")
                {
                    return {"STRING_TYPE",value};
                }
                else if(value =="true")
                {
                    return {"TRUE",value};
                }
                else if(value =="void")
                {
                    return {"VOID",value};
                }
                else if(value =="while")
                {
                    return {"WHILE",value};
                }
                return {"KEYWORD", value};
            } else {
                if(value.size()>256)
                {
                    errores_encontrados.push_back(tuple("Error: Identificador fuera de rango: '",value,row+1,col+1));
                    return{"ERROR",value};
                }
                return {"IDENTIFIER", value};
            }
        }
        // Verificar caracteres (char)
        if (c == '\'') {
            string value(1, c);
            c = getchar(buffer, row, col);
            value += c;

            // Revisar si es un carácter válido o un carácter de escape
            if (c == '\\') {
                c = getchar(buffer, row, col); // Capturar el carácter escapado
                value += c;
            }

            c = getchar(buffer, row, col);
            if (c == '\'') {
                value += c;
                return {"CHAR", value}; // Se retorna el token de tipo char
            } else {
                errores_encontrados.push_back(tuple("Error: no se cerro el char: '",value,row+1,col+1));
                return {"ERROR", value}; // Error si no se cierra con comilla simple
            }
        }
        // Verificar cadenas (string)
        if (c == '"') {
            string value(1, c);
            c = getchar(buffer, row, col);

            while (c != '"' && row < buffer.size()) {
                value += c;
                c = getchar(buffer, row, col);
            }

            if (c == '"') {
                value += c;
                return {"STRING", value}; // Retorna el token de tipo string
            } else {
                errores_encontrados.push_back(tuple("Error: no se cerro el la cadena: '",value,row+1,col+1));
                return {"ERROR", value}; // Error si no se cierra con comillas dobles
            }
        }
        // Verificar enteros
        if (isdigit(c)) {
            string value(1, c);
            while (isdigit(peekchar(buffer, row, col))) {
                value += getchar(buffer, row, col);
            }
            if(isalpha(peekchar(buffer,row,col)))
            {
                while (isalnum(peekchar(buffer, row, col)) || peekchar(buffer, row, col) == '_') {
                value += getchar(buffer, row, col);
                }
                errores_encontrados.push_back(tuple("Error: Identificador iniciado con int:  '",value,row+1,col+1));
                return{"ERROR",value};
            }
            if(value.size()>19)
            {
                errores_encontrados.push_back(tuple("Error: Entero fuera de rango: '",value,row+1,col+1));
                return {"ERROR",value};
            }
            else if( value.size() ==19 )  //valor maximo para int es de 2^63 - 1   9,223,372,036,854,775,807
            {

                int part1 = stoi(value.substr(0,9));
                int part2 = stoi(value.substr(10));
                if (part1 <= 922337203 && part2<=854775807)
                {
                    return {"INTEGER", value};
                }
                else{
                    errores_encontrados.push_back(tuple("Error: Entero fuera de rango: '",value,row+1,col+1));
                    return {"ERROR",value};
                }
            }
            else{
                return {"INTEGER",value};
            }
            
        }

        // Verificar operadores (incluyendo operadores de múltiples caracteres)
        string op(1, c);
        if (isOperator(op)) {
            char next = peekchar(buffer, row, col);
            string potentialOp = op + next;
            if (isOperator(potentialOp)) {
                getchar(buffer, row, col); // avanzar al siguiente carácter
                if(potentialOp == "++")
                {
                    return {"PLUSPLUS",potentialOp};
                }
                else if(potentialOp == "--")
                {
                    return {"MINUS_MINUS",potentialOp};
                }
                else if(potentialOp == "&&")
                {
                    return {"AND",potentialOp};
                }
                else if(potentialOp == "||")
                {
                    return {"OR",potentialOp};
                }
                else if(potentialOp == "<=")
                {
                    return {"MINOR_EQUAL",potentialOp};
                }
                else if(potentialOp == ">=")
                {
                    return {"MAJOR_EQUAL",potentialOp};
                }
                else if(potentialOp == "==")
                {
                    return {"EQUAL_EQUAL",potentialOp};
                }
                else if(potentialOp == "!=")
                {
                    return {"NOT_EQUAL",potentialOp};
                }
            }
            if(op =="+")
            {
                return {"PLUS",op};
            }
            else if(op == "-")
            {
                return {"MINUS",op};
            }
            else if(op == "*")
            {
                return {"TIMES",op};
            }
            else if(op == "/")
            {
                return {"DIV",op};
            }
            else if(op == "%")
            {
                return {"MOD",op};
            }
            else if(op == "^")
            {
                return {"POW",op};
            }
            else if(op == "=")
            {
                return {"EQUAL",op};
            }
            else if(op == "<")
            {
                return {"MINOR",op};
            }
            else if(op == ">")
            {
                return {"MAYOR",op};
            }
            return {"OPERATOR", op};
        }

        // Verificar símbolos
        string symbol(1, c);
        if (isSymbol(c)) {
            if(symbol == ",")
            {
                return {"COMMA",symbol};
            }
            else if(symbol ==";")
            {
                return {"SEMICOLON",symbol};
            }
            else if(symbol ==":")
            {
                return {"COLON",symbol};
            }
            else if(symbol =="(")
            {
                return {"L_PARENTHESIS",symbol};
            }
            else if(symbol ==")")
            {
                return {"R_PARENTHESIS",symbol};
            }
            else if(symbol =="[")
            {
                return {"L_BRACKET",symbol};
            }
            else if(symbol =="]")
            {
                return {"R_BRACKET",symbol};
            }
            else if(symbol =="{")
            {
                return {"L_BRACES",symbol};
            }
            else if(symbol =="}")
            {
                return {"R_BRACES",symbol};
            }
            
        }

        // Si el carácter es inválido, registrar error
        errores_encontrados.push_back(tuple("Error: Caracter no valido: '",string(1,c),row+1,col+1));
        return {"ERROR", string(1, c)};
    }

    vector<tuple<string, string, int, int>> Tokenize(vector<vector<char>>& buffer) {
        int row = 0, col = 0;
        vector<tuple<string, string, int, int>> tokens;

        while (row < buffer.size()) {
            int startRow = row;
            int startCol = col;
            pair<string, string> token = gettoken(buffer, row, col);
            if (token.first == "ERROR" || token.first == "COMMENT_LINE" || token.first =="COMMENT_LARGE") {
                continue;
            }

            if (token.first == "EOF") {
                //tokens.push_back(make_tuple("EOF", "", startRow, startCol));
                break;
            }
            
            tokens.push_back(make_tuple(token.first, token.second, startRow, startCol));

            if(mostrar_solo_errores == 1)
            {
                continue;
            }

            else{
                printf("Token: %-20s Value: %-20s Fila: %-20d Columna: %-20d\n", 
       token.first.c_str(), token.second.c_str(), startRow + 1, startCol + 1);
            }
        }

        return tokens;
    }

private:
    vector<string> keywords;
    vector<string> symbols;
    vector<string> operators;
};



class Parser {
public:

    vector<tuple<string, string, int, int>> tokens;
    const char* cursor;
    int index;
    bool debug;

    Parser(vector<tuple<string, string, int, int>>& tkns, bool dbg) {
        tokens = tkns;
        cursor = get<1>(tokens[0]).c_str();
        index = 0;
        debug = dbg;

        if (PROGRAM()) {
            cout << "Successful parse" << endl;
        } else {
            cout << "Error parsing in line " << get<2>(tokens[index]) << ", column " << get<3>(tokens[index]) << endl;
        }
    }

private:

    bool checkToken(const string& token) {
        int tokenSize = token.size();

        for (int i = 0; i < tokenSize; i++) {
            if (cursor[i] == token[i]) cout << cursor[i] << " = " << token[i] << endl;
            if (cursor[i] != token[i]) {
                return false;
            }
        }
        return true;
    }

    void goNextToken() {
        if (index + 1 < tokens.size()) {
            index++;
            cursor = get<1>(tokens[index]).c_str(); 
        }
    }

    void goBackToken() {
        if (index - 1 >= 0) {
            index--;
            cursor = get<1>(tokens[index]).c_str(); 
        }
    }

    bool PROGRAM() {
        if (debug) cout << "PROGRAM -> DECLARATION PROGRAM_REST" << endl;
        cout << endl;
        if (DECLARATION()) {
        /* PROGRAM -> DECLARATION PROGRAM_REST */
            if (PROGRAM_REST()) {
                return true;
            }
        }
        return false;
    }

    bool PROGRAM_REST() {
        if (debug) cout << "PROGRAM_REST -> DECLARATION PROGRAM_REST" << endl;
        if (debug) cout << "PROGRAM_REST -> EOP" << endl;
        cout << endl;
        if (DECLARATION()) {
        /* PROGRAM_REST -> DECLARATION PROGRAM_REST*/
            if (PROGRAM_REST()) {
                return true;
            }
        } else if (EOP()) {
            /* PROGRAM_REST -> EOP */
            return true;
        }
        return false;
    }

    bool DECLARATION() {
        if (debug) cout << "DECLARATION -> FUNCTION" << endl;
        if (debug) cout << "DECLARATION -> VAR_DECL" << endl;
        cout << endl;
        if (VAR_DECL()) {
        /* DECLARATION -> FUNCTION */
            return true;
        } else if (FUNCTION()) {
        /* DECLARATION -> VAR_DECL */
            return true;
        }
        return false;
    }
    
    bool FUNCTION() {
        if (debug) cout << "FUNCTION -> FUNCTION_REST (PARAMS) { STMT_LIST }" << endl;
        cout << endl;
        if (FUNCTION_REST()) {
        /* FUNCTION -> FUNCTION_REST (PARAMS) { STMT_LIST } */
            if (checkToken("(")) {
                goNextToken();
                if (PARAMS()) {
                    if (checkToken(")")) {
                        goNextToken();
                        if (checkToken("{")) {
                            goNextToken();
                            if (STMT_LIST()) {
                                if (checkToken("}")) {
                                    goNextToken();
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    bool FUNCTION_REST() {
        if (debug) cout << "FUNCTION_REST -> TYPE IDENTIFIER" << endl;
        cout << endl;
        if (TYPE()) {
        /* FUNCTION_REST -> TYPE IDENTIFIER */
            if (IDENTIFIER()) {
                return true;
            }
        }
        return false;
    }

    bool TYPE() {
        if (debug) cout << "TYPE -> INT_TYPE TYPE_REST" << endl;
        if (debug) cout << "TYPE -> BOOL_TYPE TYPE_REST" << endl;
        if (debug) cout << "TYPE -> CHAR_TYPE TYPE_REST" << endl;
        if (debug) cout << "TYPE -> STRING_TYPE TYPE_REST" << endl;
        if (debug) cout << "TYPE -> VOID_TYPE TYPE_REST" << endl;
        cout << endl;
        if (INT_TYPE()) {
        /* TYPE -> INT_TYPE TYPE_REST */
            if (TYPE_REST()) {
                return true;
            }
        } else if (BOOL_TYPE()) {
        /* TYPE -> BOOL_TYPE TYPE_REST */
            if (TYPE_REST()) {
                return true;
            }
        } else if (CHAR_TYPE()) {
        /* TYPE -> CHAR_TYPE TYPE_REST*/
            if (TYPE_REST()) {
                return true;
            }
        } else if (STRING_TYPE()) {
        /* TYPE -> STRING_TYPE TYPE_REST*/
            if (TYPE_REST()) {
                return true;
            }
        } else if (VOID_TYPE()) {
        /* TYPE -> VOID_TYPE TYPE_REST */
            if (TYPE_REST()) {
                return true;
            }
        }
        return false;
    }

    bool TYPE_REST() {
        if (debug) cout << "TYPE_REST -> [ ] TYPE_REST" << endl;
        if (debug) cout << "TYPE_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("[")) {
        /* TYPE_REST -> [ ] TYPE_REST */  
            goNextToken();
            if (checkToken("]")) {
                goNextToken();
                if (TYPE_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
        /* TYPE_REST -> EOP */
            return true;
        }
        return false;
    }

    bool PARAMS() {
        if (debug) cout << "PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST" << endl;
        if (debug) cout << "PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST" << endl;
        if (debug) cout << "PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST" << endl;
        if (debug) cout << "PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST" << endl;
        if (debug) cout << "PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST" << endl;
        if (debug) cout << "PARAMS -> EOP" << endl;
        cout << endl;
        if (INT_TYPE()) {
        /* PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else if (BOOL_TYPE()) {
        /* PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else if (CHAR_TYPE()) {
        /* PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else if (STRING_TYPE()) {
        /* PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else if (VOID_TYPE()) {
        /* PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else if (EOP()) {
        /* PARAMS -> EOP */
            return true;
        }
        return false;
    }

    bool PARAMS_REST() {
        if (debug) cout << "PARAMS_REST -> , PARAMS" << endl;
        if (debug) cout << "PARAMS_REST -> EOP" << endl;
        cout << endl;
        if (checkToken(",")) {
        /* PARAMS_REST -> , PARAMS */
            goNextToken();
            if (PARAMS()) {
                return true;
            }
        } else if (EOP()) {
        /* PARAMS_REST -> EOP  */
            return true;
        }
        return false;
    }

    bool VAR_DECL() {
        if (debug) cout << "VAR_DECL-> FUNCTION_REST VAR_DECL_REST" << endl;
        cout << endl;
        if (FUNCTION_REST()) {
        /* VAR_DECL-> FUNCTION_REST VAR_DECL_REST */
            if (VAR_DECL_REST()) {
                return true;
            }
        }
        return false;
    }

    bool VAR_DECL_REST() {
        if (debug) cout << "VAR_DECL_REST -> ;" << endl;
        if (debug) cout << "VAR_DECL_REST -> = EXPRESSION ;" << endl;
        cout << endl;
        if (checkToken(";")) {
        /* VAR_DECL_REST -> ; */
            goNextToken();
            return true;
        } else if (checkToken("=")) {
        /* VAR_DECL_REST -> = EXPRESSION ; */
            goNextToken();
            if (EXPRESSION()) {
                if (checkToken(";")) {
                    goNextToken();
                    return true;
                }
            }
        }
        return false;
    }

    bool STMT_LIST() {
        if (debug) cout << "STMT_LIST -> STATEMENT STMT_LIST_REST" << endl;
        cout << endl;
        if (STATEMENT()) {
        /* STMT_LIST -> STATEMENT STMT_LIST_REST */
            if (STMT_LIST_REST()) {
                return true;
            }
        }
        return false;
    };

    bool STMT_LIST_REST() {
        if (debug) cout << "STMT_LIST_REST -> STATEMENT STMT_LIST_REST" << endl;
        if (debug) cout << "STMT_LIST_REST -> EOP" << endl;
        cout << endl;
        if (STATEMENT()) {
        /* STMT_LIST_REST -> STATEMENT STMT_LIST_REST */
            if (STMT_LIST_REST()) {
                return true;
            }
        } else if (EOP()) {
        /* STMT_LIST_REST -> EOP */
            return true;
        }
        return false;
    }

    bool STATEMENT() {
        if (debug) cout << "STATEMENT -> VAR_DECL" << endl;
        if (debug) cout << "STATEMENT -> IF_STMT" << endl;
        if (debug) cout << "STATEMENT -> FOR_STMT" << endl;
        if (debug) cout << "STATEMENT -> RETURN_STMT" << endl;
        if (debug) cout << "STATEMENT -> EXPR_STMT" << endl;
        if (debug) cout << "STATEMENT -> PRINT_STMT" << endl;
        if (debug) cout << "STATEMENT -> { STMT_LIST }" << endl;
        cout << endl;
        if (VAR_DECL()) {
        /* STATEMENT -> VAR_DECL */
            return true;
        } else if (IF_STMT()) {
        /* STATEMENT -> IF_STMT */
            return true;
        } else if (FOR_STMT()) {
        /* STATEMENT -> FOR_STMT */
            return true;
        } else if (RETURN_STMT()) {
        /* STATEMENT -> RETURN_STMT */
            return true;
        } else if (EXPR_STMT()) {
        /* STATEMENT -> EXPR_STMT */
            return true;
        } else if (PRINT_STMT()) {
        /* STATEMENT -> PRINT_STMT */
            return true;
        } else if (checkToken("{")) {
        /* STATEMENT -> { STMT_LIST } */
            goNextToken();
            if (STMT_LIST()) {
                if (checkToken("}")) {
                    goNextToken();
                    return true;
                }
            }
        }
        return false;
    }

    bool IF_STMT() {
        if (debug) cout << "IF_STMT -> if ( EXPRESSION ) { STATEMENT } IF_STMT_REST" << endl;
        cout << endl;
        if (checkToken("if")) {
        /* IF_STMT -> if ( EXPRESSION ) { STATEMENT } IF_STMT_REST */
            goNextToken();
            if (checkToken("(")) {
                goNextToken();
                if (EXPRESSION()) {
                    if (checkToken(")")) {
                        goNextToken();
                        if (checkToken("{")) {
                            goNextToken();
                            if (STATEMENT()) {
                                if (checkToken("}")) {
                                    goNextToken();
                                    if (STATEMENT()) {
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    bool IF_STMT_REST() {
        if (debug) cout << "IF_STMT_REST -> else { STATEMENT }" << endl;
        if (debug) cout << "IF_STMT_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("else")) {
        /* IF_STMT_REST -> else { STATEMENT } */
            goNextToken();
            if (checkToken("{")) {
                goNextToken();
                if (STATEMENT()) {
                    if (checkToken("}")) {
                        return true;
                    }
                }
            }
        } else if (EOP()) {
            /* IF_STMT_REST -> EOP */
            return true;
        }
        return false;
    }

    bool FOR_STMT() {
        if (debug) cout << "FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STATEMENT }" << endl;
        cout << endl;
        if (checkToken("for")) {
        /* FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STATEMENT } */
            goNextToken();
            if (checkToken("(")) {
                goNextToken();
                if (EXPR_STMT()) {
                    if (EXPRESSION()) {
                        if (checkToken(";")) {
                            goNextToken();
                            if (EXPR_STMT()) {
                                if (checkToken(")")) {
                                    goNextToken();
                                    if (checkToken("{")) {
                                        goNextToken();
                                        if (STATEMENT()) {
                                            if (checkToken("}")) {
                                                goNextToken();
                                                return true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    bool RETURN_STMT() {
        if (debug) cout << "RETURN_STMT -> return EXPRESSION ;" << endl;
        cout << endl;
        if (checkToken("return")) {
        /* RETURN_STMT -> return EXPRESSION ; */
            goNextToken();
            if (EXPRESSION()) {
                if (checkToken(";")) {
                    return true;
                }
            }
        }
        return false;
    }

    bool PRINT_STMT() {
        if (debug) cout << "PRINT_STMT -> print ( EXPR_LIST ) ;" << endl;
        cout << endl;
        if (checkToken("print")) {
        /* PRINT_STMT -> print ( EXPR_LIST ) ; */
            goNextToken();
            if (checkToken("(")) {
                goNextToken();
                if (EXPR_LIST()) {
                    if (checkToken(")")) {
                        goNextToken();
                        if (checkToken(";")) {
                            goNextToken();
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool EXPR_STMT() {
        if (debug) cout << "EXPR_STMT -> EXPRESSION ;" << endl;
        if (debug) cout << "EXPR_STMT -> ;" << endl;
        cout << endl;
        if (EXPRESSION()) {
        /* EXPR_STMT -> EXPRESSION ; */
            if (checkToken(";")) {
                goNextToken();
                return true;
            }
        } else if (checkToken(";")) {
        /* EXPR_STMT -> ; */
            goNextToken();
            return true;
        }
        return false;
    }

    bool EXPR_LIST() {
        if (debug) cout << "EXPR_LIST -> EXPRESSION EXPR_LIST_REST" << endl;
        if (debug) cout << "EXPR_LIST_REST -> , EXPR_LIST" << endl;
        if (debug) cout << "EXPR_LIST_REST -> EOP" << endl;
        cout << endl;
        if (EXPRESSION()) {
        /* EXPR_LIST -> EXPRESSION EXPR_LIST_REST */
            if (EXPR_LIST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool EXPR_LIST_REST() {
        if (checkToken(",")) {
        /* EXPR_LIST_REST -> , EXPR_LIST */
            goNextToken();
            if (EXPR_LIST()) {
                return true;
            }
        } else if (EOP()) {
        /* EXPR_LIST_REST -> EOP */
            return true;
        }
        return false;
    }

    bool EXPRESSION() {
        if (debug) cout << "EXPRESSION -> IDENTIFIER = EXPRESSION" << endl;
        if (debug) cout << "EXPRESSION -> OR_EXPR" << endl;
        cout << endl;
        if (IDENTIFIER()) {
        /* EXPRESSION -> IDENTIFIER = EXPRESSION */
            if (checkToken("=")) {
                goNextToken();
                if (EXPRESSION()) {
                    return true;
                }
            }
        } else if (OR_EXPR()) {
        /* EXPRESSION -> OR_EXPR */
            return true;
        }
        return false;
    }

    bool OR_EXPR() {
        if (debug) cout << "OR_EXPR -> AND_EXPR OR_EXPR_REST" << endl;
        cout << endl;
        if (AND_EXPR()) {
        /* OR_EXPR -> AND_EXPR OR_EXPR_REST */
            if (OR_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }

    bool OR_EXPR_REST() {
        if (debug) cout << "OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST" << endl;
        if (debug) cout << "OR_EXPR_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("||")) {
        /* OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST */
            goNextToken();
            if (AND_EXPR()) {
                if (OR_EXPR_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
        /* OR_EXPR_REST -> EOP */
            return true;
        }
        return false;
    }

    bool AND_EXPR() {
        if (debug) cout << "AND_EXPR -> EQ_EXPR AND_EXPR_REST" << endl;
        cout << endl;
        if (EQ_EXPR()) {
        /* AND_EXPR -> EQ_EXPR AND_EXPR_REST */
            if (AND_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }
    
    bool AND_EXPR_REST() {
        if (debug) cout << "AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST" << endl;
        if (debug) cout << "AND_EXPR_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("&&")) {
        /* AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST */
            goNextToken();
            if (EQ_EXPR()) {
                if (AND_EXPR_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
        /* AND_EXPR_REST -> EOP */
            return true;
        }
        return false;
    }

    bool EQ_EXPR() {
        if (debug) cout << "EQ_EXPR -> REL_EXPR EQ_EXPR_REST" << endl;
        cout << endl;
        if (REL_EXPR()) {
        /* EQ_EXPR -> REL_EXPR EQ_EXPR_REST */
            if (EQ_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }

    bool EQ_EXPR_REST() {
        if (debug) cout << "EQ_EXPR_REST -> == REL_EXPR EQ_EXPR_REST" << endl;
        if (debug) cout << "EQ_EXPR_REST -> != REL_EXPR EQ_EXPR_REST" << endl;
        if (debug) cout << "EQ_EXPR_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("==")) {
        /* EQ_EXPR_REST -> == REL_EXPR EQ_EXPR_REST */
            goNextToken();
            if (REL_EXPR()) {
                if (EQ_EXPR_REST()) {
                    return true;
                }
            }
        } else if (checkToken("!=")) {
        /* EQ_EXPR_REST -> != REL_EXPR EQ_EXPR_REST */  
            goNextToken();
            if (REL_EXPR()) {
                if (EQ_EXPR_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
        /* EQ_EXPR_REST -> EOP */
            return true;
        }
        return false;
    }

    bool REL_EXPR() {
        if (debug) cout << "REL_EXPR -> TERM REL_EXPR_REST" << endl;
        cout << endl;
        if (TERM()) {
        /* REL_EXPR -> TERM REL_EXPR_REST */
            if (REL_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }

    bool REL_EXPR_REST() {
        if (debug) cout << "REL_EXPR_REST -> < TERM REL_EXPR_REST" << endl;
        if (debug) cout << "REL_EXPR_REST -> > TERM REL_EXPR_REST" << endl;
        if (debug) cout << "REL_EXPR_REST -> <= TERM REL_EXPR_REST" << endl;
        if (debug) cout << "REL_EXPR_REST -> >= TERM REL_EXPR_REST" << endl;
        cout << endl;
        if (checkToken("<")) {
        /* REL_EXPR_REST -> < TERM REL_EXPR_REST */   
            if (TERM()) {
                if (REL_EXPR_REST()) {
                    return true;
                }
            }
        } else if (checkToken(">")) {
        /* REL_EXPR_REST -> > TERM REL_EXPR_REST */
            if (TERM()) {
                if (REL_EXPR_REST()) {
                    return true;
                }
            }
        } else if (checkToken("<=")) {
        /* REL_EXPR_REST -> <= TERM REL_EXPR_REST */
            if (TERM()) {
                if (REL_EXPR_REST()) {
                    return true;
                }
            }
        } else if (checkToken(">=")) {
        /* REL_EXPR_REST -> >= TERM REL_EXPR_REST */
            if (TERM()) {
                if (REL_EXPR_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
            return true;
        }
        return false;
    }

    bool TERM() {
        if (debug) cout << "TERM -> UNARY TERM_REST" << endl;
        cout << endl;
        if (UNARY()) {
        /* TERM -> UNARY TERM_REST */
            if (TERM_REST()) {
                return true;
            }
        }
        return false;
    }

    bool TERM_REST() {
        if (debug) cout << "TERM_REST -> * UNARY TERM_REST" << endl;
        if (debug) cout << "TERM_REST -> / UNARY TERM_REST" << endl;
        if (debug) cout << "TERM_REST -> % UNARY TERM_REST" << endl;
        if (checkToken("*")) {
        /* TERM_REST -> * UNARY TERM_REST */
            goNextToken();
            if (UNARY()) {
                if (TERM_REST()) {
                    return true;
                }
            }
        } else if (checkToken("/")) {
        /* TERM_REST -> / UNARY TERM_REST */
            goNextToken();
            if (UNARY()) {
                if (TERM_REST()) {
                    return true;
                }
            }
        } else if (checkToken("%")) {
        /* TERM_REST -> % UNARY TERM_REST */
            goNextToken();
            if (UNARY()) {
                if (TERM_REST()) {
                    return true;
                }
            }
        } else if (EOP()) {
            return true;
        }
        return false;
    }

    bool UNARY() {
        if (debug) cout << "UNARY -> ! UNARY" << endl;
        if (debug) cout << "UNARY -> - UNARY" << endl;
        if (debug) cout << "UNARY -> FACTOR" << endl;
        cout << endl;
        if (checkToken("!")) {
        /* UNARY -> ! UNARY */
            goNextToken();
            if (UNARY()) {
                return true;
            }
        } else if (checkToken("-")) {
        /* UNARY -> - UNARY */
            goNextToken();
            if (UNARY()) {
                return true;
            }
        } else if (FACTOR()) {
        /* UNARY -> FACTOR */
            return true;
        }
        return false;
    }

    bool FACTOR() {
        if (debug) cout << "FACTOR -> IDENTIFIER FACTOR_REST" << endl;
        if (debug) cout << "FACTOR -> INT_LITERAL FACTOR_REST" << endl;
        if (debug) cout << "FACTOR -> CHAR_LITERAL FACTOR_REST" << endl;
        if (debug) cout << "FACTOR -> STRING_LITERAL FACTOR_REST" << endl;
        if (debug) cout << "FACTOR -> BOOL_LITERAL FACTOR_REST" << endl;
        if (debug) cout << "FACTOR -> ( EXPRESSION ) FACTOR_REST" << endl;
        cout << endl;
        if (IDENTIFIER()) {
        /* FACTOR -> IDENTIFIER FACTOR_REST */
            if (FACTOR_REST()) {
                return true;
            }
        } else if (INT_LITERAL()) {
        /* FACTOR -> INT_LITERAL FACTOR_REST */
            if (FACTOR_REST()) {
                return true;
            }
        } else if (CHAR_LITERAL()) {
        /* FACTOR -> CHAR_LITERAL FACTOR_REST */
            if (FACTOR_REST()) {
                return true;
            }
        } else if (STRING_LITERAL()) {
        /* FACTOR -> STRING_LITERAL FACTOR_REST */
            if (FACTOR_REST()) {
                return true;
            }
        } else if (BOOL_LITERAL()) {
        /* FACTOR -> BOOL_LITERAL FACTOR_REST */
            if (FACTOR_REST()) {
                return true;
            }
        } else if (checkToken("(")) {
        /* FACTOR -> ( EXPRESSION ) FACTOR_REST */
            goNextToken();
            if (EXPRESSION()) {
                if (checkToken(")")) {
                    goNextToken();
                    if (FACTOR_REST()) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool FACTOR_REST() {
        if (debug) cout << "FACTOR_REST -> [ EXPRESSION ] FACTOR_REST" << endl;
        if (debug) cout << "FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST" << endl;
        if (debug) cout << "FACTOR_REST -> EOP" << endl;
        cout << endl;
        if (checkToken("[")) {
        /* FACTOR_REST -> [ EXPRESSION ] FACTOR_REST */
            goNextToken();
            if (EXPRESSION()) {
                if (checkToken("]")) {
                    goNextToken();
                    if (FACTOR_REST()) {
                        return true;
                    }
                }
            }
        } else if (checkToken("(")) {
        /* FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST */
            goNextToken();
            if (EXPR_LIST()) {
                if (checkToken(")")) {
                    goNextToken();
                    if (FACTOR_REST()) {
                        return true;
                    }
                }
            }
        } else if (EOP()) {
        /* FACTOR_REST -> EOP */
            return true;
        }
        return false;
    }

    bool EOP() {
        /* EOP -> '' */
        return true;
    }

    bool IDENTIFIER() {
        if (debug) cout << "IDENTIFIER -> LETTER IDENTIFIER_REST" << endl;
        if (debug) cout << "IDENTIFIER -> _ IDENTIFIER_REST" << endl;
        cout << endl;
        if (LETTER()) {
        /* IDENTIFIER -> LETTER IDENTIFIER_REST */
            if (IDENTIFIER_REST()) {
                goNextToken();
                return true;
            }
        } else if (checkToken("_")) {
        /* IDENTIFIER -> _ IDENTIFIER_REST */
            cursor++;
            if (IDENTIFIER_REST()) {
                goNextToken();
                return true;
            }
        }
        return false;
    }

    bool IDENTIFIER_REST() {
        if (debug) cout << "IDENTIFIER_REST -> LETTER IDENTIFIER_REST" << endl;
        if (debug) cout << "IDENTIFIER_REST -> DIGIT IDENTIFIER_REST" << endl;
        if (debug) cout << "IDENTIFIER_REST -> _ IDENTIFIER_REST" << endl;
        if (debug) cout << "IDENTIFIER_REST -> EOP" << endl;
        cout << endl;
        if (LETTER()) {
        /* IDENTIFIER_REST -> LETTER IDENTIFIER_REST */
            if (IDENTIFIER_REST()) {
                return true;
            }
        } else if (DIGIT()) {
        /* IDENTIFIER_REST -> DIGIT IDENTIFIER_REST */
            if (IDENTIFIER_REST()) {
                return true;
            }
        } else if (checkToken("_")) {
        /* IDENTIFIER_REST -> _ IDENTIFIER_REST */
            cursor++;
            if (IDENTIFIER_REST()) {
                return true;
            }
        } else if (EOP()) {
        /* IDENTIFIER_REST -> EOP */
            return true;
        }
        return false;
    }

    bool LETTER() {
        if (debug) cout << "LETTER -> a | b | c | ... | z | A | B | C | ... | Z" << endl;
        cout << endl;
        if ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z')) {
        /* LETTER -> a | b | c | ... | z | A | B | C | ... | Z */
            cursor++;
            cout << "l" << endl;
            return true;
        }
        return false;
    }

    bool DIGIT() {
        if (debug) cout << "DIGIT -> 0 | 1 | 2 | ... | 9" << endl;
        cout << endl;
        if (*cursor >= '0' && *cursor <= '9') {
        /* DIGIT -> 0 | 1 | 2 | ... | 9 */
            cout << "d" << endl;
            cursor++;
            return true;
        }
        return false;
    }

    bool SYMBOL() {
        if (debug) cout << "SYMBOL -> ! | \" | # | $ | % | & | ' | ( | ) | * | + | , | - | . | / | : | ; | < | = | > | ? | @ | [ | \\ | ] | ^ | _ | ` | { | | | | }" << endl;
        cout << endl;
        const char* symbols = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
        for (const char* s = symbols; *s != '\0'; s++) {
            if (*cursor == *s) {
        /* SYMBOL -> ! | " | # | $ | % | & | ' | ( | ) | * | + | , | - | . | / | : | ; | < | = | > | ? | @ | [ | \ | ] | ^ | _ | ` | { | | | | } */
                cout << "s" << endl;
                cursor++;
                return true;
            }
        }
        return false;
    }

    bool INT_TYPE() {
        if (debug) cout << "INT_TYPE -> int" << endl;
        cout << endl;
        if (checkToken("integer")) {
        /* INT_TYPE -> int */
            goNextToken();
            return true;
        }
        return false;
    }

    bool BOOL_TYPE() {
        if (debug) cout << "BOOL_TYPE -> bool" << endl;
        cout << endl;
        if (checkToken("boolean")) {
        /* BOOL_TYPE -> bool */
            goNextToken();
            return true;
        }
        return false;
    }

    bool CHAR_TYPE() {
        if (debug) cout << "CHAR_TYPE -> char" << endl;
        cout << endl;
        if (checkToken("char")) {
        /* CHAR_TYPE -> char */
            goNextToken();
            return true;
        }
        return false;
    }

    bool STRING_TYPE() {
        if (debug) cout << "STRING_TYPE -> string" << endl;
        cout << endl;
        if (checkToken("string")) {
        /* STRING_TYPE -> string */
            goNextToken();
            return true;
        }
        return false;
    }

    bool VOID_TYPE() {
        if (debug) cout << "VOID_TYPE -> void" << endl;
        cout << endl;
        if (checkToken("void")) {
        /* VOID_TYPE -> void */
            goNextToken();
            return true;
        }
        return false;
    }

    bool INT_LITERAL() {
        if (debug) cout << "INT_LITERAL -> DIGIT INT_LITERAL_REST" << endl;
        cout << endl;
        if (DIGIT()) {
        /* INT_LITERAL -> DIGIT INT_LITERAL_REST */
            if (INT_LITERAL_REST()) {
                return true;
            }
        }
        return false;
    }

    bool INT_LITERAL_REST() {
        if (debug) cout << "INT_LITERAL_REST -> DIGIT INT_LITERAL_REST" << endl;
        if (debug) cout << "INT_LITERAL_REST -> EOP" << endl;
        cout << endl;
        if (DIGIT()) {
        /* INT_LITERAL_REST -> DIGIT INT_LITERAL_REST */
            if (INT_LITERAL_REST()) {
                return true;
            }
        } else if (EOP()) {
        /* INT_LITERAL_REST -> EOP */
            return true;
        }
        return false;
    }

    bool CHAR_LITERAL() {
        if (debug) cout << "CHAR_LITERAL -> ' CHAR_LITERAL_REST '" << endl;
        if (debug) cout << "CHAR_LITERAL -> EOP" << endl;
        cout << endl;
        if (checkToken("\'")) {
        /* CHAR_LITERAL -> ' CHAR_LITERAL_REST ' */
            goNextToken();
            if (CHAR_LITERAL_REST()) {
                if (checkToken("'")) {
                    goNextToken();
                    return true;
                }
            }
        } else if (EOP()) {
        /* CHAR_LITERAL -> EOP */
            return true;
        }
        return false;
    }

    bool CHAR_LITERAL_REST() {
        if (debug) cout << "CHAR_LITERAL_REST -> DIGIT" << endl;
        if (debug) cout << "CHAR_LITERAL_REST -> LETTER" << endl;
        if (debug) cout << "CHAR_LITERAL_REST -> SYMBOL" << endl;
        if (debug) cout << "CHAR_LITERAL_REST -> EOP" << endl;
        cout << endl;
        if (DIGIT()) {
        /* CHAR_LITERAL_REST -> DIGIT */
            if (CHAR_LITERAL_REST()) {
                return true;
            }
        } else if (LETTER()) {
        /* CHAR_LITERAL_REST -> LETTER */
            if (CHAR_LITERAL_REST()) {
                return true;
            }
        } else if (SYMBOL()) {
        /* CHAR_LITERAL_REST -> SYMBOL */
            if (CHAR_LITERAL_REST()) {
                return true;
            }
        } else if (EOP()) {
        /* CHAR_LITERAL_REST -> EOP */
            return true;
        }
        return false;
    }

    bool STRING_LITERAL() {
        if (debug) cout << "STRING_LITERAL -> \" STRING_LITERAL_REST \"" << endl;
        cout << endl;
        if (checkToken("\"")) {
        /* STRING_LITERAL -> " STRING_LITERAL_REST " */
            goNextToken();
            if (STRING_LITERAL_REST()) {
                if (checkToken("\"")) {
                    goNextToken();
                    return true;
                }
            }
        }
        return false;
    }

    bool STRING_LITERAL_REST() {
        if (debug) cout << "STRING_LITERAL_REST -> CHAR_LITERAL_REST STRING_LITERAL_REST" << endl;
        if (debug) cout << "STRING_LITERAL_REST -> EOP" << endl;
        cout << endl;
        if (CHAR_LITERAL_REST()) {
        /* STRING_LITERAL_REST -> CHAR_LITERAL_REST STRING_LITERAL_REST */
            if (STRING_LITERAL_REST()) {
                return true;
            }
        } else if (EOP()) {
        /* STRING_LITERAL_REST -> EOP */
            return true;
        }
        return false;
    }

    bool BOOL_LITERAL() {
        if (debug) cout << "BOOL_LITERAL -> true" << endl;
        if (debug) cout << "BOOL_LITERAL -> false" << endl;
        cout << endl;
        if (checkToken("true")) {
        /* BOOL_LITERAL -> true */
            goNextToken();
            return true;
        } else if (checkToken("false")) {
        /* BOOL_LITERAL -> false */
            goNextToken();
            return true;
        }
        return false;
    }
};

int main() {
    Scanner scanner;

    vector<vector<char>> buffer = scanner.readBMMFile("input2.bmm");

    // for (int i = 0; i < buffer.size(); i++) {
    //     for (int j = 0; j < buffer[i].size(); j++) {
    //         cout << buffer[i][j];
    //     }
    // }

    cout<<"------EMPEZANDO EL SCANNER..."<<endl;
    vector<tuple<string, string, int, int>> tokens = scanner.Tokenize(buffer);

    if(errores_encontrados.size()>0)
    {
        cout<<"------SCANNER TERMINADO CON "<<errores_encontrados.size() <<" ERRORES"<<endl;
        for(int i =0 ; i <errores_encontrados.size();i++)
        {
            cout << get<0>(errores_encontrados[i])  << get<1>(errores_encontrados[i]) << "' en fila " 
                     << get<2>(errores_encontrados[i]) << ", columna " << get<3>(errores_encontrados[i])-1<< endl;
        }
    }
    else{
        cout<<"------SCANNER TERMINADO SIN ERRORES"<<endl;
    }
    
    Parser parser(tokens, true);

    return 0;
}
