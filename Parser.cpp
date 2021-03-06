#include <iostream>
#include <fstream>
#include <string>
#include "Parser.h"

using namespace std;

Parser::Parser(string file_name) {
	inFile.open(file_name.c_str());

	if (!inFile.is_open()){
		outFile << "Error: File won't open." << endl;
	}

	outFile.open("out.txt");
}

LexTokPair Parser::getTokenLexemePair() {
	char next_char;
	bool not_invalid = false;

	inFile.get(next_char);

	// skip whitespace
	while (isspace(next_char)) {
		inFile.get(next_char);
	}

	// if first char of token is a 2-char operator
	if (((next_char == '=' || next_char == '!' || next_char == '<'
		|| next_char == '>') && inFile.peek() == '=')) {
		LexTokPair pair;
		pair.lexeme += next_char;
		inFile.get(next_char);
		pair.lexeme += next_char;
		pair.token = "operator";
		return pair;
	}

	// if first char of token is a single-char operator
	else if (isOperator(next_char)) {
		LexTokPair pair;
		pair.lexeme += next_char;
		pair.token = "operator";
		return pair;
	}

	// if first char of token is a 2-char separator
	else if ((next_char == '$') && (inFile.peek() == '$')) {
		LexTokPair pair;
		pair.lexeme += next_char;
		inFile.get(next_char);
		pair.lexeme += next_char;
		pair.token = "separator";
		return pair;
	}

	// if first char of token is a separator
	else if (isSeparator(next_char)) {
		LexTokPair pair;
		pair.lexeme += next_char;
		pair.token = "separator";
		return pair;
	}

	// if first char of token is an invalid/unknown char
	else if ((next_char == '.')
		|| (!isSeparator(next_char) && !isOperator(next_char) && !isalnum(next_char))) {
		LexTokPair pair;
		pair.lexeme += next_char;
		if (next_char == '.' && isdigit(inFile.peek())) {
			while (isdigit(inFile.peek())) {
				inFile.get(next_char);
				pair.lexeme += next_char;
			}
		}
		pair.token = "invalid";
		return pair;
	}

	// if first char of token is a digit, go to digit/real dfsm
	else if (isdigit(next_char)) {
		int state = 0, // starting state: 0
			col = 0; // initializing column to 0
		bool token_found = false,
			leave_machine = false;
		LexTokPair pair;

		// starting state: 0
		// acceptance states: 1, 4
		int DigitOrRealTable[5][2] = { { 1, 2 }, // row 0
		{ 1, 3 }, // row 1
		{ 2, 2 }, // row 2
		{ 4, 2 }, // row 3
		{ 4, 2 }, }; // row 4

		// set machine variable to digits/reals
		machine = "dr";

		// while token is not found and it's not yet time to leave this machine
		while (!token_found) {
			switch (state) {
				// states 1, 4 are acceptance states
				// original states were numbered 1-5, not 0-4
			case 1:
				// if char is . then need to go to state 4
				if (next_char == '.') {
					state = 4;
					break;
				}
				// if next char in stream is not EOF and next_char is valid
				if ((inFile.peek() != EOF) && not_invalid) {
					// tack on next_char to lexeme variable
					pair.lexeme += next_char;
					// if next char is a digit, token not found yet
					while (isdigit(inFile.peek())) {
						inFile.get(next_char);
						// tack on next_char to lexeme variable
						pair.lexeme += next_char;
					}
				}
				// if next char in stream is a period, then go to state 4 for real numbers
				if (inFile.peek() == '.') {
					inFile.get(next_char);
					state = 4;
					break;
				}
				pair.token = "integer";
				token_found = true;
				return pair;
				//break;
			case 2: // only invalid lexemes transition to state 2
				pair.lexeme += next_char;
				while (isdigit(inFile.peek()) || (inFile.peek() == '.')) {
					inFile.get(next_char);
					pair.lexeme += next_char;
				}
				pair.token = "invalid";
				return pair;
				//break;
			case 4:
				// if next char is a digit, token not found yet
				if (isdigit(inFile.peek())) {
					// tack on next_char to lexeme variable
					pair.lexeme += next_char;
					// read in next char from file
					inFile.get(next_char);
					// tack on next_char to lexeme variable
					pair.lexeme += next_char;
					// read in next char from file, as long as it's a digit
					while (isdigit(inFile.peek())) {
						inFile.get(next_char);
						// tack on next_char to lexeme variable
						pair.lexeme += next_char;
					}
				}
				// else lexeme is invalid/unknown
				else {
					state = 2;
					break;
				}
				if (inFile.peek() == '.') {
					state = 2;
					inFile.get(next_char);
					break;
				}
				pair.token = "real";
				token_found = true;
				return pair;
				//break;
			default:
				// tack on next_char to lexeme variable
				pair.lexeme += next_char;

				// find column for next_char
				col = findLexemeColumn(next_char);
				// get next state
				state = DigitOrRealTable[state][col];

				// if next char is a digit or period, read in next char from file
				if (!isInvalid(inFile.peek()) && !isspace(inFile.peek()) && !isalpha(inFile.peek())
					&& !isOperator(inFile.peek()) && !isSeparator(inFile.peek())) {
					inFile.get(next_char);
					not_invalid = true;
				}
				break;
			}
		}
	}

	// if first char of token is a letter, go to identifer dfsm
	else if (isalpha(next_char)) {
		int state = 0, // starting state: 0
			col = 0; // initializing column to 0
		bool token_found = false,
			leave_machine = false;
		LexTokPair pair;

		// starting state: 0
		// acceptance states: 1, 3
		int IdentifierTable[5][2] = { { 1, 2 }, // row 0
		{ 3, 4 }, // row 1
		{ 2, 2 }, // row 2
		{ 3, 4 }, // row 3
		{ 3, 4 }, }; // row 4

		machine = "id";

		// while token is not found and it's not yet time to leave this machine
		while (!token_found) {
			switch (state) {
				// states 1, 3 are acceptance states
				// original states were numbered 1-5, not 0-4
			case 1:
				// if not a single char identifier
				if (isalpha(inFile.peek())) {
					// tack on next_char to lexeme variable
					pair.lexeme += next_char;
					// get next char
					inFile.get(next_char);
					state = 3;
					break;
				}
				else if (isdigit(inFile.peek())) {
					state = 3;
					break;
				}
				if (inFile.peek() != EOF && not_invalid) {
					// tack on next_char to lexeme variable
					pair.lexeme += next_char;
				}
				// if lexeme is a keyword
				if (isKeyword(pair.lexeme)) {
					pair.token = "keyword";
				}
				else {
					pair.token = "identifier";
				}
				token_found = true;
				return pair;
				//break;
			case 2: // only invalid lexemes transition to state 2
				pair.lexeme += next_char;
				pair.token = "invalid";
				return pair;
				//break;
			case 3:
				// tack on next_char to lexeme variable
				pair.lexeme += next_char;
				if (inFile.peek() != EOF && not_invalid) {
					// if next char is a letter or digit, token not found yet
					while (isalnum(inFile.peek())) {
						// get next char
						inFile.get(next_char);
						/*if (isdigit(next_char) && !isalnum(inFile.peek())) {
						pair.lexeme += next_char;
						pair.token = "invalid";
						return pair;
						}*/
						// tack on next_char to lexeme variable
						pair.lexeme += next_char;
					}
				}
				if (isdigit(next_char)) {
					pair.token = "invalid";
					return pair;
				}
				// if lexeme is a keyword
				if (isKeyword(pair.lexeme)) {
					pair.token = "keyword";
				}
				else {
					pair.token = "identifier";
				}
				token_found = true;
				return pair;
				//break;
			default:
				// tack on next_char to lexeme variable
				pair.lexeme += next_char;

				// find column for next_char
				col = findLexemeColumn(next_char);
				// get next state
				state = IdentifierTable[state][col];

				// if next char is a digit or letter, read in next char from file
				if (!isInvalid(inFile.peek()) && !isspace(inFile.peek()) && (inFile.peek() != '.')
					&& !isSeparator(inFile.peek()) && !isOperator(inFile.peek())) {
					inFile.get(next_char);
					not_invalid = true;
				}
				break;
			}
		}
	}
}

