#pragma once

#include <string>
#include <fstream>

using namespace std;

struct LexTokPair {
	string token,
		lexeme;
};
 
class Parser {
public:
	Parser(string);
	// constructor takes the name of an input file and attempts to open the file

	LexTokPair getTokenLexemePair();
	// if file input is valid, returns lexeme-token pair

	int findLexemeColumn(char);
	// depending on what the character is, this function returns the corresponding column of the 2D array

	bool isInvalid(char);
	// if character is not in the language, returns true

	bool isSeparator(char);
	// if character is a separator, returns true
	// this function also checks if the character is a part of the separator "<-"

	bool isOperator(char);
	// if character is an operator, returns true

	bool isKeyword(string);
	// if the identifier string is a keyword, returns true

	bool atEndOfFile();
	// if at end of file, returns true

	void closeFiles();
	// close i/o files at object level (so that main driver program has access to this feature)

	void Rat15su();
	void OptFuncDef();
	void FuncDef();
	void Func();
	void OptParams();
	void Params();
	void Param();
	void OptDeclList();
	void DeclList();
	void Decl();
	void Qualifier();
	void IDs();
	void Body();
	void StmtList();
	void Stmt();
	void Compound();
	void Assign();
	void If();
	void Return();
	void Write();
	void Read();
	void While();
	void Cond();
	void Relop();
	void Expr();
	void ExprPrime();
	void Term();
	void TermPrime();
	void Factor();
	void Primary();

private:
	ifstream inFile;
	ofstream outFile;
	string machine = ""; // digits/reals (dr) or identifiers (id) machine?
	LexTokPair currentPair;
};