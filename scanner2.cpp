#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <cctype>
#include <algorithm>
#include <tuple>
#include <string.h>
#include <string>
#include <set>
#include <map>
#include <functional>
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

struct Node {
    int id;
    int parentId;
    string value;
    string type;
    vector<int> children;
    bool includeInAST;
};

const set<string> nodesToRemove = {
    "PROGRAM_REST", "TYPE_REST", "PARAMS_REST", "STMT_LIST_REST",
    "IF_STMT_REST", "EXPR_LIST_REST", "OR_EXPR_REST", "AND_EXPR_REST",
    "EQ_EXPR_REST", "EQ_EXPR_REST_REST", "REL_EXPR_REST", "REL_EXPR_REST_REST",
    "EXPR_REST", "EXPR_REST_REST", "TERM_REST", "TERM_REST_REST",
    "FACTOR_REST", "EOP", "OR_EXPR", "AND_EXPR", "EQ_EXPR", "REL_EXPR",
    "EXPR", "TERM", "UNARY", "FACTOR"
};

const set<string> keepNodes = {
    "PROGRAM", "DECLARATION", "FUNCTION", "IF_STMT", "FOR_STMT",
    "RETURN_STMT", "PRINT_STMT", "VAR_DECL", "TYPE"
};

const set<string> uselessSymbols = {"(", ")", "{", "}", ";"};

string readCSVField(istream& str) {
    string result;
    bool inQuotes = false;
    char c;
    
    while (str.get(c)) {
        if (c == '"') {
            if (str.peek() == '"') {
                str.get();
                result += '"';
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            break;
        } else {
            result += c;
        }
    }
    
    return result;
}

vector<string> splitCSVLine(const string& line) {
    vector<string> fields;
    stringstream ss(line);
    
    while (ss.good()) {
        fields.push_back(readCSVField(ss));
    }
    
    return fields;
}

Node* createNode(int id, int parentId, const string& value, const string& type) {
    Node* node = new Node;
    node->id = id;
    node->parentId = parentId;
    node->value = value;
    node->type = type;
    node->includeInAST = node->type == "terminal" || keepNodes.find(node->value) != keepNodes.end();
    return node;
}

void buildAST(const vector<Node*>& nodes, map<int, Node*>& astNodes, int rootId) {
    if (rootId < 0) return;

    Node* root = nodes[rootId];
    astNodes[root->id] = root;

    for (int childId : root->children) {
        Node* child = nodes[childId];
        astNodes[child->id] = child;
        buildAST(nodes, astNodes, childId);
    }
}

void generateAST(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile);
    string line;
    vector<Node*> allNodes;
    map<int, Node*> astNodes;

    getline(inFile, line);

    while (getline(inFile, line)) {
        vector<string> fields = splitCSVLine(line);

        if (fields.size() >= 4) {
            Node* node = createNode(stoi(fields[0]), stoi(fields[1]), fields[2], fields[3]);
            if (uselessSymbols.find(node->value) == uselessSymbols.end()) {
                allNodes.push_back(node);
                if (node->parentId >= 0) {
                    allNodes[node->parentId]->children.push_back(node->id);
                }
            } else {
                delete node;
            }
        }
    }

    int rootId = -1;
    for (Node* node : allNodes) {
        if (node->parentId < 0) {
            rootId = node->id;
            break;
        }
    }
    cout << "------------" << endl;
    buildAST(allNodes, astNodes, rootId);

    ofstream outFile(outputFile);
    outFile << "ID,PadreID,Valor,Tipo\n";

    for (auto& pair : astNodes) {
        Node* node = pair.second;
        string escapedValue = node->value;
        bool needsQuotes = escapedValue.find(',') != string::npos || escapedValue.find('"') != string::npos;

        if (needsQuotes) {
            string quoted = "\"";
            for (char c : escapedValue) {
                if (c == '"') quoted += "\"\"";
                else quoted += c;
            }
            quoted += "\"";
            escapedValue = quoted;
        }

        outFile << node->id << "," << node->parentId << "," << escapedValue << "," << node->type << "\n";
    }

    for (Node* node : allNodes) {
        delete node;
    }
}

class Parser {
public:
    vector<tuple<string, string, int, int>> tokens;
    const char* cursor;
    int index;
    bool debug;
    int tab = 0;

    vector<string> errors;

    int currentID = 0;
    vector<tuple<int, int, string, string>> tempNodes;

    Parser(vector<tuple<string, string, int, int>>& tkns, bool dbg) {
        tokens = tkns;
        cursor = get<1>(tokens[0]).c_str();
        index = 0;
        debug = dbg;

        ofstream file("parseTree.csv", ios::trunc);
        file << "ID,PadreID,Valor,Tipo\n";
        file.close();

        if (PROGRAM(-1)) {
            writeTreeToFile();
            generateAST("parseTree.csv", "astTree.csv");
            cout << "Successful parse" << endl;
        } else {
            cout << "Error parsing in line " << get<2>(tokens[index]) << ", column " << get<3>(tokens[index]) << endl;
        }
    }

private:
    void printTempNodes() {
        cout << "ID\tParentID\tValue\tType" << endl;
        cout << "------------------------------------------" << endl;

        for (const auto& node : tempNodes) {
            int id = get<0>(node);
            int parentID = get<1>(node);
            string value = get<2>(node);
            string type = get<3>(node);

            cout << id << "\t" << parentID << "\t\t" << value << "\t" << type << endl;
        }
    }