int Parser::findLexemeColumn(char some_char) {
	if (machine == "dr") {
		if (isdigit(some_char)) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		if (isalpha(some_char)) {
			return 0;
		}
		else {
			return 1;
		}
	}
}

bool Parser::isInvalid(char some_char) {
	return (!
		(isSeparator(some_char)
		|| isOperator(some_char)
		|| isalnum(some_char)
		|| (some_char == '.')
		));
}

bool Parser::isSeparator(char c) {
	return (
		c == '('
		|| c == ')'
		|| c == ';'
		|| c == ','
		|| c == '{'
		|| c == '}'
		);
}

bool Parser::isOperator(char c) {
	return (
		c == '='
		|| c == '!'
		|| c == '<'
		|| c == '>'
		|| c == '+'
		|| c == '-'
		|| c == '*'
		|| c == '/'
		);
}

bool Parser::isKeyword(string input) {
	return (
		input == "function"
		|| input == "return"
		|| input == "integer"
		|| input == "real"
		|| input == "boolean"
		|| input == "if"
		|| input == "fi"
		|| input == "else"
		|| input == "read"
		|| input == "write"
		|| input == "while"
		|| input == "for"
		);
}

bool Parser::atEndOfFile() {
	if (!inFile) {
		return true;
	}
	else {
		return false;
	}
}

