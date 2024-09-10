#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>

using namespace std;

class Scanner {
public:
    Scanner() {
        //TODO
    }

    ~Scanner() {
        //TODO
    }

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

    vector<pair<string, string>> Tokenize(vector<vector<char>> buffer) {

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

    vector<pair<string, string>> tokens;

    return 0;
}
