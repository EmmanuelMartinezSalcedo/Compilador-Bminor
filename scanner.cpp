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
#include <unordered_map>
#include <unordered_set>
using namespace std;

int parser_error = 0;
int mostrar_solo_errores = 0;

vector<tuple<string,string,int,int>> errores_encontrados;

class Scanner {
public:
    Scanner() {
        keywords = {"array", "boolean", "char", "else", "false", "for", "function", "if", 
                    "integer","map", "print", "return", "string", "true", "void", "while"};
        symbols = {",", ";", ":", "(", ")", "[", "]", "{", "}","\"","\'"};
        operators = {"++", "--", "+", "-", "*", "/", "%", "^", "&&", "||", "!", "=", "<", ">", "<=", ">=", "==", "!=", "&", "|"};
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

            // Revisar si es un caracter valido o un caracter de escape
            if (c == '\\') {
                c = getchar(buffer, row, col); // Capturar el caracter escapado
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
                getchar(buffer, row, col); // avanzar al siguiente caracter
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

        // Si el caracter es invalido, registrar error
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
    int parentID;
    string value;
    string type;
    bool hasChildren = false;
    vector<int> children;
};

void deleteNonTerminalLeafs(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<string> lines;
    string header;

    ifstream file(csvFile);
    if (!file.is_open()) {
        cout << "Error al abrir el archivo" << endl;
        return;
    }
    
    getline(file, header);
    lines.push_back(header);
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, parentIDStr, value, type;
        
        getline(ss, idStr, ',');
        getline(ss, parentIDStr, ',');
        getline(ss, value, ',');
        getline(ss, type);
        
        int id = stoi(idStr);
        int parentID = stoi(parentIDStr);
        
        Node node{id, parentID, value, type, false};
        nodes[id] = node;
        lines.push_back(line);
    }
    file.close();
    
    for (auto& pair : nodes) {
        Node& node = pair.second;
        if (node.parentID != -1) {
            nodes[node.parentID].hasChildren = true;
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    
    vector<int> toDelete;
    for (const auto& pair : nodes) {
        const Node& node = pair.second;
        if (!node.hasChildren && node.type == "non-terminal") {
            toDelete.push_back(node.id);
        }
    }
    
    ofstream outFile(csvFile);
    if (!outFile.is_open()) {
        cout << "Error al abrir el archivo para escritura" << endl;
        return;
    }
    
    outFile << header << endl;
    
    for (size_t i = 1; i < lines.size(); i++) {
        stringstream ss(lines[i]);
        string idStr;
        getline(ss, idStr, ',');
        int id = stoi(idStr);
        
        if (find(toDelete.begin(), toDelete.end(), id) == toDelete.end()) {
            outFile << lines[i] << endl;
        }
    }
    
    outFile.close();
}

void reduceTree(const string& csvFile) {
    unordered_map<int, Node> nodes;
    unordered_map<int, vector<int>> childrenMap;
    vector<Node> orderedNodes;
    ifstream file(csvFile);
    string line;
    bool firstLine = true;

    while (getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }

        stringstream ss(line);
        Node node;
        string item;

        getline(ss, item, ',');
        node.id = stoi(item);

        getline(ss, item, ',');
        node.parentID = stoi(item);

        getline(ss, item, ',');
        node.value = item;

        getline(ss, item, ',');
        node.type = item;

        nodes[node.id] = node;

        if (node.parentID != -1) {
            childrenMap[node.parentID].push_back(node.id);
        }
    }
    file.close();

    bool changes;
    do {
        changes = false;
        vector<int> nodesToRemove;

        for (const auto& [id, node] : nodes) {
            if (node.type == "non-terminal" && childrenMap[id].size() == 1) {
                int childId = childrenMap[id][0];
                Node& childNode = nodes[childId];

                childNode.parentID = node.parentID;

                if (node.parentID != -1) {
                    auto& parentChildren = childrenMap[node.parentID];
                    replace(parentChildren.begin(), parentChildren.end(), id, childId);
                }

                nodesToRemove.push_back(id);
                changes = true;
            }
        }

        for (int id : nodesToRemove) {
            nodes.erase(id);
            childrenMap.erase(id);
        }
    } while (changes);

    for (const auto& [id, node] : nodes) {
        orderedNodes.push_back(node);
    }

    sort(orderedNodes.begin(), orderedNodes.end(), [](const Node& a, const Node& b) {
        return a.id < b.id;
    });

    ofstream outFile(csvFile, ios::trunc);
    outFile << "ID,PadreID,Valor,Tipo\n";

    for (const auto& node : orderedNodes) {
        outFile << node.id << "," << node.parentID << "," << node.value << "," << node.type << "\n";
    }
}

void removeUselessSymbols(const string& csvFile) {
    unordered_map<int, Node> nodes;
    unordered_map<int, vector<int>> childrenMap;
    vector<Node> orderedNodes;
    unordered_set<string> uselessSymbols = {"quote", "comma", "{", "}", ";", "(", ")", "if", "for", "else", "'"};

    ifstream file(csvFile);
    string line;
    bool firstLine = true;

    while (getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }

        stringstream ss(line);
        Node node;
        string item;

        getline(ss, item, ',');
        node.id = stoi(item);

        getline(ss, item, ',');
        node.parentID = stoi(item);

        getline(ss, item, ',');
        node.value = item;

        getline(ss, item, ',');
        node.type = item;

        nodes[node.id] = node;

        if (node.parentID != -1) {
            childrenMap[node.parentID].push_back(node.id);
        }
    }
    file.close();

    bool changes;
    do {
        changes = false;
        vector<int> nodesToRemove;

        for (const auto& [id, node] : nodes) {
            if (uselessSymbols.find(node.value) != uselessSymbols.end()) {
                auto& children = childrenMap[id];

                for (int childId : children) {
                    if (nodes.find(childId) != nodes.end()) {
                        Node& childNode = nodes[childId];
                        childNode.parentID = node.parentID;
                        if (node.parentID != -1) {
                            childrenMap[node.parentID].push_back(childId);
                        }
                    }
                }

                if (node.parentID != -1) {
                    auto& parentChildren = childrenMap[node.parentID];
                    parentChildren.erase(
                        remove(parentChildren.begin(), parentChildren.end(), id),
                        parentChildren.end()
                    );
                }

                nodesToRemove.push_back(id);
                changes = true;
            }
        }

        for (int id : nodesToRemove) {
            nodes.erase(id);
            childrenMap.erase(id);
        }
    } while (changes);

    for (const auto& [id, node] : nodes) {
        orderedNodes.push_back(node);
    }

    sort(orderedNodes.begin(), orderedNodes.end(), [](const Node& a, const Node& b) {
        return a.id < b.id;
    });

    ofstream outFile(csvFile, ios::trunc);
    outFile << "ID,PadreID,Valor,Tipo\n";

    for (const auto& node : orderedNodes) {
        outFile << node.id << "," << node.parentID << "," << node.value << "," << node.type << "\n";
    }
}

void processEqualNodes(const string& csvFile) {
    unordered_map<int, Node> nodes;
    ifstream file(csvFile);
    string line;
    bool isHeader = true;

    while (getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        stringstream ss(line);
        string idStr, parentIDStr, value, type;
        getline(ss, idStr, ',');
        getline(ss, parentIDStr, ',');
        getline(ss, value, ',');
        getline(ss, type, ',');

        Node node;
        node.id = stoi(idStr);
        node.parentID = stoi(parentIDStr);
        node.value = value;
        node.type = type;

        nodes[node.id] = node;
    }
    file.close();

    for (auto& [id, node] : nodes) {
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(id);
            nodes[node.parentID].hasChildren = true;
        }
    }

