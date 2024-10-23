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
            if(token.first == "STRING")
            {
                
                string clean = token.second.substr(1, token.second.length() - 2);
                string comillas = "\"";
                tokens.push_back(make_tuple("DOUBLE_QUOTE",comillas,startRow,startCol));
                tokens.push_back(make_tuple(token.first,clean,startRow,startCol));
                tokens.push_back(make_tuple("DOUBLE_QUOTE",comillas,startRow,startCol));
                continue;
            }
            if(token.first == "CHAR")
            {
                
                string clean = token.second.substr(1, token.second.length() - 2);
                string comillas_m = "\'";
                tokens.push_back(make_tuple("SINGLE_QUOTE",comillas_m,startRow,startCol));
                tokens.push_back(make_tuple(token.first,clean,startRow,startCol));
                tokens.push_back(make_tuple("SINGLE_QUOTE",comillas_m,startRow,startCol));
                continue;
            }
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
    int tab = 0;

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
            for (int i = 0; i < tab; i++) {
                cout << ' ';
            }
            if (cursor[i] == token[i]) {
                cout << cursor[i] << " = " << token[i] << endl;
            }
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
        cout << "------------- NEXT TOKEN -------------" << endl;
        cout << get<1>(tokens[index]) << endl;
        cout << "----------------------------------------" << endl;
    }

    bool checkTokenType(const string& tokenType) {
        if (get<0>(tokens[index]) == tokenType) {
            cout << "-----> " << get<1>(tokens[index]) << "-" << tokenType << " <-----" << endl;
            return true;
        }
        return false;
    }
    void printDebug(string s) {
        if (debug) {
            for (int i = 0; i < tab; i++) {
                cout << ' ';
            }
            cout << s << endl;
        }
        //tab++;
    }

    bool PROGRAM() {
        printDebug("PROGRAM -> DECLARATION PROGRAM_REST");
        /* PROGRAM -> DECLARATION PROGRAM_REST */
        if (DECLARATION()) {
            if (PROGRAM_REST()) {
                cout << "PARSE ENDED IN " << get<0>(tokens[index]) << ' ' << get<1>(tokens[index]) << ' ' << get<2>(tokens[index]) << ' ' << get<3>(tokens[index]) <<endl;
                if (index != tokens.size() - 1) {
                    cout << "Failure parsing";
                    return false;
                }
                return true;
            }
        }
        return false;
    }

    bool PROGRAM_REST() {
        printDebug("PROGRAM_REST -> DECLARATION PROGRAM_REST");
        /* PROGRAM_REST -> DECLARATION PROGRAM_REST*/
        if (DECLARATION()) {
            if (PROGRAM_REST()) {
                return true;
            }
        } else {
            
            printDebug("PROGRAM_REST -> EOP");
        /* PROGRAM_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool DECLARATION() {
        printDebug("DECLARATION -> FUNCTION DECLARATION_REST");
        /* DECLARATION -> FUNCTION DECLARATION_REST*/
        if (FUNCTION()) {
            if (DECLARATION_REST()) {
                return true;
            }
        }
        return false;
    }
    
    bool DECLARATION_REST() {
        printDebug("DECLARATION_REST -> ( PARAMS ) { STMT_LIST }");
        /* DECLARATION_REST -> ( PARAMS ) { STMT_LIST }  */
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
        } else {
            
            printDebug("DECLARATION_REST -> VAR_DECL");
        /* DECLARATION_REST -> VAR_DECL  */
            if (VAR_DECL()) {
                return true;
            }
        }
        return false;
    }

    bool FUNCTION() {
        printDebug("FUNCTION -> TYPE IDENTIFIER");
        /* FUNCTION -> TYPE IDENTIFIER */
        if (TYPE()) {
            if (IDENTIFIER()) {
                return true;
            }
        }
        return false;
    }

    bool TYPE() {
        printDebug("TYPE -> INT_TYPE TYPE_REST");
        /* TYPE -> INT_TYPE TYPE_REST */
        if (INT_TYPE()) {
            if (TYPE_REST()) {
                return true;
            }
        } else {
            
            printDebug("TYPE -> BOOL_TYPE TYPE_REST");
        /* TYPE -> BOOL_TYPE TYPE_REST */
            if (BOOL_TYPE()) {
                if (TYPE_REST()) {
                    return true;
                }
            }
            else {
                
                printDebug("TYPE -> CHAR_TYPE TYPE_REST");
        /* TYPE -> CHAR_TYPE TYPE_REST*/
                if (CHAR_TYPE()) {
                    if (TYPE_REST()) {
                        return true;
                    }
                }
                else {
                    
                    printDebug("TYPE -> STRING_TYPE TYPE_REST");
        /* TYPE -> STRING_TYPE TYPE_REST*/
                    if (STRING_TYPE()) {
                        if (TYPE_REST()) {
                            return true;
                        }
                    }
                    else {
                        
                        printDebug("TYPE -> VOID_TYPE TYPE_REST");
        /* TYPE -> VOID_TYPE TYPE_REST */
                        if (VOID_TYPE()) {
                            if (TYPE_REST()) {
                                return true;
                            }
                        }
                    
                    }
                }
            }
        }
        return false;
    }

    bool TYPE_REST() {
        printDebug("TYPE_REST -> [ ] TYPE_REST");
        /* TYPE_REST -> [ ] TYPE_REST */  
        if (checkToken("[")) {
            goNextToken();
            if (checkToken("]")) {
                goNextToken();
                if (TYPE_REST()) {
                    return true;
                }
            }
        } else {
            
            printDebug("TYPE_REST -> EOP");
        /* TYPE_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool PARAMS() {
        printDebug("PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
        if (INT_TYPE()) {
            if (TYPE_REST()) {
                if (IDENTIFIER()) {
                    if (PARAMS_REST()) {
                        return true;
                    }
                }
            }
        } else {
            
            printDebug("PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
            if (BOOL_TYPE()) {
                if (TYPE_REST()) {
                    if (IDENTIFIER()) {
                        if (PARAMS_REST()) {
                            return true;
                        }
                    }
                }
            } else {
                
                printDebug("PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                if (CHAR_TYPE()) {
                    if (TYPE_REST()) {
                        if (IDENTIFIER()) {
                            if (PARAMS_REST()) {
                                return true;
                            }
                        }
                    }
                } else {
                    
                    printDebug("PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                    if (STRING_TYPE()) {
                        if (TYPE_REST()) {
                            if (IDENTIFIER()) {
                                if (PARAMS_REST()) {
                                    return true;
                                }
                            }
                        }
                    } else {
                        
                        printDebug("PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                        if (VOID_TYPE()) {
                            if (TYPE_REST()) {
                                if (IDENTIFIER()) {
                                    if (PARAMS_REST()) {
                                        return true;
                                    }
                                }
                            }
                        } else {
                            
                            printDebug("PARAMS -> EOP");
        /* PARAMS -> EOP */
                            if (EOP()) {
                                return true;
                            }
                        }
                    }
                }
            }
        } 
        return false;
    }

    bool PARAMS_REST() {
        printDebug("PARAMS_REST -> , PARAMS");
        /* PARAMS_REST -> , PARAMS */
        if (checkToken(",")) {
            goNextToken();
            if (PARAMS()) {
                return true;
            }
        } else {
            
            printDebug("PARAMS_REST -> EOP");
        /* PARAMS_REST -> EOP  */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool VAR_DECL() {
        printDebug("VAR_DECL -> ;");
        /* VAR_DECL-> ; */
        if (checkToken(";")) {
            goNextToken();
            return true;
        } else  {
            
            printDebug("VAR_DECL -> = EXPRESSION ;");
        /* VAR_DECL-> = EXPRESSION ; */
            if (checkToken("=")) {
                goNextToken();
                if (EXPRESSION()) {
                    if (checkToken(";")) {
                        goNextToken();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool STMT_LIST() {
        printDebug("STMT_LIST -> STATEMENT STMT_LIST_REST");
        /* STMT_LIST -> STATEMENT STMT_LIST_REST */
        if (STATEMENT()) {
            if (STMT_LIST_REST()) {
                return true;
            }
        }
        return false;
    };

    bool STMT_LIST_REST() {
        printDebug("STMT_LIST_REST -> STATEMENT STMT_LIST_REST");
        /* STMT_LIST_REST -> STATEMENT STMT_LIST_REST */
        if (STATEMENT()) {
            if (STMT_LIST_REST()) {
                return true;
            }
        } else {
            
            printDebug("STMT_LIST_REST -> EOP");
        /* STMT_LIST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool STATEMENT() {
        /* STATEMENT -> FUNCTIONVAR_DECL */
        printDebug("STATEMENT -> FUNCTION VAR_DECL");
        if (FUNCTION()) {
            if (VAR_DECL()) {
                return true;
            }
        } else {
            
            printDebug("STATEMENT -> IF_STMT");
        /* STATEMENT -> IF_STMT */
            if (IF_STMT()) {
                return true;
            } else {
                
                printDebug("STATEMENT -> FOR_STMT");
        /* STATEMENT -> FOR_STMT */
                if (FOR_STMT()) {
                    return true;
                } else {
                    
                    printDebug("STATEMENT -> RETURN_STMT");
        /* STATEMENT -> RETURN_STMT */
                    if (RETURN_STMT()) {
                        return true;
                    } else {
                        
                        printDebug("STATEMENT -> EXPR_STMT");
        /* STATEMENT -> EXPR_STMT */
                        if (EXPR_STMT()) {
                            return true;
                        } else {
                            
                            printDebug("STATEMENT -> PRINT_STMT");
        /* STATEMENT -> PRINT_STMT */
                            if (PRINT_STMT()) {
                                return true;
                            } else {
                                
                                printDebug("{ STMT_LIST }");
        /* STATEMENT -> { STMT_LIST } */
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
            }
        }
        return false;
    }

    bool IF_STMT() {
        printDebug("IF_STMT -> if ( EXPRESSION ) { STMT_LIST } IF_STMT_REST");
        /* IF_STMT -> if ( EXPRESSION ) { STMT_LIST } IF_STMT_REST */
        if (checkToken("if")) {
            goNextToken();
            if (checkToken("(")) {
                goNextToken();
                if (EXPRESSION()) {
                    if (checkToken(")")) {
                        goNextToken();
                        if (checkToken("{")) {
                            goNextToken();
                            if (STMT_LIST()) {
                                cout << get<1>(tokens[index]) << "------------" << endl;                                 
                                if (checkToken("}")) {
                                    goNextToken();
                                    if (IF_STMT_REST()) {
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
        printDebug("IF_STMT_REST -> else { STMT_LIST }");
        /* IF_STMT_REST -> else { STMT_LIST } */
        if (checkToken("else")) {
            goNextToken();
            if (checkToken("{")) {
                goNextToken();
                if (STMT_LIST()) {
                    if (checkToken("}")) {
                        return true;
                    }
                }
            }
        } else {
            
            printDebug("IF_STMT_REST -> EOP");
        /* IF_STMT_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool FOR_STMT() {
        printDebug("FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STMT_LIST }");
        /* FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STMT_LIST } */
        if (checkToken("for")) {
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
                }
            }
        }
        return false;
    }

    bool RETURN_STMT() {
        printDebug("RETURN_STMT -> return EXPRESSION ;");
        /* RETURN_STMT -> return EXPRESSION ; */
        if (checkToken("return")) {
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

    bool PRINT_STMT() {
        printDebug("PRINT_STMT -> print ( EXPR_LIST ) ;");
        /* PRINT_STMT -> print ( EXPR_LIST ) ; */
        if (checkToken("print")) {
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
        printDebug("EXPR_STMT -> EXPRESSION ;");
        /* EXPR_STMT -> EXPRESSION ; */
        if (EXPRESSION()) {
            if (checkToken(";")) {
                goNextToken();
                return true;
            }
        } else {
            
            printDebug("EXPR_STMT -> ;");
            /* EXPR_STMT -> ; */
            if (checkToken(";")) {
                goNextToken();
                return true;
            }
        }
        return false;
    }

    bool EXPR_LIST() {
        printDebug("EXPR_LIST -> EXPRESSION EXPR_LIST_REST");
        /* EXPR_LIST -> EXPRESSION EXPR_LIST_REST */
        if (EXPRESSION()) {
            if (EXPR_LIST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool EXPR_LIST_REST() {
        printDebug("EXPR_LIST_REST -> , EXPR_LIST");
        /* EXPR_LIST_REST -> , EXPR_LIST */
        if (checkToken(",")) {
            goNextToken();
            if (EXPR_LIST()) {
                return true;
            }
        } else {
            
            printDebug("EXPR_LIST_REST -> EOP");
        /* EXPR_LIST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool EXPRESSION() {
        printDebug("EXPRESSION -> IDENTIFIER = EXPRESSION");
        /* EXPRESSION -> IDENTIFIER = EXPRESSION */
        bool flag = false;
        int state = index;
        if (IDENTIFIER()) {
            if (checkToken("=")) {
                goNextToken();
                if (EXPRESSION()) {
                    return true;
                } else {
                    flag = true;
                }
            } else {
                flag = true;
            }
        } else {
            flag = true;
        }
        if (flag) {
            index = state;
            
            printDebug("EXPRESSION -> OR_EXPR");
        /* EXPRESSION -> OR_EXPR */
            if (OR_EXPR()) {
                return true;
            }
        }
        return false;
    }

    bool OR_EXPR() {
        printDebug("OR_EXPR -> AND_EXPR OR_EXPR_REST");
        /* OR_EXPR -> AND_EXPR OR_EXPR_REST */
        if (AND_EXPR()) {
            if (OR_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }

    bool OR_EXPR_REST() {
        printDebug("OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST");
        /* OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST */
        if (checkToken("||")) {
            goNextToken();
            if (AND_EXPR()) {
                if (OR_EXPR_REST()) {
                    return true;
                }
            }
        } else {
            
            printDebug("OR_EXPR_REST -> EOP");
        /* OR_EXPR_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool AND_EXPR() {
        printDebug("AND_EXPR -> EQ_EXPR AND_EXPR_REST");
        /* AND_EXPR -> EQ_EXPR AND_EXPR_REST */
        if (EQ_EXPR()) {
            if (AND_EXPR_REST()) {
                return true;
            }
        }
        return false;
    }

    bool AND_EXPR_REST() {
        printDebug("AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST");
        /* AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST */
        if (checkToken("&&")) {
            goNextToken();
            if (EQ_EXPR()) {
                if (AND_EXPR_REST()) {
                    return true;
                }
            }
        } else {
            
            printDebug("AND_EXPR_REST -> EOP");
        /* AND_EXPR_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool EQ_EXPR() {
        printDebug("EQ_EXPR -> REL_EXPR EQ_EXPR_REST_REST");
        /* EQ_EXPR -> EXPR EQ_EXPR_REST_REST */
        if (REL_EXPR()) {
            if (EQ_EXPR_REST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool EQ_EXPR_REST() {
        printDebug("EQ_EXPR_REST -> == REL_EXPR");
        /* EQ_EXPR_REST -> == REL_EXPR*/
        if (checkToken("==")) {
            goNextToken();
            if (REL_EXPR()) {
                if (EQ_EXPR_REST()) {
                    return true;
                }
            }
        } else {
            
            printDebug("EQ_EXPR_REST -> != REL_EXPR");
        /* EQ_EXPR_REST -> != REL_EXPR*/
            if (checkToken("!=")) {
                goNextToken();
                if (REL_EXPR()) {
                    return true;
                }
            }
        }
        return false;
    }

    bool EQ_EXPR_REST_REST() {
        printDebug("EQ_EXPRT_REST_REST -> EQ_EXPR_REST EQ_EXPR_REST_REST");
        /* EQ_EXPR_REST_REST -> EQ_EXPR_REST EQ_EXPR_REST_REST */
        if (EQ_EXPR_REST()) {
            if (EQ_EXPR_REST_REST()) {
                return true;
            }
        }
        else {
            
            printDebug("EQ_EXPR_REST_REST -> EOP");
        /* EQ_EXPR_REST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool REL_EXPR() {
        printDebug("REL_EXPR -> EXPR REL_EXPR_REST_REST");
        /* REL_EXPR -> EXPR REL_EXPR_REST_REST */
        if (EXPR()) {
            if (REL_EXPR_REST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool REL_EXPR_REST() {
        printDebug("REL_EXPR_REST -> < EXPR");
        /* REL_EXPR_REST -> < EXPR */
        if (checkToken("<")) {
            goNextToken();
            if (EXPR()) {
                return true;
            }
        } else {
            
            printDebug("REL_EXPR_REST -> > EXPR");
        /* REL_EXPR_REST -> > EXPR */
            if (checkToken(">")) {
                goNextToken();
                if (EXPR()) {
                    return true;
                }
            } else {
                
                printDebug("REL_EXPR_REST -> <= EXPR");
        /* REL_EXPR_REST -> <= EXPR */
                if (checkToken("<=")) {
                    goNextToken();
                    if (EXPR()) {
                        return true;
                    }
                } else {
                    
                    printDebug("REL_EXPR_REST -> >= EXPR");
        /* REL_EXPR_REST -> >= EXPR */
                    if (checkToken(">=")) {
                        goNextToken();
                        if (EXPR()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool REL_EXPR_REST_REST() {
        printDebug("REL_EXPR_REST_REST -> REL_EXPR_REST REL_EXPR_REST_REST");
        /* REL_EXPR_REST_REST -> REL_EXPR_REST REL_EXPR_REST_REST */
        if (REL_EXPR_REST()) {
            if (REL_EXPR_REST_REST()) {
                return true;
            }
        }
        else {
            
            printDebug("REL_EXPR_REST_REST -> EOP");
        /* EXPR_REST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool EXPR() {
        printDebug("EXPR -> TERM EXPR_REST_REST");
        /* EXPR -> TERM EXPR_REST_REST */
        if (TERM()) {
            if (EXPR_REST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool EXPR_REST() {
        printDebug("EXPR_REST -> + TERM");
        /* EXPR_REST -> + TERM */
        if (checkToken("+")) {
            goNextToken();
            if (TERM()) {
                return true;
            }
        }
        else {
            
            printDebug("EXPR_REST -> - TERM");
            if (checkToken("-")) {
                goNextToken(); 
                if (TERM()) {
                    return true;
                }
            }
        }
        return false;
    }

    bool EXPR_REST_REST() {
        printDebug("EXPR_REST_REST -> EXPR_REST EXPR_REST_REST");
        /* EXPR_REST_REST -> EXPR_REST EXPR_REST_REST */
        if (EXPR_REST()) {
            if (EXPR_REST_REST()) {
                return true;
            }
        }
        else {
            
            printDebug("EXPR_REST_REST -> EOP");
        /* EXPR_REST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        }
        return false;
    }

    bool TERM() {
        printDebug("TERM -> UNARY TERM_REST_REST");
        /* TERM -> UNARY TERM_REST_REST */
        if (UNARY()) {
            if (TERM_REST_REST()) {
                return true;
            }
        }
        return false;
    }

    bool TERM_REST() {
        printDebug("TERM_REST -> * UNARY");
        /* TERM_REST -> * UNARY */
        if (checkToken("*")) {
            goNextToken();
            if (UNARY()) {
                return true;
            }
        } else {
            
            printDebug("TERM_REST -> / UNARY");
        /* TERM_REST -> / UNARY */
            if (checkToken("/")) {
                goNextToken();
                if (UNARY()) {
                    return true;
                }
            } else {
                
                printDebug("TERM_REST -> % UNARY");
        /* TERM_REST -> % UNARY */
                if (checkToken("%")) {
                    goNextToken();
                    if (UNARY()) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool TERM_REST_REST() {
        printDebug("TERM_REST_REST -> TERM_REST TERM_REST_REST");
    /* TERM_REST_REST -> TERM_REST TERM_REST_REST */
        if (TERM_REST()) {
            if (TERM_REST_REST()) {
                return true;
            }
        }
        else {
            
            printDebug("TERM_REST_REST -> EOP");
        /* TERM_REST_REST -> EOP */
            if (EOP()) {
                return true;
            }
        } 
        return false;  
    }

    bool UNARY() {
        printDebug("UNARY -> ! UNARY");
        /* UNARY -> ! UNARY */
        if (checkToken("!")) {
            goNextToken();
            if (UNARY()) {
                return true;
            }
        } else {
            
            printDebug("UNARY -> - UNARY");
        /* UNARY -> - UNARY */
            if (checkToken("-")) {
                goNextToken();
                if (UNARY()) {
                    return true;
                }
            } else {
                
                printDebug("UNARY -> FACTOR");
        /* UNARY -> FACTOR */
                if (FACTOR()) {
                    return true;
                }
            }
        } 
        return false;
    }

    bool FACTOR() {
        printDebug("FACTOR -> IDENTIFIER FACTOR_REST");
        /* FACTOR -> IDENTIFIER FACTOR_REST */
        if (IDENTIFIER()) {
            if (FACTOR_REST()) {
                return true;
            }
        } else {
            
            printDebug("FACTOR -> INT_LITERAL FACTOR_REST");
        /* FACTOR -> INT_LITERAL FACTOR_REST */
            if (INT_LITERAL()) {
                if (FACTOR_REST()) {
                    return true;
                }
            } else {
                
                printDebug("FACTOR -> CHAR_LITERAL FACTOR_REST");
        /* FACTOR -> CHAR_LITERAL FACTOR_REST */
                if (CHAR_LITERAL()) {
                    if (FACTOR_REST()) {
                        return true;
                    }
                } else {
                    
                    printDebug("FACTOR -> STRING_LITERAL FACTOR_REST");
        /* FACTOR -> STRING_LITERAL FACTOR_REST */
                    if (STRING_LITERAL()) {
                        if (FACTOR_REST()) {
                            return true;
                        }
                    } else {
                        
                        printDebug("FACTOR -> BOOL_LITERAL FACTOR_REST");
        /* FACTOR -> BOOL_LITERAL FACTOR_REST */
                        if (BOOL_LITERAL()) {
                            if (FACTOR_REST()) {
                                return true;
                            }
                        } else {
                            
                            printDebug("FACTOR -> ( EXPRESSION ) FACTOR_REST");
        /* FACTOR -> ( EXPRESSION ) FACTOR_REST */
                            if (checkToken("(")) {
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
                        }
                    }
                }
            }
        }
        return false;
    }

    bool FACTOR_REST() {
        printDebug("FACTOR_REST -> [ EXPRESSION ] FACTOR_REST");
        /* FACTOR_REST -> [ EXPRESSION ] FACTOR_REST */
        if (checkToken("[")) {
            goNextToken();
            if (EXPRESSION()) {
                if (checkToken("]")) {
                    goNextToken();
                    if (FACTOR_REST()) {
                        return true;
                    }
                }
            }
        } else {
            
            printDebug("FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST");
        /* FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST */
            if (checkToken("(")) {
                goNextToken();
                if (EXPR_LIST()) {
                    if (checkToken(")")) {
                        goNextToken();
                        if (FACTOR_REST()) {
                            return true;
                        }
                    }
                }
            } else {
                
                printDebug("FACTOR_REST -> EOP");
                if (EOP()) {
                    return true;
                }
        /* FACTOR_REST -> EOP */
            }
        }
        return false;
    }

    bool EOP() {
        return true;
    }

    bool IDENTIFIER() {
        printDebug("IDENTIFIER -> <Check Token Type>");
        /* IDENTIFIER -> <Check Token Type> */
        if (checkTokenType("IDENTIFIER")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool INT_TYPE() {
        printDebug("INT_TYPE -> integer");
        /* INT_TYPE -> integer */
        if (checkToken("integer")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool BOOL_TYPE() {
        printDebug("BOOL_TYPE -> boolean");
        /* BOOL_TYPE -> boolean */
        if (checkToken("boolean")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool CHAR_TYPE() {
        printDebug("CHAR_TYPE -> char");
        /* CHAR_TYPE -> char */
        if (checkToken("char")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool STRING_TYPE() {
        printDebug("STRING_TYPE -> string");
        /* STRING_TYPE -> string */
        if (checkToken("string")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool VOID_TYPE() {
        printDebug("VOID_TYPE -> void");
        /* VOID_TYPE -> void */
        if (checkToken("void")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool INT_LITERAL() {
        printDebug("INT_LITERAL -> <Check Token Type>");
        /* INT_LITERAL -> <Check Token Type> */
        if (checkTokenType("INTEGER")) {
            goNextToken();
            return true;
        }
        return false;
    }

    bool CHAR_LITERAL() {
        printDebug("CHAR_LITERAL -> ' <Check Token Type> '");
        /* CHAR_LITERAL -> ' <Check Token Type> ' */
        if (checkToken("'")) {
            goNextToken();
            if (checkTokenType("CHAR")) {
                goNextToken();
                if (checkToken("'")) {
                    goNextToken();
                    return true;
                }
            }
        }
        return false;
    }

    bool STRING_LITERAL() {
        printDebug("STRING_LITERAL -> \" <Check Token Type> \"");
        /* STRING_LITERAL -> " <Check Token Type> " */
        if (checkToken("\"")) {
            goNextToken();
            if (checkTokenType("STRING")) {
                goNextToken();
                if (checkToken("\"")) {
                    goNextToken();
                    return true;
                }
            }
        }
        return false;
    }

    bool BOOL_LITERAL() {
        printDebug("BOOL_LITERAL -> true");
        /* BOOL_LITERAL -> true */
        if (checkToken("true")) {
            goNextToken();
            return true;
        } else {
            printDebug("BOOL_LITERAL -> false");
        /* BOOL_LITERAL -> false */
            if (checkToken("false")) {
                goNextToken();
                return true;
            }
        }
        return false;
    }
};

int main() {
    Scanner scanner;

    vector<vector<char>> buffer = scanner.readBMMFile("input3.bmm");

    for (int i = 0; i < buffer.size(); i++) {
        for (int j = 0; j < buffer[i].size(); j++) {
            cout << buffer[i][j];
        }
    }

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
    for (int i = 0; i < tokens.size(); i++) {
        printf("Token: %-20s Value: %-20s Fila: %-20d Columna: %-20d\n", 
       get<0>(tokens[i]).c_str(), get<1>(tokens[i]).c_str(), get<2>(tokens[i]) + 1, get<3>(tokens[i]) + 1);
    }

    
    Parser parser(tokens, false);

    return 0;
}