void Parser::closeFiles() {
	inFile.close();
	outFile.close();
}

void Parser::Rat15su() {
	currentPair = getTokenLexemePair();

	outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
	outFile << "<Rat15su> ::= $$ <Opt Function Definitions> $$ <Opt Declaration List> <Statement List> $$" << endl << endl;

	if (currentPair.lexeme != "$$") {
		cerr << "First $$ marker expected; Rat15su()" << endl;
		exit(0);
	}

	currentPair = getTokenLexemePair();

	if (currentPair.lexeme == "$$") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Rat15su> ::= $$ <Opt Function Definitions> $$ <Opt Declaration List> <Statement List> $$" << endl << endl;
	}

	if ((currentPair.lexeme != "function") && (currentPair.lexeme != "$$")) {
		cerr << "function lexeme or $$ marker expected; Rat15su()" << endl;
		exit(0);
	}

	if (currentPair.lexeme == "function") {
		OptFuncDef();
	}

	if (currentPair.lexeme == "$$") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Rat15su> ::= $$ <Opt Function Definitions> $$ <Opt Declaration List> <Statement List> $$" << endl << endl;
	} else {
		cerr << "Second $$ marker expected; Rat15su()" << endl;
		exit(0);
	}

	currentPair = getTokenLexemePair();

	if (currentPair.lexeme == "real" || currentPair.lexeme == "boolean" || currentPair.lexeme == "integer") {
		OptDeclList();
	}

	StmtList();
	
	if (currentPair.lexeme == "$$") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Rat15su> ::= $$ <Opt Function Definitions> $$ <Opt Declaration List> <Statement List> $$" << endl << endl;
	} else {
		cerr << "Last $$ marker expected; Rat15su()" << endl;
		exit(0);
	}
}

void Parser::OptFuncDef() {
	while (currentPair.lexeme == "function") {
		FuncDef();
	}
}

void Parser::FuncDef() {
	Func();
}
void Parser::Func() {
	if (currentPair.lexeme == "function") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Function> ::= function <Identifier> (<Opt Parameters>) <Opt Declaration List> <Body>" << endl << endl;

		currentPair = getTokenLexemePair();
		if (currentPair.token == "identifier") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Function> ::= function <Identifier> (<Opt Parameters>) <Opt Declaration List> <Body>" << endl << endl;

			currentPair = getTokenLexemePair();
			if (currentPair.lexeme == "(") {
				outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
				outFile << "<Function> ::= function <Identifier> (<Opt Parameters>) <Opt Declaration List> <Body>" << endl << endl;

				currentPair = getTokenLexemePair();
				OptParams();
				if (currentPair.lexeme == ")") {
					outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
					outFile << "<Function> ::= function <Identifier> (<Opt Parameters>) <Opt Declaration List> <Body>" << endl << endl;

					currentPair = getTokenLexemePair();
					if (currentPair.lexeme != "{") {
						OptDeclList();
					}
					Body();
				} else {
					cerr << ") lexeme expected; Func()" << endl;
					exit(0);
				}
			} else {
				cerr << "( lexeme expected; Func()" << endl;
				exit(0);
			}
		} else {
			cerr << "identifier token expected; Func()" << endl;
			exit(0);
		}
	} else {
		cerr << "function lexeme expected; Func()" << endl;
		exit(0);
	}
}

