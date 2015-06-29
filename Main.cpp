#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include "Parser.h"

using namespace std;

int main() {
	//LexTokPair pair;
	//string filename = "in.txt";
	string filename;
	//ofstream outFile;

	// create file to print to
	//outFile.open("out.txt");

	cout << "Enter input file name: ";
	getline(cin, filename);

	Parser p(filename);

	p.Rat15su();

	//outFile << "Token" << "\t\t\t" << "Lexeme" << endl;

	/*while (!l.atEndOfFile()) {
		pair = l.getTokenLexemePair();
		outFile << pair.token << setw(18) << pair.lexeme << endl;
	}*/

	// close i/o files
	p.closeFiles();
}