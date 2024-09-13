#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <cctype>
#include <algorithm>
#include <tuple>

using namespace std;

vector<string> tokens = {
	"IDENTIFIER",
	"KEYWORD",
	"INTEGER",
	"STRING",
	"CHAR",
	"OPERATOR",
	"COMMENT",
	"WHITESPACE",
	"SYMBOL",
	"UNKNOWN",
	"ERROR"
};

class Scanner {
private:
	vector<string> keywords;
	vector<string> symbols;
	vector<string> operators;

public:
	Scanner() {
		keywords = {"array", "boolean", "char", "else", "false", "for", "function", "if",
								"integer", "print", "return", "string", "true", "void", "while"};
		symbols = {",", ";", ":", "(", ")", "[", "]", "{", "}"};
		operators = {"++", "--", "+", "-", "*", "/", "%", "^", "&&", "||", "!", "=", "<", ">", "<=", ">=", "==", "!="};
	}

	~Scanner() {}

	vector<vector<char>> readBMMFile(const string filename) {
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

	char getchar(vector<vector<char>> buffer, int &row, int &col) {
		char c = buffer[row][col];
		col++;
		if (col >= buffer[row].size()) {
			row++;
			col = 0;
		}
		return c;
	}

	char peekchar(vector<vector<char>> buffer, int row, int col) {
		if (row >= buffer.size() || col >= buffer[row].size())
			return '\0';
		return buffer[row][col];
	}

	bool isSymbol(char c) {
		return find(symbols.begin(), symbols.end(), string(1, c)) != symbols.end();
	}

	bool isOperator(string str) {
		return find(operators.begin(), operators.end(), str) != operators.end();
	}

	pair<string, string> gettoken(vector<vector<char>> buffer, int row, int col) {
		if (row >= buffer.size())
			return {"EOF", ""};

		char c = getchar(buffer, row, col);

		// Ignorar espacios en blanco
		while (isspace(c)) {
			if (row >= buffer.size())
				return {"EOF", ""};
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
				return {"COMMENT", "//"};
			}
			else if (next == '*') {
				// Comentario de múltiples líneas
				getchar(buffer, row, col); // avanzar
				while (!(c == '*' && peekchar(buffer, row, col) == '/') && row < buffer.size()) {
					c = getchar(buffer, row, col);
				}
				getchar(buffer, row, col); // avanzar para cerrar el comentario
				return {"COMMENT", "/* */"};
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
				return {"KEYWORD", value};
			}
			else {
				return {"IDENTIFIER", value};
			}
		}

		// Verificar enteros
		if (isdigit(c)) {
			string value(1, c);
			while (isdigit(peekchar(buffer, row, col))) {
				value += getchar(buffer, row, col);
			}
			return {"INTEGER", value};
		}

		// Verificar operadores (incluyendo operadores de múltiples caracteres)
		string op(1, c);
		if (isOperator(op)) {
			char next = peekchar(buffer, row, col);
			string potentialOp = op + next;
			if (isOperator(potentialOp)) {
				getchar(buffer, row, col); // avanzar al siguiente carácter
				return {"OPERATOR", potentialOp};
			}
			return {"OPERATOR", op};
		}

		// Verificar símbolos
		if (isSymbol(c)) {
			return {"SYMBOL", string(1, c)};
		}

		// Si el carácter es inválido, registrar error
		return {"ERROR", string(1, c)};
	}

	vector<tuple<string, string, int, int>> tokenize(vector<vector<char>> &buffer) {
		int row = 0, col = 0;
		vector<tuple<string, string, int, int>> tokens;

		while (row < buffer.size()) {
			int startRow = row;
			int startCol = col;
			pair<string, string> token = gettoken(buffer, row, col);

			if (token.first == "EOF")
				break;

			tokens.push_back(make_tuple(token.first, token.second, startRow, startCol));

			// Manejo de errores: imprimir mensaje si es un token de error
			if (token.first == "ERROR") {
				cerr << "Error: Caracter no valido '" << token.second << "' en fila "
						 << startRow + 1 << ", columna " << startCol + 1 << endl;
			}
		}

		return tokens;
	}
};

int main() {
	Scanner scanner;

	vector<vector<char>> buffer = scanner.readBMMFile("input.bmm");

	for (int i = 0; i < buffer.size(); i++) {
		for (int j = 0; j < buffer[i].size(); j++) {
			cout << buffer[i][j];
		}
	}

	vector<tuple<string, string, int, int>> tokens = scanner.tokenize(buffer);

	for (const auto &token : tokens) {
		cout << "Token: " << get<0>(token) << ", Value: " << get<1>(token)
				 << "    Fila: " << get<2>(token) + 1 << ", Columna: " << get<3>(token) + 1 << endl;
	}

	return 0;
}