void Parser::OptParams() {
	Params();
}

void Parser::Params() {
	do {
		Param();
	} while (currentPair.lexeme == ","); //
}

void Parser::Param() {
	if (currentPair.lexeme == ",") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Parameter> ::= <Identifier> <Qualifier>" << endl << endl;

		currentPair = getTokenLexemePair();
	}
	if (currentPair.token == "identifier") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Parameter> ::= <Identifier> <Qualifier>" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "identifier token expected; Param()" << endl;
		exit(0);
	}
	Qualifier();
}

void Parser::OptDeclList() {
	DeclList();
}

void Parser::DeclList() {
	do {
		Decl();
		if (currentPair.lexeme == ";") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Declaration List> ::= <Declaration>; | <Declaration>; <Declaration List>" << endl << endl;

			currentPair = getTokenLexemePair();
		}
		else {
			cerr << "; lexeme expected; DeclList()" << endl;
			exit(0);
		}
	} while (currentPair.lexeme == "integer" || currentPair.lexeme == "boolean" || currentPair.lexeme == "real");
}

void Parser::Decl() {
	Qualifier();
	IDs();
}

void Parser::Qualifier() {
	if (currentPair.lexeme == "integer") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Qualifier> ::= integer" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "boolean") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Qualifier> ::= boolean" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "real") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Qualifier> ::= real" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "integer, boolean, or real lexeme expected; Qualifier()" << endl;
		exit(0);
	}
}

void Parser::IDs() {
	do {
		if (currentPair.lexeme == ",") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<IDs> ::= <Identifier>, <IDs>" << endl << endl;

			currentPair = getTokenLexemePair();
		}
		if (currentPair.token == "identifier") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<IDs> ::= <Identifier>" << endl << endl;

			currentPair = getTokenLexemePair();
		}
		else {
			cerr << "identifier token expected; IDs()" << endl;
			exit(0);
		}
	} while (currentPair.lexeme == "," || currentPair.token == "identifier");
}

void Parser::Body() {
	if (currentPair.lexeme == "{") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Body> ::= { <Statement List> }" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "{ lexeme expected; Body()" << endl;
		exit(0);
	}

	StmtList();
	if (currentPair.lexeme == "}") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Body> ::= { <Statement List> }" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "} lexeme expected; Body()" << endl;
		exit(0);
	}
}

void Parser::StmtList() {
	while (currentPair.lexeme != "}" && currentPair.lexeme != "$$") {
		Stmt();
	}
}

void Parser::Stmt() {
	if (currentPair.lexeme == "{") {
		Compound();
	} else if (currentPair.token == "identifier") {
		Assign();
	} else if (currentPair.lexeme == "if") {
		If();
	} else if (currentPair.lexeme == "return") {
		Return();
	} else if (currentPair.lexeme == "write") {
		Write();
	} else if (currentPair.lexeme == "read") {
		Read();
	} else if (currentPair.lexeme == "while") {
		While();
	} else {
		cerr << "{, identifier, if, return, write, read, or while lexeme expected; Stmt()" << endl;
		exit(0);
	}
}

void Parser::Compound() {
	if (currentPair.lexeme == "{") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Compound> ::= { <Statement List> }" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "{ lexeme expected; Compound()" << endl;
		exit(0);
	}

	StmtList();

	if (currentPair.lexeme == "}") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Compound> ::= { <Statement List> }" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "} lexeme expected; Compound()" << endl;
		exit(0);
	}
}

