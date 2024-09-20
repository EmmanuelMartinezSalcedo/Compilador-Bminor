#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <cctype>
#include <algorithm>
#include <tuple>
#include <string.h>
using namespace std;

enum TokenType {
    IDENTIFIER,
    KEYWORD,
    INTEGER,
    STRING,
    CHAR,
    OPERATOR,
    COMMENT,
    WHITESPACE,
    SYMBOL,
    UNKNOWN,
    ERROR
};

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
                    return {"BOOLEAN",value};
                }
                else if(value =="char")
                {
                    return {"CHAR",value};
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
                    return {"INTEGER",value};
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
                    return {"STRING",value};
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
                    return{"ERROR_IDENTIFIER",value};
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
                return {"CARACTER", value}; // Se retorna el token de tipo char
            } else {
                return {"ERROR_CHAR", value}; // Error si no se cierra con comilla simple
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
                return {"CADENAS", value}; // Retorna el token de tipo string
            } else {
                return {"ERROR_CADENA", value}; // Error si no se cierra con comillas dobles
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
                return{"ERROR_ID_INICIO",value};
            }
            if(value.size()>19)
            {
                return {"ERROR_INT",value};
            }
            else if( value.size() ==19 )  //valor maximo para int es de 2^63 - 1   9,223,372,036,854,775,807
            {

                int part1 = stoi(value.substr(0,9));
                int part2 = stoi(value.substr(10));
                if(part1 <= 922337203 && part2<=854775807)
                {
                    return {"INTEGER", value};
                }
                else{
                    return {"ERROR_INT",value};
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
            else if(symbol =="\"")
            {
                return {"DOUBLE_QUOTE",symbol};
            }
            else if(symbol =="\'")
            {
                return {"QUOTE",symbol};
            }
            
        }

        // Si el carácter es inválido, registrar error
        return {"ERROR", string(1, c)};
    }

    vector<tuple<string, string, int, int>> Tokenize(vector<vector<char>>& buffer) {
        int row = 0, col = 0;
        vector<tuple<string, string, int, int>> tokens;

        while (row < buffer.size()) {
            int startRow = row;
            int startCol = col;
            pair<string, string> token = gettoken(buffer, row, col);

            if (token.first == "EOF") break;
            
            tokens.push_back(make_tuple(token.first, token.second, startRow, startCol));

            // Manejo de errores: imprimir mensaje si es un token de error
            if(token.first == "COMMENT_LINE" || token.first =="COMMENT_LARGE")
            {
                continue;
            }
            else if (token.first == "ERROR") {
                cerr << "Error: Caracter no valido: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else if(token.first == "ERROR_INT")
            {
                cerr << "Error: Entero fuera de rango: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else if(token.first == "ERROR_IDENTIFIER")
            {
                cerr << "Error: Identificador fuera de rango: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else if(token.first == "ERROR_CADENA")
            {
                cerr << "Error: No se cerro la cadena: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else if(token.first == "ERROR_CHAR")
            {
                cerr << "Error: no se cerro el char: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else if(token.first =="ERROR_ID_INICIO")
            {
                cerr << "Error: Identificador iniciado con int: '" << token.second << "' en fila " 
                     << startRow + 1 << ", columna " << startCol + 1 << endl;
            }
            else{
                cout << "Token: " << token.first << ", Value: " << token.second
             << "    Fila: " << startRow + 1 << ", Columna: " << startCol + 1 << endl;
            }
        }

        return tokens;
    }

private:
    vector<string> keywords;
    vector<string> symbols;
    vector<string> operators;
};

int main() {
    Scanner scanner;

    vector<vector<char>> buffer = scanner.readBMMFile("input.bmm");

    for (int i = 0; i < buffer.size(); i++) {
        for (int j = 0; j < buffer[i].size(); j++) {
            cout << buffer[i][j];
        }
    }

    vector<tuple<string, string, int, int>> tokens = scanner.Tokenize(buffer);


    return 0;
}