    vector<int> nodesToRemove;
    for (auto& [id, node] : nodes) {
        if (node.value == "=") {
            int parentID = node.parentID;
            if (nodes.find(parentID) != nodes.end()) {
                Node& parentNode = nodes[parentID];
                if (parentNode.value == "VAR_DECL") {
                    nodesToRemove.push_back(id);
                } else if (parentNode.value == "EXPRESSION") {
                    parentNode.value = "=";
                    nodesToRemove.push_back(id);
                }
            }
        }
    }

    for (int id : nodesToRemove) {
        int parentID = nodes[id].parentID;
        auto& siblings = nodes[parentID].children;
        siblings.erase(remove(siblings.begin(), siblings.end(), id), siblings.end());
        nodes.erase(id);
    }

    ofstream outFile(csvFile);
    outFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outFile << node.id << "," << node.parentID << "," << node.value << "," << node.type << "\n";
    }
    outFile.close();
}

void processSymbols(const string& csvFile) {
    unordered_set<string> operators = {
        "+", "-", "*", "/", "%", "!", ">=", "<=", 
        "==", "!=", "<", ">", "&&", "||"
    };
    
    vector<Node> nodes;
    
    vector<string> lines;
    ifstream file(csvFile);
    string line;
    
    getline(file, line);
    lines.push_back(line);
    
    while (getline(file, line)) {
        istringstream ss(line);
        string token;
        Node node;

        getline(ss, token, ',');
        node.id = stoi(token);

        getline(ss, token, ',');
        node.parentID = stoi(token);

        getline(ss, token, ',');
        node.value = token;

        getline(ss, token, ',');
        node.type = token;
        
        nodes.push_back(node);
        lines.push_back(line);
    }
    file.close();
    
    vector<int> nodesToRemove;
    for (size_t i = 0; i < nodes.size(); i++) {
        if (operators.find(nodes[i].value) != operators.end()) {
            for (auto& parent : nodes) {
                if (parent.id == nodes[i].parentID) {
                    parent.value = nodes[i].value;
                    nodesToRemove.push_back(nodes[i].id);
                    break;
                }
            }
        }
    }
    
    ofstream outFile(csvFile);
    outFile << lines[0] << endl;
    
    for (size_t i = 1; i < lines.size(); i++) {
        istringstream ss(lines[i]);
        string idStr;
        getline(ss, idStr, ',');
        int currentId = stoi(idStr);
        
        bool shouldSkip = false;
        for (int removeId : nodesToRemove) {
            if (currentId == removeId) {
                shouldSkip = true;
                break;
            }
        }
        
        if (!shouldSkip) {
            for (const Node& node : nodes) {
                if (node.id == currentId) {
                    outFile << node.id << ","
                           << node.parentID << ","
                           << node.value << ","
                           << node.type << endl;
                    break;
                }
            }
        }
    }
    outFile.close();
}