void Parser::Assign() {
	if (currentPair.token == "identifier") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Assign> ::= <Identifier> = <Expression>;" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "identifier token expected; Assign()" << endl;
		exit(0);
	}

	if (currentPair.lexeme == "=") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Assign> ::= <Identifier> = <Expression>;" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "= lexeme expected; Assign()" << endl;
		exit(0);
	}

	Expr();

	if (currentPair.lexeme == ";") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Assign> ::= <Identifier> = <Expression>;" << endl << endl;

		currentPair = getTokenLexemePair();
	}
	else {
		cerr << "; lexeme expected; Assign()" << endl;
		exit(0);
	}
}

void Parser::If() {
	if (currentPair.lexeme == "if") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<If> ::= if (<Condition>) <Statement> fi | if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "if lexeme expected; If()" << endl;
		exit(0);
	}

	if (currentPair.lexeme == "(") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<If> ::= if (<Condition>) <Statement> fi | if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "( lexeme expected; If()" << endl;
		exit(0);
	}

	Cond();

	if (currentPair.lexeme == ")") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<If> ::= if (<Condition>) <Statement> fi | if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << ") lexeme expected; If()" << endl;
		exit(0);
	}

	Stmt();

	if (currentPair.lexeme == "else") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<If> ::= if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

		currentPair = getTokenLexemePair();

		Stmt();

		if (currentPair.lexeme == "fi") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<If> ::= if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

			currentPair = getTokenLexemePair();
		} else {
			cerr << "fi lexeme expected; If()" << endl;
			exit(0);
		}
	} else if (currentPair.lexeme == "fi") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<If> ::= if (<Condition>) <Statement> else <Statement> fi" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "else or fi lexeme expected; If()" << endl;
		exit(0);
	}
}

void Parser::Return() {
	if (currentPair.lexeme == "return") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Return> ::= return <Expression>;" << endl << endl;

		currentPair = getTokenLexemePair();
		Expr();
		if (currentPair.lexeme == ";") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Return> ::= return <Expression>;" << endl << endl;

			currentPair = getTokenLexemePair();
		} else {
			cerr << "; lexeme expected; Return()" << endl;
			exit(0);
		}
	} else {
		cerr << "return lexeme expected; Return()" << endl;
		exit(0);
	}
}

void Parser::Write() {
	if (currentPair.lexeme == "write") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Write> ::= write(<Expression>);" << endl << endl;

		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Write> ::= write(<Expression>);" << endl << endl;

			currentPair = getTokenLexemePair();
			Expr();
			if (currentPair.lexeme == ")") {
				outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
				outFile << "<Write> ::= write(<Expression>);" << endl << endl;

				currentPair = getTokenLexemePair();
				if (currentPair.lexeme == ";") {
					outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
					outFile << "<Write> ::= write(<Expression>);" << endl << endl;

					currentPair = getTokenLexemePair();
				} else {
					cerr << "; lexeme expected; Write()" << endl;
					exit(0);
				}
			} else {
				cerr << ") lexeme expected; Write()" << endl;
				exit(0);
			}
		} else {
			cerr << "( lexeme expected; Write()" << endl;
			exit(0);
		}
	} else {
		cerr << "write lexeme expected; Write()" << endl;
		exit(0);
	}
}

void Parser::Read() {
	if (currentPair.lexeme == "read") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Read> ::= read(<IDs>);" << endl << endl;

		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Read> ::= read(<IDs>);" << endl << endl;

			currentPair = getTokenLexemePair();
			IDs();
			if (currentPair.lexeme == ")") {
				outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
				outFile << "<Read> ::= read(<IDs>);" << endl << endl;

				currentPair = getTokenLexemePair();
				if (currentPair.lexeme == ";") {
					outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
					outFile << "<Read> ::= read(<IDs>);" << endl << endl;

					currentPair = getTokenLexemePair();
				} else {
					cerr << "; lexeme expected; Read()" << endl;
					exit(0);
				}
			} else {
				cerr << ") lexeme expected; Read()" << endl;
				exit(0);
			}
		} else {
			cerr << "( lexeme expected; Read()" << endl;
			exit(0);
		}
	} else {
		cerr << "read lexeme expected; Read()" << endl;
		exit(0);
	}
}