    bool checkToken(const string& token) {
        int tokenSize = token.size();
        for (int i = 0; i < tokenSize; i++) {
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

    bool checkTokenType(const string& tokenType) {
        return (get<0>(tokens[index]) == tokenType);
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

    void addNode(int id, int padreID, const string& value, const string& tokenType) {
        tempNodes.push_back(make_tuple(id, padreID, value, tokenType));
    }

    void writeTreeToFile() {
        ofstream file("parseTree.csv", ios::app);
        if (file.is_open()) {
            for (const auto& node : tempNodes) {
                file << get<0>(node) << "," 
                     << get<1>(node) << "," 
                     << get<2>(node) << "," 
                     << get<3>(node) << "\n";
            }
            file.close();
        } else {
            cerr << "Error al abrir el archivo: parseTree.csv" << endl;
        }
    }
    
    class ScopeGuard {
        Parser& parser;
        size_t savedSize;
        bool committed;
        
    public:
        ScopeGuard(Parser& p) : parser(p), savedSize(p.tempNodes.size()), committed(false) {}

        void commit() {
            committed = true;
        }
        
        ~ScopeGuard() {
            if (!committed) {
                parser.tempNodes.resize(savedSize);
            }
        }
    };

    void reportError(const string& msg) {
        // Registra el error con la información de la línea y columna
        string errorMsg = "Error at line " + to_string(get<2>(tokens[index])) + ", column " + to_string(get<3>(tokens[index])) + ": " + msg;
        errors.push_back(errorMsg);
        cout << "Error at line " + to_string(get<2>(tokens[index])) + ", column " + to_string(get<3>(tokens[index])) + ": " + get<1>(tokens[index]) << endl;
    }

    // Función de recuperación en el modo panic
    void recoverFromError(string syncToken) {
        syncToken = ";";
        // Avanzar hasta encontrar el token de sincronización (syncToken)
        while (index < tokens.size() && get<1>(tokens[index]) != syncToken) {
            goNextToken();
        }
        // Asegurarse de que se ha encontrado el token de sincronización
        if (index < tokens.size() && get<1>(tokens[index]) == syncToken) {
            goNextToken(); // Avanzar al siguiente token después del sincronizador
        }
    }


    bool PROGRAM(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "PROGRAM", "non-terminal");

        printDebug("PROGRAM -> DECLARATION PROGRAM_REST");
        /* PROGRAM -> DECLARATION PROGRAM_REST */
        if (DECLARATION(myID)) {
            if (PROGRAM_REST(myID)) {
                if (index == tokens.size() - 1) {
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("DECLARATION");

        return false;
    }

    bool PROGRAM_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "PROGRAM_REST", "non-terminal");

        printDebug("PROGRAM_REST -> DECLARATION PROGRAM_REST");
        /* PROGRAM_REST -> DECLARATION PROGRAM_REST*/
        if (DECLARATION(myID)) {
            if (PROGRAM_REST(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("PROGRAM_REST -> EOP");
        /* PROGRAM_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("DECLARATION");
        return false;
    }

    bool DECLARATION(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "DECLARATION", "non-terminal");

        printDebug("DECLARATION -> FUNCTION DECLARATION_REST");
        /* DECLARATION -> FUNCTION DECLARATION_REST*/
        if (FUNCTION(myID)) {
            if (DECLARATION_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("FUNCTION");
        return false;
    }
    
    bool DECLARATION_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "DECLARATION_REST", "non-terminal");

        printDebug("DECLARATION_REST -> ( PARAMS ) { STMT_LIST }");
        /* DECLARATION_REST -> ( PARAMS ) { STMT_LIST }  */
        if (checkToken("(")) {
            int openParID = currentID++;
            addNode(openParID, myID, "(", "terminal");
            goNextToken();
            if (PARAMS(myID)) {
                if (checkToken(")")) {
                    int closeParID = currentID++;
                    addNode(closeParID, myID, ")", "terminal");
                    goNextToken();
                    if (checkToken("{")) {
                        int openBraceID = currentID++;
                        addNode(openBraceID, myID, "{", "terminal");
                        goNextToken();
                        if (STMT_LIST(myID)) {
                            if (checkToken("}")) {
                                int closeBraceID = currentID++;
                                addNode(closeBraceID, myID, "}", "terminal");
                                goNextToken();
                                guard.commit();
                                return true;
                            }
                        }
                    }
                }
            }
        } else {
            printDebug("DECLARATION_REST -> VAR_DECL");
        /* DECLARATION_REST -> VAR_DECL  */
            if (VAR_DECL(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("VAR_DECL");
        return false;
    }

    bool FUNCTION(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "FUNCTION", "non-terminal");

        printDebug("FUNCTION -> TYPE IDENTIFIER");
        /* FUNCTION -> TYPE IDENTIFIER */
        if (TYPE(myID)) {
            if (IDENTIFIER(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("TYPE");
        return false;
    }

    bool TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "TYPE", "non-terminal");

        printDebug("TYPE -> INT_TYPE TYPE_REST");
        /* TYPE -> INT_TYPE TYPE_REST */
        if (INT_TYPE(myID)) {
            if (TYPE_REST(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("TYPE -> BOOL_TYPE TYPE_REST");
        /* TYPE -> BOOL_TYPE TYPE_REST */
            if (BOOL_TYPE(myID)) {
                if (TYPE_REST(myID)) {
                    guard.commit();
                    return true;
                }
            }
            else {
                printDebug("TYPE -> CHAR_TYPE TYPE_REST");
        /* TYPE -> CHAR_TYPE TYPE_REST*/
                if (CHAR_TYPE(myID)) {
                    if (TYPE_REST(myID)) {
                        guard.commit();
                        return true;
                    }
                }
                else {
                    printDebug("TYPE -> STRING_TYPE TYPE_REST");
        /* TYPE -> STRING_TYPE TYPE_REST*/
                    if (STRING_TYPE(myID)) {
                        if (TYPE_REST(myID)) {
                            guard.commit();
                            return true;
                        }
                    }
                    else {
                        printDebug("TYPE -> VOID_TYPE TYPE_REST");
        /* TYPE -> VOID_TYPE TYPE_REST */
                        if (VOID_TYPE(myID)) {
                            if (TYPE_REST(myID)) {
                                guard.commit();
                                return true;
                            }
                        }
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("VOID_TYPE");
        return false;
    }

    bool TYPE_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "TYPE_REST", "non-terminal");

        printDebug("TYPE_REST -> [ ] TYPE_REST");
        /* TYPE_REST -> [ ] TYPE_REST */  
        if (checkToken("[")) {
            int openBracketID = currentID++;
            addNode(openBracketID, myID, "[", "terminal");
            goNextToken();
            if (checkToken("]")) {
                int closeBracketID = currentID++;
                addNode(closeBracketID, myID, "]", "terminal");
                goNextToken();
                if (TYPE_REST(myID)) {
                    guard.commit();
                    return true;
                }
            }
        } else {
            printDebug("TYPE_REST -> EOP");
        /* TYPE_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("[");
        return false;
    }

    bool PARAMS(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "PARAMS", "non-terminal");

        printDebug("PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> INT_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
        if (INT_TYPE(myID)) {
            if (TYPE_REST(myID)) {
                if (IDENTIFIER(myID)) {
                    if (PARAMS_REST(myID)) {
                        guard.commit();
                        return true;
                    }
                }
            }
        } else {
            printDebug("PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> BOOL_TYPE TYPE_REST IDENTIFIER PARAMS_REST*/
            if (BOOL_TYPE(myID)) {
                if (TYPE_REST(myID)) {
                    if (IDENTIFIER(myID)) {
                        if (PARAMS_REST(myID)) {
                            guard.commit();
                            return true;
                        }
                    }
                }
            } else {
                printDebug("PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> CHAR_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                if (CHAR_TYPE(myID)) {
                    if (TYPE_REST(myID)) {
                        if (IDENTIFIER(myID)) {
                            if (PARAMS_REST(myID)) {
                                guard.commit();
                                return true;
                            }
                        }
                    }
                } else {
                    printDebug("PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> STRING_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                    if (STRING_TYPE(myID)) {
                        if (TYPE_REST(myID)) {
                            if (IDENTIFIER(myID)) {
                                if (PARAMS_REST(myID)) {
                                    guard.commit();
                                    return true;
                                }
                            }
                        }
                    } else {
                        printDebug("PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST");
        /* PARAMS -> VOID_TYPE TYPE_REST IDENTIFIER PARAMS_REST */
                        if (VOID_TYPE(myID)) {
                            if (TYPE_REST(myID)) {
                                if (IDENTIFIER(myID)) {
                                    if (PARAMS_REST(myID)) {
                                        guard.commit();
                                        return true;
                                    }
                                }
                            }
                        } else {   
                            printDebug("PARAMS -> EOP");
        /* PARAMS -> EOP */
                            if (EOP(myID)) {
                                guard.commit();
                                return true;
                            }
                        }
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("VOID_TYPE");
        return false;
    }

    bool PARAMS_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "PARAMS_REST", "non-terminal");

        printDebug("PARAMS_REST -> , PARAMS");
        /* PARAMS_REST -> , PARAMS */
        if (checkToken(",")) {
            int commaID = currentID++;
            addNode(commaID, myID, ",", "terminal");
            goNextToken();
            if (PARAMS(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("PARAMS_REST -> EOP");
        /* PARAMS_REST -> EOP  */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError(",");
        return false;
    }

    bool VAR_DECL(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "VAR_DECL", "non-terminal");

        printDebug("VAR_DECL -> ;");
        /* VAR_DECL-> ; */
        if (checkToken(";")) {
            int semicolonID = currentID++;
            addNode(semicolonID, myID, ";", "terminal");
            goNextToken();
            guard.commit();
            return true;
        } else  {
            printDebug("VAR_DECL -> = EXPRESSION ;");
        /* VAR_DECL-> = EXPRESSION ; */
            if (checkToken("=")) {
                int equalID = currentID++;
                addNode(equalID, myID, "=", "terminal");
                goNextToken();
                if (EXPRESSION(myID)) {
                    if (checkToken(";")) {
                        int semicolonID = currentID++;
                        addNode(semicolonID, myID, ";", "terminal");
                        goNextToken();
                        guard.commit();
                        return true;
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("EXPRESSION");
        return false;
    }

    bool STMT_LIST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "STMT_LIST", "non-terminal");

        printDebug("STMT_LIST -> STATEMENT STMT_LIST_REST");
        /* STMT_LIST -> STATEMENT STMT_LIST_REST */
        if (STATEMENT(myID)) {
            if (STMT_LIST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("STATEMENT");
        return false;
    };

    bool STMT_LIST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "STMT_LIST_REST", "non-terminal");

        printDebug("STMT_LIST_REST -> STATEMENT STMT_LIST_REST");
        /* STMT_LIST_REST -> STATEMENT STMT_LIST_REST */
        if (STATEMENT(myID)) {
            if (STMT_LIST_REST(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("STMT_LIST_REST -> EOP");
        /* STMT_LIST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("STATEMENT");
        return false;
    }

    bool STATEMENT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "STATEMENT", "non-terminal");

        /* STATEMENT -> FUNCTIONVAR_DECL */
        printDebug("STATEMENT -> FUNCTION VAR_DECL");
        if (FUNCTION(myID)) {
            if (VAR_DECL(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("STATEMENT -> IF_STMT");
        /* STATEMENT -> IF_STMT */
            if (IF_STMT(myID)) {
                guard.commit();
                return true;
            } else {
                printDebug("STATEMENT -> FOR_STMT");
        /* STATEMENT -> FOR_STMT */
                if (FOR_STMT(myID)) {
                    guard.commit();
                    return true;
                } else {
                    printDebug("STATEMENT -> RETURN_STMT");
        /* STATEMENT -> RETURN_STMT */
                    if (RETURN_STMT(myID)) {
                        guard.commit();
                        return true;
                    } else {
                        printDebug("STATEMENT -> EXPR_STMT");
        /* STATEMENT -> EXPR_STMT */
                        if (EXPR_STMT(myID)) {
                            guard.commit();
                            return true;
                        } else {
                            printDebug("STATEMENT -> PRINT_STMT");
        /* STATEMENT -> PRINT_STMT */
                            if (PRINT_STMT(myID)) {
                                guard.commit();
                                return true;
                            } else {
                                printDebug("{ STMT_LIST }");
        /* STATEMENT -> { STMT_LIST } */
                                if (checkToken("{")) {
                                    int openCurlyBracesID = currentID++;
                                    addNode(openCurlyBracesID, myID, "{", "terminal");
                                    goNextToken();
                                    if (STMT_LIST(myID)) {
                                        if (checkToken("}")) {
                                            int closeCurlyBracesID = currentID++;
                                            addNode(closeCurlyBracesID, myID, "}", "terminal");
                                            goNextToken();
                                            guard.commit();
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
        reportError("Error");
        recoverFromError("{");
        return false;
    }

    bool IF_STMT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "IF_STMT", "non-terminal");

        printDebug("IF_STMT -> if ( EXPRESSION ) { STMT_LIST } IF_STMT_REST");
        /* IF_STMT -> if ( EXPRESSION ) { STMT_LIST } IF_STMT_REST */
        if (checkToken("if")) {
            int ifID = currentID++;
            addNode(ifID, myID, "if", "terminal");
            goNextToken();
            if (checkToken("(")) {
                int openParID = currentID++;
                addNode(openParID, myID, "(", "terminal");
                goNextToken();
                if (EXPRESSION(myID)) {
                    if (checkToken(")")) {
                        int closeParID = currentID++;
                        addNode(closeParID, myID, ")", "terminal");
                        goNextToken();
                        if (checkToken("{")) {
                            int openCurlyBracesID = currentID++;
                            addNode(openCurlyBracesID, myID, "{", "terminal");
                            goNextToken();
                            if (STMT_LIST(myID)) {                            
                                if (checkToken("}")) {
                                    int closeCurlyBracesID = currentID++;
                                    addNode(closeCurlyBracesID, myID, "}", "terminal");
                                    goNextToken();
                                    if (IF_STMT_REST(myID)) {
                                        guard.commit();
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("if");
        return false;
    }

    bool IF_STMT_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "IF_STMT_REST", "non-terminal");

        printDebug("IF_STMT_REST -> else { STMT_LIST }");
        /* IF_STMT_REST -> else { STMT_LIST } */
        if (checkToken("else")) {
            int elseID = currentID++;
            addNode(elseID, myID, "else", "terminal");
            goNextToken();
            if (checkToken("{")) {
                int openCurlyBracesID = currentID++;
                addNode(openCurlyBracesID, myID, "{", "terminal");
                goNextToken();
                if (STMT_LIST(myID)) {
                    if (checkToken("}")) {
                        int closeCurlyBracesID = currentID++;
                        addNode(closeCurlyBracesID, myID, "}", "terminal");
                        goNextToken();
                        guard.commit();
                        return true;
                    }
                }
            }
        } else {
            printDebug("IF_STMT_REST -> EOP");
        /* IF_STMT_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("else");
        return false;
    }

    bool FOR_STMT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "FOR_STMT", "non-terminal");

        printDebug("FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STMT_LIST }");
        /* FOR_STMT -> for ( EXPR_STMT EXPRESSION ; EXPR_STMT ) { STMT_LIST } */
        if (checkToken("for")) {
            int forID = currentID++;
            addNode(forID, myID, "for", "terminal");
            goNextToken();
            if (checkToken("(")) {
                int openParID = currentID++;
                addNode(openParID, myID, "(", "terminal");
                goNextToken();
                if (EXPR_STMT(myID)) {
                    if (EXPRESSION(myID)) {
                        if (checkToken(";")) {
                            int semicolonID = currentID++;
                            addNode(semicolonID, myID, ";", "terminal");
                            goNextToken();
                            if (EXPR_STMT(myID)) {
                                if (checkToken(")")) {
                                    int closeParID = currentID++;
                                    addNode(closeParID, myID, ")", "terminal");
                                    goNextToken();
                                    if (checkToken("{")) {
                                        int openCurlyBracesID = currentID++;
                                        addNode(openCurlyBracesID, myID, "{", "terminal");
                                        goNextToken();
                                        if (STMT_LIST(myID)) {
                                            if (checkToken("}")) {
                                                int closeCurlyBracesID = currentID++;
                                                addNode(closeCurlyBracesID, myID, "}", "terminal");
                                                goNextToken();
                                                guard.commit();
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
        reportError("Error");
        recoverFromError("for");
        return false;
    }

    bool RETURN_STMT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "RETURN_STMT", "non-terminal");

        printDebug("RETURN_STMT -> return EXPRESSION ;");
        /* RETURN_STMT -> return EXPRESSION ; */
        if (checkToken("return")) {
            int returnID = currentID++;
            addNode(returnID, myID, "return", "terminal");
            goNextToken();
            if (EXPRESSION(myID)) {
                if (checkToken(";")) {
                    int semicolonID = currentID++;
                    addNode(semicolonID, myID, ";", "terminal");
                    goNextToken();
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("return");
        return false;
    }

    bool PRINT_STMT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "PRINT_STMT", "non-terminal");

        printDebug("PRINT_STMT -> print ( EXPR_LIST ) ;");
        /* PRINT_STMT -> print ( EXPR_LIST ) ; */
        if (checkToken("print")) {
            int printID = currentID++;
            addNode(printID, myID, "print", "terminal"); 
            goNextToken();
            if (checkToken("(")) {
                int openParID = currentID++;
                addNode(openParID, myID, "(", "terminal");
                goNextToken();
                if (EXPR_LIST(myID)) {
                    if (checkToken(")")) {
                        int closeParID = currentID++;
                        addNode(closeParID, myID, ")", "terminal");
                        goNextToken();
                        if (checkToken(";")) {
                            int semicolonID = currentID++;
                            addNode(semicolonID, myID, ";", "terminal");
                            goNextToken();
                            guard.commit();
                            return true;
                        }
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("print");
        return false;
    }

    bool EXPR_STMT(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR_STMT", "non-terminal");

        printDebug("EXPR_STMT -> EXPRESSION ;");
        /* EXPR_STMT -> EXPRESSION ; */
        if (EXPRESSION(myID)) {
            if (checkToken(";")) {
                int semicolonID = currentID++;
                addNode(semicolonID, myID, ";", "terminal");
                goNextToken();
                guard.commit();
                return true;
            }
        } else {
            printDebug("EXPR_STMT -> ;");
            /* EXPR_STMT -> ; */
            if (checkToken(";")) {
                int semicolonID = currentID++;
                addNode(semicolonID, myID, ";", "terminal");
                goNextToken();
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("EXPRESSION");
        return false;
    }

    bool EXPR_LIST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR_LIST", "non-terminal");

        printDebug("EXPR_LIST -> EXPRESSION EXPR_LIST_REST");
        /* EXPR_LIST -> EXPRESSION EXPR_LIST_REST */
        if (EXPRESSION(myID)) {
            if (EXPR_LIST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("EXPRESSION");
        return false;
    }

    bool EXPR_LIST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR_LIST_REST", "non-terminal");

        printDebug("EXPR_LIST_REST -> , EXPR_LIST");
        /* EXPR_LIST_REST -> , EXPR_LIST */
        if (checkToken(",")) {
            int commaID = currentID++;
            addNode(commaID, myID, ",", "terminal");
            goNextToken();
            if (EXPR_LIST(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("EXPR_LIST_REST -> EOP");
        /* EXPR_LIST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError(",");
        return false;
    }

    bool EXPRESSION(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPRESSION", "non-terminal");

        printDebug("EXPRESSION -> IDENTIFIER = EXPRESSION");
        /* EXPRESSION -> IDENTIFIER = EXPRESSION */
        {
            ScopeGuard assignmentGuard(*this);
            int savedIndex = index;
            
            if (IDENTIFIER(myID)) {
                if (checkToken("=")) {
                    int equalID = currentID++;
                    addNode(equalID, myID, "=", "terminal");
                    goNextToken();
                    
                    if (EXPRESSION(myID)) {
                        assignmentGuard.commit();
                        guard.commit();
                        return true;
                    }
                }
            }

            index = savedIndex;
        }

        {
            ScopeGuard orExprGuard(*this);
            printDebug("EXPRESSION -> OR_EXPR");
        /* EXPRESSION -> OR_EXPR */

            addNode(myID, parentID, "EXPRESSION", "non-terminal");
            
            if (OR_EXPR(myID)) {
                orExprGuard.commit();
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("OR_EXPR");
        return false;
    }

    bool OR_EXPR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "OR_EXPR", "non-terminal");

        printDebug("OR_EXPR -> AND_EXPR OR_EXPR_REST");
        /* OR_EXPR -> AND_EXPR OR_EXPR_REST */
        if (AND_EXPR(myID)) {
            if (OR_EXPR_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("AND_EXPR");
        return false;
    }

    bool OR_EXPR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "OR_EXPR_REST", "non-terminal");

        printDebug("OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST");
        /* OR_EXPR_REST -> || AND_EXPR OR_EXPR_REST */
        if (checkToken("||")) {
            int orID = currentID++;
            addNode(orID, myID, "||", "terminal");
            goNextToken();
            if (AND_EXPR(myID)) {
                if (OR_EXPR_REST(myID)) {
                    guard.commit();
                    return true;
                }
            }
        } else {
            printDebug("OR_EXPR_REST -> EOP");
        /* OR_EXPR_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("OR_EXPR_REST");
        return false;
    }

    bool AND_EXPR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "AND_EXPR", "non-terminal");

        printDebug("AND_EXPR -> EQ_EXPR AND_EXPR_REST");
        /* AND_EXPR -> EQ_EXPR AND_EXPR_REST */
        if (EQ_EXPR(myID)) {
            if (AND_EXPR_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("AND_EXPR");
        return false;
    }

    bool AND_EXPR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "AND_EXPR_REST", "non-terminal");

        printDebug("AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST");
        /* AND_EXPR_REST -> && EQ_EXPR AND_EXPR_REST */
        if (checkToken("&&")) {
            int andID = currentID++;
            addNode(andID, myID, "&&", "terminal");
            goNextToken();
            if (EQ_EXPR(myID)) {
                if (AND_EXPR_REST(myID)) {
                    guard.commit();
                    return true;
                }
            }
        } else {
            printDebug("AND_EXPR_REST -> EOP");
        /* AND_EXPR_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("&&");
        return false;
    }

    bool EQ_EXPR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EQ_EXPR", "non-terminal");

        printDebug("EQ_EXPR -> REL_EXPR EQ_EXPR_REST_REST");
        /* EQ_EXPR -> EXPR EQ_EXPR_REST_REST */
        if (REL_EXPR(myID)) {
            if (EQ_EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("REL_EXPR");
        return false;
    }

    bool EQ_EXPR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EQ_EXPR_REST", "non-terminal");

        printDebug("EQ_EXPR_REST -> == REL_EXPR");
        /* EQ_EXPR_REST -> == REL_EXPR*/
        if (checkToken("==")) {
            int equalID = currentID++;
            addNode(equalID, myID, "==", "terminal");
            goNextToken();
            if (REL_EXPR(myID)) {
                if (EQ_EXPR_REST(myID)) {
                    guard.commit();
                    return true;
                }
            }
        } else {
            printDebug("EQ_EXPR_REST -> != REL_EXPR");
        /* EQ_EXPR_REST -> != REL_EXPR*/
            if (checkToken("!=")) {
                int notEqualID = currentID++;
                addNode(notEqualID, myID, "!=", "terminal");
                goNextToken();
                if (REL_EXPR(myID)) {
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("!=");
        return false;
    }

    bool EQ_EXPR_REST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EQ_EXPR_REST_REST", "non-terminal");

        printDebug("EQ_EXPRT_REST_REST -> EQ_EXPR_REST EQ_EXPR_REST_REST");
        /* EQ_EXPR_REST_REST -> EQ_EXPR_REST EQ_EXPR_REST_REST */
        if (EQ_EXPR_REST(myID)) {
            if (EQ_EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        else {
            printDebug("EQ_EXPR_REST_REST -> EOP");
        /* EQ_EXPR_REST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("EQ_EXPR_REST");
        return false;
    }

    bool REL_EXPR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "REL_EXPR", "non-terminal");

        printDebug("REL_EXPR -> EXPR REL_EXPR_REST_REST");
        /* REL_EXPR -> EXPR REL_EXPR_REST_REST */
        if (EXPR(myID)) {
            if (REL_EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("EXPR");
        return false;
    }

    bool REL_EXPR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "REL_EXPR_REST", "non-terminal");

        printDebug("REL_EXPR_REST -> < EXPR");
        /* REL_EXPR_REST -> < EXPR */
        if (checkToken("<")) {
            int lessID = currentID++;
            addNode(lessID, myID, "<", "terminal");
            goNextToken();
            if (EXPR(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("REL_EXPR_REST -> > EXPR");
        /* REL_EXPR_REST -> > EXPR */
            if (checkToken(">")) {
                int greaterID = currentID++;
                addNode(greaterID, myID, ">", "terminal");
                goNextToken();
                if (EXPR(myID)) {
                    guard.commit();
                    return true;
                }
            } else {
                printDebug("REL_EXPR_REST -> <= EXPR");
        /* REL_EXPR_REST -> <= EXPR */
                if (checkToken("<=")) {
                    int lessOrEqualID = currentID++;
                    addNode(lessOrEqualID, myID, "<=", "terminal");
                    goNextToken();
                    if (EXPR(myID)) {
                        guard.commit();
                        return true;
                    }
                } else {
                    printDebug("REL_EXPR_REST -> >= EXPR");
        /* REL_EXPR_REST -> >= EXPR */
                    if (checkToken(">=")) {
                        int greaterOrEqualID = currentID++;
                        addNode(greaterOrEqualID, myID, ">=", "terminal");
                        goNextToken();
                        if (EXPR(myID)) {
                            guard.commit();
                            return true;
                        }
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError(">=");
        return false;
    }

    bool REL_EXPR_REST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "REL_EXPR_REST_REST", "non-terminal");

        printDebug("REL_EXPR_REST_REST -> REL_EXPR_REST REL_EXPR_REST_REST");
        /* REL_EXPR_REST_REST -> REL_EXPR_REST REL_EXPR_REST_REST */
        if (REL_EXPR_REST(myID)) {
            if (REL_EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        else {
            printDebug("REL_EXPR_REST_REST -> EOP");
        /* EXPR_REST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("REL_EXPR_REST");
        return false;
    }

    bool EXPR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR", "non-terminal");

        printDebug("EXPR -> TERM EXPR_REST_REST");
        /* EXPR -> TERM EXPR_REST_REST */
        if (TERM(myID)) {
            if (EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("TERM");
        return false;
    }

    bool EXPR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR_REST", "non-terminal");

        printDebug("EXPR_REST -> + TERM");
        /* EXPR_REST -> + TERM */
        if (checkToken("+")) {
            int plusID = currentID++;
            addNode(plusID, myID, "+", "terminal");
            goNextToken();
            if (TERM(myID)) {
                guard.commit();
                return true;
            }
        }
        else {
            printDebug("EXPR_REST -> - TERM");
            if (checkToken("-")) {
                int minusID = currentID++;
                addNode(minusID, myID, "-", "terminal");
                goNextToken(); 
                if (TERM(myID)) {
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("-");
        return false;
    }

    bool EXPR_REST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "EXPR_REST_REST", "non-terminal");

        printDebug("EXPR_REST_REST -> EXPR_REST EXPR_REST_REST");
        /* EXPR_REST_REST -> EXPR_REST EXPR_REST_REST */
        if (EXPR_REST(myID)) {
            if (EXPR_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        else {
            printDebug("EXPR_REST_REST -> EOP");
        /* EXPR_REST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("EXPR_REST");
        return false;
    }

    bool TERM(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "TERM", "non-terminal");

        printDebug("TERM -> UNARY TERM_REST_REST");
        /* TERM -> UNARY TERM_REST_REST */
        if (UNARY(myID)) {
            if (TERM_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError("UNARY");
        return false;
    }

    bool TERM_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "TERM_REST", "non-terminal");

        printDebug("TERM_REST -> * UNARY");
        /* TERM_REST -> * UNARY */
        if (checkToken("*")) {
            int timesID = currentID++;
            addNode(timesID, myID, "*", "terminal");
            goNextToken();
            if (UNARY(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("TERM_REST -> / UNARY");
        /* TERM_REST -> / UNARY */
            if (checkToken("/")) {
                int dividedID = currentID++;
                addNode(dividedID, myID, "/", "terminal");
                goNextToken();
                if (UNARY(myID)) {
                    guard.commit();
                    return true;
                }
            } else {
                printDebug("TERM_REST -> % UNARY");
        /* TERM_REST -> % UNARY */
                if (checkToken("%")) {
                    int moduleID = currentID++;
                    addNode(moduleID, myID, "%", "terminal");
                    goNextToken();
                    if (UNARY(myID)) {
                        guard.commit();
                        return true;
                    }
                }
            }
        }
        reportError("Error");
        recoverFromError("%");
        return false;
    }

    bool TERM_REST_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "TERM_REST_REST", "non-terminal");

        printDebug("TERM_REST_REST -> TERM_REST TERM_REST_REST");
    /* TERM_REST_REST -> TERM_REST TERM_REST_REST */
        if (TERM_REST(myID)) {
            if (TERM_REST_REST(myID)) {
                guard.commit();
                return true;
            }
        }
        else {
            printDebug("TERM_REST_REST -> EOP");
        /* TERM_REST_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        } 
        reportError("Error");
        recoverFromError("TERM_REST");
        return false;  
    }

    bool UNARY(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "UNARY", "non-terminal");

        printDebug("UNARY -> ! UNARY");
        /* UNARY -> ! UNARY */
        if (checkToken("!")) {
            int notID = currentID++;
            addNode(notID, myID, "!", "terminal");
            goNextToken();
            if (UNARY(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("UNARY -> - UNARY");
        /* UNARY -> - UNARY */
            if (checkToken("-")) {
                int minusID = currentID++;
                addNode(minusID, myID, "-", "terminal");
                goNextToken();
                if (UNARY(myID)) {
                    guard.commit();
                    return true;
                }
            } else {
                printDebug("UNARY -> FACTOR");
        /* UNARY -> FACTOR */
                if (FACTOR(myID)) {
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("FACTOR");
        return false;
    }

    bool FACTOR(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "FACTOR", "non-terminal");

        printDebug("FACTOR -> IDENTIFIER FACTOR_REST");
        /* FACTOR -> IDENTIFIER FACTOR_REST */
        if (IDENTIFIER(myID)) {
            if (FACTOR_REST(myID)) {
                guard.commit();
                return true;
            }
        } else {
            printDebug("FACTOR -> INT_LITERAL FACTOR_REST");
        /* FACTOR -> INT_LITERAL FACTOR_REST */
            if (INT_LITERAL(myID)) {
                if (FACTOR_REST(myID)) {
                    guard.commit();
                    return true;
                }
            } else {
                printDebug("FACTOR -> CHAR_LITERAL FACTOR_REST");
        /* FACTOR -> CHAR_LITERAL FACTOR_REST */
                if (CHAR_LITERAL(myID)) {
                    if (FACTOR_REST(myID)) {
                        guard.commit();
                        return true;
                    }
                } else {
                    printDebug("FACTOR -> STRING_LITERAL FACTOR_REST");
        /* FACTOR -> STRING_LITERAL FACTOR_REST */
                    if (STRING_LITERAL(myID)) {
                        if (FACTOR_REST(myID)) {
                            guard.commit();
                            return true;
                        }
                    } else {
                        printDebug("FACTOR -> BOOL_LITERAL FACTOR_REST");
        /* FACTOR -> BOOL_LITERAL FACTOR_REST */
                        if (BOOL_LITERAL(myID)) {
                            if (FACTOR_REST(myID)) {
                                guard.commit();
                                return true;
                            }
                        } else {
                            printDebug("FACTOR -> ( EXPRESSION ) FACTOR_REST");
        /* FACTOR -> ( EXPRESSION ) FACTOR_REST */
                            if (checkToken("(")) {
                                int openParID = currentID++;
                                addNode(openParID, myID, "(", "terminal");
                                goNextToken();
                                if (EXPRESSION(myID)) {
                                    if (checkToken(")")) {
                                        int closeParID = currentID++;
                                        addNode(closeParID, myID, ")", "terminal");
                                        goNextToken();
                                        if (FACTOR_REST(myID)) {
                                            guard.commit();
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
        reportError("Error");
        recoverFromError("(");
        return false;
    }

    bool FACTOR_REST(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "FACTOR_REST", "non-terminal");

        printDebug("FACTOR_REST -> [ EXPRESSION ] FACTOR_REST");
        /* FACTOR_REST -> [ EXPRESSION ] FACTOR_REST */
        if (checkToken("[")) {
            int openBracesID = currentID++;
            addNode(openBracesID, myID, "[", "terminal");
            goNextToken();
            if (EXPRESSION(myID)) {
                if (checkToken("]")) {
                    int closeBracesID = currentID++;
                    addNode(closeBracesID, myID, "]", "terminal");
                    goNextToken();
                    if (FACTOR_REST(myID)) {
                        guard.commit();
                        return true;
                    }
                }
            }
        } else {
            printDebug("FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST");
        /* FACTOR_REST -> ( EXPR_LIST ) FACTOR_REST */
            if (checkToken("(")) {
                int openParID = currentID++;
                addNode(openParID, myID, "(", "terminal");
                goNextToken();
                if (EXPR_LIST(myID)) {
                    if (checkToken(")")) {
                        int closeParID = currentID++;
                        addNode(closeParID, myID, ")", "terminal");
                        goNextToken();
                        if (FACTOR_REST(myID)) {
                            guard.commit();
                            return true;
                        }
                    }
                }
            } else {
                printDebug("FACTOR_REST -> EOP");
        /* FACTOR_REST -> EOP */
                if (EOP(myID)) {
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError("(");
        return false;
    }

    bool EOP(int parentID) {
        return true;
    }

    bool IDENTIFIER(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "IDENTIFIER", "non-terminal");

        printDebug("IDENTIFIER -> <Check Token Type>");
        /* IDENTIFIER -> <Check Token Type> */
        if (checkTokenType("IDENTIFIER")) {
            int identifierID = currentID++;
            addNode(identifierID, myID, get<1>(tokens[index]), "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError(";");
        return false;
    }

    bool INT_TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "INT_TYPE", "non-terminal");

        printDebug("INT_TYPE -> integer");
        /* INT_TYPE -> integer */
        if (checkToken("integer")) {
            int integerID = currentID++;
            addNode(integerID, myID, "integer", "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError("integer");
        return false;
    }

    bool BOOL_TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "BOOL_TYPE", "non-terminal");

        printDebug("BOOL_TYPE -> boolean");
        /* BOOL_TYPE -> boolean */
        if (checkToken("boolean")) {
            int booleanID = currentID++;
            addNode(booleanID, myID, "boolean", "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError("boolean");
        return false;
    }

    bool CHAR_TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "CHAR_TYPE", "non-terminal");

        printDebug("CHAR_TYPE -> char");
        /* CHAR_TYPE -> char */
        if (checkToken("char")) {
            int charID = currentID++;
            addNode(charID, myID, "char", "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError("char");
        return false;
    }

    bool STRING_TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "STRING_TYPE", "non-terminal");

        printDebug("STRING_TYPE -> string");
        /* STRING_TYPE -> string */
        if (checkToken("string")) {
            int stringID = currentID++;
            addNode(stringID, myID, "string", "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError("string");
        return false;
    }

    bool VOID_TYPE(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "VOID_TYPE", "non-terminal");

        printDebug("VOID_TYPE -> void");
        /* VOID_TYPE -> void */
        if (checkToken("void")) {
            int voidID = currentID++;
            addNode(voidID, myID, "void", "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError("void");
        return false;
    }

    bool INT_LITERAL(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "INT_LITERAL", "non-terminal");

        printDebug("INT_LITERAL -> <Check Token Type>");
        /* INT_LITERAL -> <Check Token Type> */
        if (checkTokenType("INTEGER")) {
            int integerID = currentID++;
            addNode(integerID, myID, get<1>(tokens[index]), "terminal");
            goNextToken();
            guard.commit();
            return true;
        }
        reportError("Error");
        recoverFromError(";");
        return false;
    }

    bool CHAR_LITERAL(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "CHAR_LITERAL", "non-terminal");

        printDebug("CHAR_LITERAL -> ' <Check Token Type> '");
        /* CHAR_LITERAL -> ' <Check Token Type> ' */
        if (checkToken("'")) {
            int openSingleQuouteID = currentID++;
            addNode(openSingleQuouteID, myID, "'", "terminal");
            goNextToken();
            if (checkTokenType("CHAR")) {
                int charID = currentID++;
                addNode(charID, myID, get<1>(tokens[index]), "terminal");
                goNextToken();
                if (checkToken("'")) {
                    int closeSingleQuoteID = currentID++;
                    addNode(closeSingleQuoteID, myID, "'", "terminal");          
                    goNextToken();
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError(";");
        return false;
    }

    bool STRING_LITERAL(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "STRING_LITERAL", "non-terminal");

        printDebug("STRING_LITERAL -> \" <Check Token Type> \"");
        /* STRING_LITERAL -> " <Check Token Type> " */
        if (checkToken("\"")) {
            int openDoubleQuouteID = currentID++;
            addNode(openDoubleQuouteID, myID, "\"", "terminal");
            goNextToken();
            if (checkTokenType("STRING")) {
                int stringID = currentID++;
                addNode(stringID, myID, get<1>(tokens[index]), "terminal");
                goNextToken();
                if (checkToken("\"")) {
                    int closeDoubleQuouteID = currentID++;
                    addNode(closeDoubleQuouteID, myID, "\"", "terminal");
                    goNextToken();
                    guard.commit();
                    return true;
                }
            }
        }
        reportError("Error");
        recoverFromError(";");
        return false;
    }

    bool BOOL_LITERAL(int parentID) {
        ScopeGuard guard(*this);
        int myID = currentID++;
        addNode(myID, parentID, "BOOL_LITERAL", "non-terminal");

        printDebug("BOOL_LITERAL -> true");
        /* BOOL_LITERAL -> true */
        if (checkToken("true")) {
            int trueID = currentID++;
            addNode(trueID, myID, "true", "terminal");
            goNextToken();
            guard.commit();
            return true;
        } else {
            printDebug("BOOL_LITERAL -> false");
        /* BOOL_LITERAL -> false */
            if (checkToken("false")) {
                int falseID = currentID++;
                addNode(falseID, myID, "false", "terminal");
                goNextToken();
                guard.commit();
                return true;
            }
        }
        reportError("Error");
        recoverFromError(";");
        return false;
    }
};

int main() {
    Scanner scanner;

    vector<vector<char>> buffer = scanner.readBMMFile("input4.bmm");

    // for (int i = 0; i < buffer.size(); i++) {
    //     for (int j = 0; j < buffer[i].size(); j++) {
    //         cout << buffer[i][j];
    //     }
    // }

    //cout<<"------EMPEZANDO EL SCANNER..."<<endl;
    vector<tuple<string, string, int, int>> tokens = scanner.Tokenize(buffer);

    if(errores_encontrados.size()<=0) {
        cout<<"Successful scanner"<<endl;
    }
    else {
        // cout<<"------SCANNER TERMINADO CON "<<errores_encontrados.size() <<" ERRORES"<<endl;
        // for(int i =0 ; i <errores_encontrados.size();i++)
        // {
        //     cout << get<0>(errores_encontrados[i])  << get<1>(errores_encontrados[i]) << "' en fila " 
        //              << get<2>(errores_encontrados[i]) << ", columna " << get<3>(errores_encontrados[i])-1<< endl;
        // }
    }
    // for (int i = 0; i < tokens.size(); i++) {
    //     printf("Token: %-20s Value: %-20s Fila: %-20d Columna: %-20d\n", 
    //    get<0>(tokens[i]).c_str(), get<1>(tokens[i]).c_str(), get<2>(tokens[i]) + 1, get<3>(tokens[i]) + 1);
    // }

    
    Parser parser(tokens, false);

    return 0;
}