void processExpr(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if ((node.value == "+" || node.value == "-") && nodes[node.parentID].value == "EXPR") {
            Node& parentExpr = nodes[node.parentID];
            
            parentExpr.children.insert(parentExpr.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentExpr.id;
            }
            
            parentExpr.value = node.value;
            parentExpr.children.erase(remove(parentExpr.children.begin(), parentExpr.children.end(), id), parentExpr.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processTerm(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;
    
    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if ((node.value == "*" || node.value == "/" || node.value == "%") && nodes[node.parentID].value == "TERM") {
            Node& parentTerm = nodes[node.parentID];
            
            parentTerm.children.insert(parentTerm.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentTerm.id;
            }
            
            parentTerm.value = node.value;
            parentTerm.children.erase(remove(parentTerm.children.begin(), parentTerm.children.end(), id), parentTerm.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processEqExpr(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;
    
    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if ((node.value == "==" || node.value == "!=") && nodes[node.parentID].value == "EQ_EXPR") {
            Node& parentEqExpr = nodes[node.parentID];
            
            parentEqExpr.children.insert(parentEqExpr.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentEqExpr.id;
            }
            
            parentEqExpr.value = node.value;
            parentEqExpr.children.erase(remove(parentEqExpr.children.begin(), parentEqExpr.children.end(), id), parentEqExpr.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processRelExpr(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;
    
    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if ((node.value == "<" || node.value == ">" || node.value == "<=" || node.value == ">=") && nodes[node.parentID].value == "REL_EXPR") {
            Node& parentRelExpr = nodes[node.parentID];
            
            parentRelExpr.children.insert(parentRelExpr.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentRelExpr.id;
            }
            
            parentRelExpr.value = node.value;
            parentRelExpr.children.erase(remove(parentRelExpr.children.begin(), parentRelExpr.children.end(), id), parentRelExpr.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processAndExpr(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;
    
    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "&&" && nodes[node.parentID].value == "AND_EXPR") {
            Node& parentAndExpr = nodes[node.parentID];
            
            parentAndExpr.children.insert(parentAndExpr.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentAndExpr.id;
            }
            
            parentAndExpr.value = node.value;
            parentAndExpr.children.erase(remove(parentAndExpr.children.begin(), parentAndExpr.children.end(), id), parentAndExpr.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processOrExpr(const string& csvFile) {
    map<int, Node> nodes;
    ifstream inputFile(csvFile);
    string line;
    
    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
            nodes[node.parentID].hasChildren = true;
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "||" && nodes[node.parentID].value == "OR_EXPR") {
            Node& parentOrExpr = nodes[node.parentID];
            
            parentOrExpr.children.insert(parentOrExpr.children.end(), node.children.begin(), node.children.end());
            for (int childID : node.children) {
                nodes[childID].parentID = parentOrExpr.id;
            }
            
            parentOrExpr.value = node.value;
            parentOrExpr.children.erase(remove(parentOrExpr.children.begin(), parentOrExpr.children.end(), id), parentOrExpr.children.end());
            nodes.erase(id);
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& [id, node] : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void processFunctions(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "FUNCTION" && !node.children.empty()) {
            auto leftChildIt = min_element(node.children.begin(), node.children.end());
            int leftChildID = *leftChildIt;

            nodes[leftChildID].parentID = node.parentID;
            
            if (node.parentID != -1) {
                auto& parentChildren = nodes[node.parentID].children;
                replace(parentChildren.begin(), parentChildren.end(), node.id, leftChildID);
            }

            for (int childID : node.children) {
                if (childID != leftChildID) {
                    nodes[childID].parentID = leftChildID;
                    nodes[leftChildID].children.push_back(childID);
                }
            }
            node.children.clear();
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "FUNCTION") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processExprList(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "EXPR_LIST" && !node.children.empty()) {
            auto leftChildIt = min_element(node.children.begin(), node.children.end());
            int leftChildID = *leftChildIt;

            nodes[leftChildID].parentID = node.parentID;

            if (node.parentID != -1) {
                auto& parentChildren = nodes[node.parentID].children;
                replace(parentChildren.begin(), parentChildren.end(), node.id, leftChildID);
            }

            for (int childID : node.children) {
                if (childID != leftChildID) {
                    nodes[childID].parentID = leftChildID;
                    nodes[leftChildID].children.push_back(childID);
                }
            }
            node.children.clear();
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "EXPR_LIST") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processReturnStmt(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "RETURN_STMT" && !node.children.empty()) {
            auto returnChildIt = find_if(node.children.begin(), node.children.end(),
                                              [&nodes](int childID) { return nodes[childID].value == "return"; });

            if (returnChildIt != node.children.end()) {
                int returnChildID = *returnChildIt;

                nodes[returnChildID].parentID = node.parentID;

                if (node.parentID != -1) {
                    auto& parentChildren = nodes[node.parentID].children;
                    replace(parentChildren.begin(), parentChildren.end(), node.id, returnChildID);
                }

                for (int childID : node.children) {
                    if (childID != returnChildID) {
                        nodes[childID].parentID = returnChildID;
                        nodes[returnChildID].children.push_back(childID);
                    }
                }
                node.children.clear();
            }
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "RETURN_STMT") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processPrintStmt(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "PRINT_STMT" && !node.children.empty()) {
            auto printChildIt = find_if(node.children.begin(), node.children.end(),
                                             [&nodes](int childID) { return nodes[childID].value == "print"; });

            if (printChildIt != node.children.end()) {
                int printChildID = *printChildIt;

                nodes[printChildID].parentID = node.parentID;

                if (node.parentID != -1) {
                    auto& parentChildren = nodes[node.parentID].children;
                    replace(parentChildren.begin(), parentChildren.end(), node.id, printChildID);
                }

                for (int childID : node.children) {
                    if (childID != printChildID) {
                        nodes[childID].parentID = printChildID;
                        nodes[printChildID].children.push_back(childID);
                    }
                }
                node.children.clear();
            }
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "PRINT_STMT") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processFactor(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "FACTOR" && !node.children.empty()) {
            auto leftChildIt = min_element(node.children.begin(), node.children.end());
            int leftChildID = *leftChildIt;

            nodes[leftChildID].parentID = node.parentID;

            if (node.parentID != -1) {
                auto& parentChildren = nodes[node.parentID].children;
                replace(parentChildren.begin(), parentChildren.end(), node.id, leftChildID);
            }

            for (int childID : node.children) {
                if (childID != leftChildID) {
                    nodes[childID].parentID = leftChildID;
                    nodes[leftChildID].children.push_back(childID);
                }
            }
            node.children.clear();
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "FACTOR") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processDeclaration(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if (node.value == "PROGRAM_REST") {
            vector<int> declarationChildren;
            for (int childID : node.children) {
                if (nodes[childID].value == "DECLARATION") {
                    declarationChildren.push_back(childID);
                }
            }

            if (!declarationChildren.empty()) {
                int leftDeclarationChildID = *min_element(declarationChildren.begin(), declarationChildren.end());

                nodes[leftDeclarationChildID].parentID = node.parentID;

                if (node.parentID != -1) {
                    auto& parentChildren = nodes[node.parentID].children;
                    replace(parentChildren.begin(), parentChildren.end(), node.id, leftDeclarationChildID);
                }

                for (int childID : node.children) {
                    if (childID != leftDeclarationChildID) {
                        nodes[childID].parentID = leftDeclarationChildID;
                        nodes[leftDeclarationChildID].children.push_back(childID);
                    }
                }
                node.children.clear();
            }
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || nodes[id].value != "PROGRAM_REST") {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void processStmt(const string& csvFile) {
    unordered_map<int, Node> nodes;
    vector<int> nodeOrder;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;

        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes[node.id] = node;
        nodeOrder.push_back(node.id);

        if (node.parentID != -1) {
            nodes[node.parentID].children.push_back(node.id);
        }
    }
    inputFile.close();

    for (auto& [id, node] : nodes) {
        if ((node.value == "STMT_LIST" || node.value == "STMT_LIST_REST") && !node.children.empty()) {
            int leftChildID = *min_element(node.children.begin(), node.children.end());

            nodes[leftChildID].parentID = node.parentID;

            if (node.parentID != -1) {
                auto& parentChildren = nodes[node.parentID].children;
                replace(parentChildren.begin(), parentChildren.end(), node.id, leftChildID);
            }

            for (int childID : node.children) {
                if (childID != leftChildID) {
                    nodes[childID].parentID = leftChildID;
                    nodes[leftChildID].children.push_back(childID);
                }
            }
            node.children.clear();
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (int id : nodeOrder) {
        if (!nodes[id].children.empty() || (nodes[id].value != "STMT_LIST" && nodes[id].value != "STMT_LIST_REST")) {
            outputFile << nodes[id].id << ',' << nodes[id].parentID << ','
                       << nodes[id].value << ',' << nodes[id].type << '\n';
        }
    }
    outputFile.close();
}

void reduceIDs(const string& csvFile) {
    vector<Node> nodes;
    map<int, int> idMapping;
    ifstream inputFile(csvFile);
    string line;

    getline(inputFile, line);
    while (getline(inputFile, line)) {
        stringstream ss(line);
        Node node;
        string temp;
        getline(ss, temp, ',');
        node.id = stoi(temp);
        getline(ss, temp, ',');
        node.parentID = stoi(temp);
        getline(ss, node.value, ',');
        getline(ss, node.type, ',');

        nodes.push_back(node);
    }
    inputFile.close();

    sort(nodes.begin(), nodes.end(), [](const Node& a, const Node& b) {
        return a.id < b.id;
    });

    int newID = 0;
    for (const auto& node : nodes) {
        if (idMapping.find(node.id) == idMapping.end()) {
            idMapping[node.id] = newID++;
        }
    }

    for (auto& node : nodes) {
        node.id = idMapping[node.id];
        if (node.parentID != -1) {
            node.parentID = idMapping[node.parentID];
        }
    }

    ofstream outputFile(csvFile);
    outputFile << "ID,PadreID,Valor,Tipo\n";
    for (const auto& node : nodes) {
        outputFile << node.id << ',' << node.parentID << ',' << node.value << ',' << node.type << '\n';
    }
    outputFile.close();
}

void createAST(const string& csvFile) {
    removeUselessSymbols(csvFile);
    deleteNonTerminalLeafs(csvFile);
    reduceTree(csvFile);
    deleteNonTerminalLeafs(csvFile);
    reduceTree(csvFile);
    processEqualNodes(csvFile);
    reduceTree(csvFile);
    processSymbols(csvFile);
    processExpr(csvFile);
    processTerm(csvFile);
    processEqExpr(csvFile);
    processRelExpr(csvFile);
    processAndExpr(csvFile);
    processOrExpr(csvFile);
    processFunctions(csvFile);
    processExprList(csvFile);
    processReturnStmt(csvFile);
    processPrintStmt(csvFile);
    processFactor(csvFile);
    //processDeclaration(csvFile);
    //processStmt(csvFile);
    reduceIDs(csvFile);
}

class Parser {
public:
    vector<tuple<string, string, int, int>> tokens;
    const char* cursor;
    int index;
    bool debug;
    int tab = 0;

    int currentID = 0;
    vector<tuple<int, int, string, string>> tempNodes;


    // Definir los tokens de sincronizacion 
    const vector<string> syncTokens = {";"};

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
            cout << "Successful parse" << endl;
            if(parser_error == 1)
            {
                cout<<"Sin crear el arbol debido a errores"<<endl;
            }
            else{
                createAST("parseTree.csv");
            cout << "AST created" << endl;

                validateSemantics("parseTree.csv");
                cout << "Semantic validation complete" << endl;
            }
            
        } else {
            cout << "Error parsing in line " << get<2>(tokens[index]) << ", column " << get<3>(tokens[index]) << ". Token: " << get<1>(tokens[index]) << endl;
        }
    }

private:

    // Método de recuperacion de errores
    void synchronize() {
        cout << "Iniciando recuperacion de error..." << endl;
        while (index < tokens.size()) {
            string currentToken = get<1>(tokens[index]);
            cout<<"Consumiendo tokens hasta sincronizar: " << currentToken<<endl;
            // Si el token actual es uno de los tokens de sincronizacion, se detiene la recuperacion
            if (find(syncTokens.begin(), syncTokens.end(), currentToken) != syncTokens.end()) {
                cout << "Recuperacion de error completa en token: " << currentToken << endl;
                // goNextToken();
                currentToken = get<1>(tokens[index]);
                
                cout << "Token actual: " << currentToken << endl;
                return;
            }
            goNextToken();
        }
        cout << "No se encontro un token de sincronizacion. Fin del analisis." << endl;
    }

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
        return (get<1>(tokens[index]) == token);
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
    
    void printRestTokens() {
        cout << "Tokens restantes:" << endl;
        for (int i = index; i < tokens.size(); i++) {
            printf("Token: %-20s Value: %-20s Fila: %-20d Columna: %-20d\n", 
                   get<0>(tokens[i]).c_str(), get<1>(tokens[i]).c_str(), get<2>(tokens[i]) + 1, get<3>(tokens[i]) + 1);
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


    struct Symbol {
        string name;
        string type;
        int scopeLevel;
        bool isFunction = false;
        vector<string> parameters; // Si es funcion
    };

    unordered_map<string, Symbol> symbolTable;

    void validateDeclaration(const string& value, const string& parentID) {
    if (symbolTable.find(parentID) != symbolTable.end()) {
        cerr << "Error semantico: Duplicacion en la declaracion de la variable '" 
             << parentID << "' de tipo " << value << endl;
        return;
    }

    Symbol symbol;
    symbol.name = parentID;
    symbol.type = value;
    symbolTable[parentID] = symbol;

        }
    vector<int> getChildren(int parentID, const string& csvFile) {
    ifstream inputFile(csvFile);
    string line;
    vector<int> children;

    if (!inputFile.is_open()) {
        cerr << "Error al abrir el archivo del AST: " << csvFile << endl;
        return children;
    }

    string id, parent, value, type;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        getline(ss, id, ',');
        getline(ss, parent, ',');
        getline(ss, value, ',');
        getline(ss, type, ',');

        if (stoi(parent) == parentID) {
            children.push_back(stoi(id));
        }
    }

    return children;
}

    string evaluateExpressionType(int expressionID, const string& csvFile) {
            ifstream inputFile(csvFile);
    string line;

    if (!inputFile.is_open()) {
        cerr << "Error al abrir el archivo del AST: " << csvFile << endl;
        return "";
    }

    // Buscar el nodo en el archivo
    string id, parentID, value, type;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        getline(ss, id, ',');
        getline(ss, parentID, ',');
        getline(ss, value, ',');
        getline(ss, type, ',');

        if (stoi(id) == expressionID) {
            // Si es un literal, devolver el tipo directamente
            if (type == "INTEGER") {
                return "integer";
            } else if (type == "STRING") {
                return "string";
            } else if (type == "CHAR") {
                return "char";
            } else if (type == "BOOLEAN") {
                return "boolean";
            }

            // Si es un identificador, verificar si existe en la tabla de simbolos
            if (type == "IDENTIFIER") {
                if (symbolTable.find(value) == symbolTable.end()) {
                    cerr << "Error semantico: Variable no declarada: '" << value << "'" << endl;
                    return "";
                }
                return symbolTable[value].type;
            }

            // Si es un operador, manejar sus operandos
            if (value == "+" || value == "-" || value == "*" || value == "/" || value == "%") {
                vector<int> children = getChildren(expressionID, csvFile);

                if (children.size() < 2) {
                    cerr << "Error semantico: Operador binario con menos de dos operandos. Nodo ID: "
                         << expressionID << endl;
                    return "";
                }

                string leftType = evaluateExpressionType(children[0], csvFile);
                string rightType = evaluateExpressionType(children[1], csvFile);

                if (leftType.empty() || rightType.empty()) {
                    return "";
                }

                if (leftType == "integer" && rightType == "integer") {
                    return "integer";
                } else {
                    cerr << "Error semantico: Tipos incompatibles para operador binario '"
                         << value << "' en Nodo ID: " << expressionID << endl;
                    return "";
                }
            }

            cerr << "Error semantico: Tipo de expresion desconocido en Nodo ID: " << expressionID << endl;
            return "";
        }
    }

    cerr << "Error semantico: Nodo de expresion no encontrado. ID: " << expressionID << endl;
    return "";
    }

    void validateAssignment(const string& variable, int expressionID, const string& csvFile) {
        if (symbolTable.find(variable) == symbolTable.end()) {
            cerr << "Error semantico: Variable no declarada: '" << variable << "'" << endl;
            return;
        }

        string varType = symbolTable[variable].type;
        string exprType = evaluateExpressionType(expressionID, csvFile);

        if (exprType.empty()) {
            cerr << "Error semantico: Expresion invalida en la asignacion a '" << variable << "'." << endl;
            return;
        }

        if (varType != exprType) {
            cerr << "Error semantico: Asignacion incompatible. Variable '" << variable
                << "' de tipo " << varType << " no puede ser asignada a una expresion de tipo " 
                << exprType << endl;
        }
    }

    void validateFunctionCall(const string& functionName, vector<int> argumentIDs, const string& csvFile) {
        if (symbolTable.find(functionName) == symbolTable.end()) {
            cerr << "Error semantico: Funcion no declarada: '" << functionName << "'" << endl;
            return;
        }

        Symbol func = symbolTable[functionName];
        if (!func.isFunction) {
            cerr << "Error semantico: '" << functionName << "' no es una funcion." << endl;
            return;
        }

        if (argumentIDs.size() != func.parameters.size()) {
            cerr << "Error semantico: Numero de parametros incorrecto en la llamada a '" 
                << functionName << "'. Se esperaban " << func.parameters.size() 
                << " parametros pero se recibieron " << argumentIDs.size() << "." << endl;
            return;
        }

        for (size_t i = 0; i < argumentIDs.size(); ++i) {
            string argType = evaluateExpressionType(argumentIDs[i], csvFile);
            if (argType != func.parameters[i]) {
                cerr << "Error semantico: Tipo de parametro incompatible en la llamada a '"
                    << functionName << "'. Se esperaba " << func.parameters[i]
                    << " pero se recibio " << argType << "." << endl;
            }
        }

        cout << "Llamada a funcion valida: " << functionName << endl;
    }

    void validateExpression(int expressionID, const string& csvFile) {
        string exprType = evaluateExpressionType(expressionID, csvFile);
        if (exprType.empty()) {
            cerr << "Error semantico: Tipo de expresion no valido para Nodo ID: " << expressionID << endl;
        } else {
            cout << "Expresion valida. Tipo: " << exprType << endl;
        }
    }

    vector<int> getArguments(int functionCallID) {
    vector<int> argumentIDs;
        for (const auto& node : tempNodes) {
            if (get<1>(node) == functionCallID && get<3>(node) == "ARGUMENT") {
                argumentIDs.push_back(get<0>(node));
            }
        }
        return argumentIDs;
    }
    vector<int> extractArguments(int functionCallID, const string& csvFile) {
        unordered_map<int, Node> nodes;
        ifstream inputFile(csvFile);
        string line;

        // Cargar el AST en memoria
        getline(inputFile, line); // Leer encabezado
        while (getline(inputFile, line)) {
            stringstream ss(line);
            Node node;
            string temp;

            getline(ss, temp, ',');
            node.id = stoi(temp);
            getline(ss, temp, ',');
            node.parentID = stoi(temp);
            getline(ss, node.value, ',');
            getline(ss, node.type, ',');

            nodes[node.id] = node;
        }
        inputFile.close();

        if (nodes.find(functionCallID) == nodes.end()) {
            cerr << "Error: Nodo de llamada a funcion no encontrado. ID: " << functionCallID << endl;
            return {};
        }

        Node& funcNode = nodes[functionCallID];
        vector<int> argumentIDs;

        for (int childID : funcNode.children) {
            if (nodes[childID].type == "ARGUMENT") {
                argumentIDs.push_back(childID);
            }
        }

        return argumentIDs;
    }

    void validateSemantics(const string& csvFile) {
        ifstream file(csvFile);
        string line;

        if (!file.is_open()) {
            cerr << "Error al abrir el archivo del AST." << endl;
            return;
        }

        getline(file, line); // Leer encabezado
        while (getline(file, line)) {
            stringstream ss(line);
            string idStr, parentIDStr, value, type;
            getline(ss, idStr, ',');
            getline(ss, parentIDStr, ',');
            getline(ss, value, ',');
            getline(ss, type, ',');

            int nodeID = stoi(idStr);

            if (type == "DECLARATION") {
                validateDeclaration(value, csvFile);
            } else if (type == "ASSIGNMENT") {
                validateAssignment(value, nodeID, csvFile); // Pasa el nodo de la expresion
            } else if (type == "FUNCTION_CALL") {
                vector<int> argumentIDs = extractArguments(nodeID, csvFile);
                validateFunctionCall(value, argumentIDs, csvFile); // Pasa los argumentos
            } else if (type == "EXPRESSION") {
                validateExpression(nodeID, csvFile); // Pasa el ID del nodo
            }
        }

        file.close();
    }

    string getNodeName(int nodeID) {
        for (const auto& node : tempNodes) {
            if (get<0>(node) == nodeID) {
                return get<2>(node); // El valor del nodo
            }
        }
        return "";
    }

    string getNodeType(int nodeID) {
        for (const auto& node : tempNodes) {
            if (get<0>(node) == nodeID) {
                return get<3>(node); // El tipo del nodo
            }
        }
        return "";
    }

    int getExpressionNodeID(int parentID) {
        for (const auto& node : tempNodes) {
            if (get<1>(node) == parentID && get<3>(node) == "EXPRESSION") {
                return get<0>(node); // ID del nodo de la expresion
            }
        }
        return -1;
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
            else{
            cout<<"ERROR in line: "<<get<2>(tokens[index])+1<<endl;
            parser_error = 1;
        }
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

                string varName = getNodeName(myID);
                string varType = getNodeType(myID);
                validateDeclaration(varType, varName);
                guard.commit();
                return true;
            }
        }
        
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
                            else{
                                // cout<<"Error: Expected '}'"<<endl;
                                // return true;
                            }
                        }
                    }
                    else{
                        cout<<"Error: Expected '{' in line: "<<get<2>(tokens[index])+1<<endl;
                        parser_error = 1;
                    synchronize();
                    if (STMT_LIST(myID)) {
                            if (checkToken("}")) {
                                int closeBraceID = currentID++;
                                addNode(closeBraceID, myID, "}", "terminal");
                                goNextToken();
                                guard.commit();
                                return true;
                            }
                            else{
                            //  synchronize();
                            }
                        }
                    }
                }
                else{
                    cout<<"Error: Expected ')' in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                    synchronize();
                    if (STMT_LIST(myID)) {
                            if (checkToken("}")) {
                                int closeBraceID = currentID++;
                                addNode(closeBraceID, myID, "}", "terminal");
                                goNextToken();
                                guard.commit();
                                return true;
                            }
                            else{
                            //  synchronize();
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
                            else{
                                cout<<"NINGUN TYPE in line: "<<get<2>(tokens[index])+1<<endl;
                                parser_error = 1;
                            }
                        }
                    }
                }
            }
        }
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
            else{
                cout<<"Error: falta cerrar ']' in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                return true;
            }
        } else {
            printDebug("TYPE_REST -> EOP");
        /* TYPE_REST -> EOP */
            if (EOP(myID)) {
                guard.commit();
                return true;
            }
        }
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
            addNode(commaID, myID, "comma", "terminal");
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
            else{
                cout<<"ERROR SIGNO DE ASIGNACION FALTANTE in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                 synchronize();
                 if (checkToken(";")) {
                        int semicolonID = currentID++;
                        addNode(semicolonID, myID, ";", "terminal");
                        goNextToken();
                        guard.commit();
                        return true;
                    }

            }
        }
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
                                        else{
                                    cout<<"LLAVES in line: "<<get<2>(tokens[index])+1<<endl;
                                    parser_error = 1;
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

                        string functionName = getNodeName(myID);
                    vector<int> argumentIDs = getArguments(myID);
                    validateFunctionCall(functionName, argumentIDs, "parseTree.csv");
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
                                else{
                                    cout<<"Error: Expected '}'. in line: "<<get<2>(tokens[index])+1<<endl;
                                    parser_error = 1;
                                    synchronize();  // Sincronizacion cuando no se encuentra '('.
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
                                        else{
                                            // synchronize();
                                        }
                                    }
                                }
                            }
                        }
                        else{
                           cout<<"Error: Expected '{'. in line: "<<get<2>(tokens[index])+1<<endl;
                           parser_error = 1;
                            synchronize();  // Sincronizacion cuando no se encuentra '('.
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
                                else{
                                    // synchronize();
                                }
                            }
                        }
                    }
                    else{
                       cout<<"Error: Expected ')'. in line: "<<get<2>(tokens[index])+1<<endl;
                       parser_error = 1;
                        synchronize();  // Sincronizacion cuando no se encuentra '('.
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
                            else{
                                // synchronize();
                            }
                        }
                    }
                }
            }
            else{
                cout<<"Error: Expected '('. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                synchronize();  // Sincronizacion cuando no se encuentra '('.
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
                    else{
                        // synchronize();
                    }
                }
            }
        }
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
                    else{
                       cout<<"LLAVES ELSE in line: "<<get<2>(tokens[index])+1<<endl;
                       parser_error = 1;
                    }
                }
            }
            else{
                cout<<"Error: Expected '{'. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                synchronize();  // Sincronizacion cuando no se encuentra '('.
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
                    else{
                        // synchronize();
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

                                    string functionName = getNodeName(myID);
                                    vector<int> argumentIDs = getArguments(myID);
                                    validateFunctionCall(functionName, argumentIDs, "parseTree.csv");
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
                                            else{
                                                cout<<"FALTA LLAVES FOR in line: "<<get<2>(tokens[index])+1<<endl;
                                                parser_error = 1;
                                            }
                                        }
                                    }
                                    else{
                                        cout<<"Error: Expected '{'. in line: "<<get<2>(tokens[index])+1<<endl;
                                        parser_error = 1;
                                    synchronize(); 
                                    if (STMT_LIST(myID)) {
                                            if (checkToken("}")) {
                                                int closeCurlyBracesID = currentID++;
                                                addNode(closeCurlyBracesID, myID, "}", "terminal");
                                                goNextToken();
                                                guard.commit();
                                                return true;
                                            }
                                            else{
                                                // synchronize();
                                            }
                                        }
                                    }
                                }
                                else{
                                    cout<<"Error: Expected ')'. in line: "<<get<2>(tokens[index])+1<<endl;
                                    parser_error = 1;
                                    synchronize(); 
                                    if (STMT_LIST(myID)) {
                                            if (checkToken("}")) {
                                                int closeCurlyBracesID = currentID++;
                                                addNode(closeCurlyBracesID, myID, "}", "terminal");
                                                goNextToken();
                                                guard.commit();
                                                return true;
                                            }
                                            else{
                                                // synchronize();
                                            }
                                        }
                                }
                            }
                        }
                        else{
                            cout<<"Error: Expected ';' en el for in line: "<<get<2>(tokens[index])+1<<endl;
                            parser_error = 1;
                            synchronize();
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
                                            else{
                                                // synchronize();
                                            }
                                        }
                                    }
                                    else{
                                        cout<<"Error: Expected '{'. in line: "<<get<2>(tokens[index])+1<<endl;
                                        parser_error = 1;
                                    synchronize(); 
                                    if (STMT_LIST(myID)) {
                                            if (checkToken("}")) {
                                                int closeCurlyBracesID = currentID++;
                                                addNode(closeCurlyBracesID, myID, "}", "terminal");
                                                goNextToken();
                                                guard.commit();
                                                return true;
                                            }
                                            else{
                                                // synchronize();
                                            }
                                        }
                                    }
                                }
                                else{
                                    cout<<"Error: Expected ')'. in line: "<<get<2>(tokens[index])+1<<endl;
                                    parser_error = 1;
                                    synchronize(); 
                                    if (STMT_LIST(myID)) {
                                            if (checkToken("}")) {
                                                int closeCurlyBracesID = currentID++;
                                                addNode(closeCurlyBracesID, myID, "}", "terminal");
                                                goNextToken();
                                                guard.commit();
                                                return true;
                                            }
                                            else{
                                                // synchronize();
                                            }
                                        }
                                }
                            }

                        }
                    }
                }
            }
            else{
                cout<<"Error: Expected '('. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                synchronize(); 
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
                                            else{
                                                // synchronize();
                                            }
                                        }
                                    }
                                    else{
                                        // synchronize();
                                    }
                                }
                                else{
                                    // synchronize();
                                }
                            }
                        }
                        else{
                            // synchronize();
                        }
                    }
                }
            }
        }
        else{
            // synchronize();
        }
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
                else{
                    cout<<"Error: expected ';' en return in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                    return true;
                }
            }
        }
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
                        string functionName = getNodeName(myID);
                        vector<int> argumentIDs = getArguments(myID);
                    validateFunctionCall(functionName, argumentIDs, "parseTree.csv");
                        if (checkToken(";")) {
                            int semicolonID = currentID++;
                            addNode(semicolonID, myID, ";", "terminal");
                            goNextToken();
                            guard.commit();
                            return true;
                        }
                        else{
                            cout<<"Error: Expected ';'. in line: "<<get<2>(tokens[index])+1<<endl;
                            parser_error = 1;
                            return true;
                        }
                    }
                    else{
                cout<<"Error: Expected ')'. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                synchronize(); 
                if (checkToken(";")) {
                            int semicolonID = currentID++;
                            addNode(semicolonID, myID, ";", "terminal");
                            goNextToken();
                            guard.commit();
                            return true;
                        }
                        else{
                            cout<<"Error: Expected ';'. in line: "<<get<2>(tokens[index])+1<<endl;
                            parser_error = 1;
                            return true;
                        }
            }
                }
            }
            else{
                cout<<"Error: Expected '('. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                synchronize(); 
                if (checkToken(";")) {
                            int semicolonID = currentID++;
                            addNode(semicolonID, myID, ";", "terminal");
                            goNextToken();
                            guard.commit();
                            return true;
                        }
            }
        }
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
            else{
                cout<<"Error: Expected ';'. in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
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
            addNode(commaID, myID, "comma", "terminal");
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

                        string varName = getNodeName(myID);
                        int exprID = getExpressionNodeID(myID);
                        validateAssignment(varName, exprID, "parseTree.csv");
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
                guard.commit();
                return true;
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
            else if(checkToken("!"))
            {
                cout<<"Error: expected != en lugar de ! in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                int notEqualID = currentID++;
                addNode(notEqualID, myID, "!=", "terminal");
                goNextToken();
                if (REL_EXPR(myID)) {
                    guard.commit();
                    return true;
                }
            }
            else if(checkToken("="))
            {
                cout<<"Error: expected != en lugar de = in line: "<<get<2>(tokens[index])+1<<endl;
                parser_error = 1;
                int notEqualID = currentID++;
                addNode(notEqualID, myID, "!=", "terminal");
                goNextToken();
                if (REL_EXPR(myID)) {
                    guard.commit();
                    return true;
                }
            }

        } 
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
            else{
                    cout<<"falta un termino del operador + in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
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
                else{
                    cout<<"falta un termino del operador - in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                    return true;
                }
            }
        }
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
            else{
                    cout<<"Falta un termino del operador * in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
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
                else{
                    cout<<"Falta un termino del operador / in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
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
                    else{
                    cout<<"Falta un termino del operador %. in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                }
                }
                
            }
        }
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
            else{
                    cout<<"Falta un termino del operador ! in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
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
                else{
                    cout<<"Falta un termino del operador - in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
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

                                        string functionName = getNodeName(myID);
                                        vector<int> argumentIDs = getArguments(myID);
                                        validateFunctionCall(functionName, argumentIDs, "parseTree.csv");
                                        if (FACTOR_REST(myID)) {
                                            guard.commit();
                                            return true;
                                        }
                                    }
                                    else{
                                        cout<<"Error: Expected ')' in line: "<<get<2>(tokens[index])+1<<endl;
                                        parser_error = 1;
                                        synchronize();
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
                else{
                    cout<<"falta cerrar ']' in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                    return true;
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

                        string functionName = getNodeName(myID);
                        vector<int> argumentIDs = getArguments(myID);
                        validateFunctionCall(functionName, argumentIDs, "parseTree.csv");
                        if (FACTOR_REST(myID)) {
                            guard.commit();
                            return true;
                        }
                    }
                    else{
                    cout<<"falta cerrar ')' in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                    return true;
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
            addNode(openDoubleQuouteID, myID, "quote", "terminal");
            goNextToken();
            if (checkTokenType("STRING")) {
                int stringID = currentID++;
                addNode(stringID, myID, get<1>(tokens[index]), "terminal");
                goNextToken();
                if (checkToken("\"")) {
                    int closeDoubleQuouteID = currentID++;
                    addNode(closeDoubleQuouteID, myID, "quote", "terminal");
                    goNextToken();
                    guard.commit();
                    return true;
                }
                else{
                    cout<<"Error: expected \" in line: "<<get<2>(tokens[index])+1<<endl;
                    parser_error = 1;
                }
            }
        }
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
        return false;
    }
};

int main() {
    Scanner scanner;

    vector<vector<char>> buffer = scanner.readBMMFile("input6.bmm");

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
        cout<<"------SCANNER TERMINADO CON "<<errores_encontrados.size() <<" ERRORES"<<endl;
        for(int i =0 ; i <errores_encontrados.size();i++)
        {
            cout << get<0>(errores_encontrados[i])  << get<1>(errores_encontrados[i]) << "' en fila " 
                     << get<2>(errores_encontrados[i]) << ", columna " << get<3>(errores_encontrados[i])-1<< endl;
        }
    }
    // for (int i = 0; i < tokens.size(); i++) {
    //    printf("Token: %-20s Value: %-20s Fila: %-20d Columna: %-20d\n", 
    //    get<0>(tokens[i]).c_str(), get<1>(tokens[i]).c_str(), get<2>(tokens[i]) + 1, get<3>(tokens[i]) + 1);
    // }

    
    Parser parser(tokens, false);

    return 0;
}