void Parser::While() {
	if (currentPair.lexeme == "while") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<While> ::= while(<Condition>) <Statement>" << endl << endl;

		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<While> ::= while(<Condition>) <Statement>" << endl << endl;

			currentPair = getTokenLexemePair();
			Cond();
			if (currentPair.lexeme == ")") {
				outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
				outFile << "<While> ::= while(<Condition>) <Statement>" << endl << endl;

				currentPair = getTokenLexemePair();
				Stmt();
			} else {
				cerr << ") lexeme expected; While()" << endl;
				exit(0);
			}
		} else {
			cerr << "( lexeme expected; While()" << endl;
			exit(0);
		}
	} else {
		cerr << "while lexeme expected; While()" << endl;
		exit(0);
	}
}

void Parser::Cond() {
	Expr();
	Relop();
	Expr();
}

void Parser::Relop() {
	if (currentPair.lexeme == "==") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Relop> ::= ==" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "!=") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Relop> ::= !=" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == ">") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Relop> ::= >" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "<") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Relop> ::= <" << endl << endl;

		currentPair = getTokenLexemePair();
	} else {
		cerr << "==, !=, >, or < lexeme expected; Relop()" << endl;
		exit(0);
	}
}

void Parser::Expr() {
	Term();
	ExprPrime();
}

void Parser::ExprPrime() { // removed else statement bc this production can be empty
	if (currentPair.lexeme == "+") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Expression Prime> ::= + <Term> <Expression Prime> | <Empty>" << endl << endl;

		currentPair = getTokenLexemePair();
		Term();
		ExprPrime();
	} else if (currentPair.lexeme == "-") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Expression Prime> ::= - <Term> <Expression Prime> | <Empty>" << endl << endl;

		currentPair = getTokenLexemePair();
		Term();
		ExprPrime();
	}
	/*else {
		cerr << "+ or - lexeme expected; ExprPrime()" << endl;
		exit(0);
	}*/
}

void Parser::Term() {
	Factor();
	TermPrime();
}

void Parser::TermPrime() { // removed else statement bc this production can be empty
	if (currentPair.lexeme == "*") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Term Prime> ::= * <Factor> <Term Prime> | <Empty>" << endl << endl;

		currentPair = getTokenLexemePair();
		Factor();
		TermPrime();
	} else if (currentPair.lexeme == "/") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Term Prime> ::= / <Factor> <Term Prime> | <Empty>" << endl << endl;

		currentPair = getTokenLexemePair();
		Factor();
		TermPrime();
	}
	/*else {
		cerr << "* or / lexeme expected; TermPrime()" << endl;
		exit(0);
	}*/
}

void Parser::Factor() {
	if (currentPair.lexeme == "-") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Factor> ::= - <Primary> | <Primary>" << endl << endl;

		currentPair = getTokenLexemePair();
	}
	Primary();
}

void Parser::Primary() {
	if (currentPair.token == "identifier") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= <Identifier>" << endl << endl;

		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Primary> ::= <Identifier> (<IDs>)" << endl << endl;

			currentPair = getTokenLexemePair();
			IDs();
			if (currentPair.lexeme == ")") {
				outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
				outFile << "<Primary> ::= <Identifier> (<IDs>)" << endl << endl;

				currentPair = getTokenLexemePair();
			} else {
				cerr << ") lexeme expected; Primary()" << endl;
				exit(0);
			}
		}
	} else if (currentPair.token == "integer") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= <Integer>" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.token == "real") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= <Real>" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "true") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= true" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "false") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= false" << endl << endl;

		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "(") {
		outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
		outFile << "<Primary> ::= (<Expression>)" << endl << endl;

		currentPair = getTokenLexemePair();
		Expr();
		if (currentPair.lexeme == ")") {
			outFile << "Token: " << currentPair.token << "\tLexeme: " << currentPair.lexeme << endl;
			outFile << "<Primary> ::= (<Expression>)" << endl << endl;

			currentPair = getTokenLexemePair();
		} else {
			cerr << ") lexeme expected; Primary()" << endl;
			exit(0);
		}
	} else {
		cerr << "identifier, real, or integer token or (, true, or false lexeme expected; Primary()" << endl;
		exit(0);
	}
}