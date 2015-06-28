#include <iostream>
#include <fstream>
#include <string>
#include "Parser.h"

using namespace std;

Parser::Parser(string file_name) {
	inFile.open(file_name.c_str());

	if (!inFile.is_open()){
		cout << "Error: File won't open." << endl;
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
	//cout << "<Rat15su> ::= $$ <Opt Function Definitions> $$ <Opt Declaration List> <Statement List> $$" << endl;

	currentPair = getTokenLexemePair();

	if (currentPair.lexeme != "$$") {
		perror("First $$ marker expected; Rat15su()");
	}

	currentPair = getTokenLexemePair();

	if ((currentPair.lexeme != "function") && (currentPair.lexeme != "$$")) {
		perror("function lexeme or $$ marker expected; Rat15su()");
	}

	while (currentPair.lexeme == "function") {
			OptFuncDef();
	}

	if (currentPair.lexeme != "$$") {
		perror("Second $$ marker expected; Rat15su()");
	}

	currentPair = getTokenLexemePair();

	OptDeclList();
	StmtList();

	if (currentPair.lexeme != "$$") {
		perror("Last $$ marker expected; Rat15su()");
	}
}

void Parser::OptFuncDef() {
	FuncDef();
}

void Parser::FuncDef() {
	Func();
}
void Parser::Func() {
	if (currentPair.lexeme == "function") {
		currentPair = getTokenLexemePair();
		if (currentPair.token == "identifier") {
			currentPair = getTokenLexemePair();
			if (currentPair.lexeme == "(") {
				currentPair = getTokenLexemePair();
				OptParams();
				if (currentPair.lexeme == ")") {
					currentPair = getTokenLexemePair();
					if (currentPair.lexeme != "{") {
						OptDeclList();
					}
					Body();
				} else {
					perror(") lexeme expected; Func()");
				}
			} else {
				perror("( lexeme expected; Func()");
			}
		} else {
			perror("identifier token expected; Func()");
		}
	} else {
		perror("function lexeme expected; Func()");
	}
}

void Parser::OptParams() {
	Params();
}

void Parser::Params() {
	do {
		Param();
	} while (currentPair.lexeme == ",");
}

void Parser::Param() {
	Qualifier();
	if (currentPair.token == "identifier") {
		currentPair = getTokenLexemePair();
	} else {
		perror("identifier token expected; Param()");
	}
}

void Parser::OptDeclList() {
	DeclList();
}

void Parser::DeclList() {
	do {
		Decl();
		if (currentPair.lexeme == ";") {
			currentPair = getTokenLexemePair();
		}
		else {
			perror("; lexeme expected; DeclList()");
		}
	} while (currentPair.lexeme == "integer" || currentPair.lexeme == "boolean" || currentPair.lexeme == "real");
}

void Parser::Decl() {
	Qualifier();
	IDs();
}

void Parser::Qualifier() {
	if (currentPair.lexeme == "integer" || currentPair.lexeme == "boolean" || currentPair.lexeme == "real") {
		currentPair = getTokenLexemePair();
	} else {
		perror("integer, boolean, or real lexeme expected; Qualifier()");
	}
}

void Parser::IDs() {
	do {
		if (currentPair.lexeme == ",") {
			currentPair = getTokenLexemePair();
		}
		if (currentPair.token == "identifier") {
			currentPair = getTokenLexemePair();
		}
		else {
			perror("identifier token expected; IDs()");
		}
	} while (currentPair.lexeme == ",");
}

void Parser::Body() {
	if (currentPair.lexeme == "{") {
		currentPair = getTokenLexemePair();
	} else {
		perror("{ lexeme expected; Body()");
	}

	StmtList();

	if (currentPair.lexeme == "}") {
		currentPair = getTokenLexemePair();
	} else {
		perror("} lexeme expected; Body()");
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
		cout << currentPair.token << " " << currentPair.lexeme << endl;
		perror("{, identifier, if, return, write, read, or while lexeme expected; Stmt()");
	}
}

void Parser::Compound() {
	if (currentPair.lexeme == "{") {
		currentPair = getTokenLexemePair();
	} else {
		perror("{ lexeme expected; Compound()");
	}

	StmtList();

	if (currentPair.lexeme == "}") {
		currentPair = getTokenLexemePair();
	} else {
		perror("} lexeme expected; Compound()");
	}
}

void Parser::Assign() {
	if (currentPair.token == "identifier") {
		currentPair = getTokenLexemePair();
	} else {
		perror("identifier token expected; Assign()");
	}

	if (currentPair.lexeme == "=") {
		currentPair = getTokenLexemePair();
	} else {
		perror("= lexeme expected; Assign()");
	}

	Expr();

	if (currentPair.lexeme == ";") {
		currentPair = getTokenLexemePair();
	}
	else {
		perror("; lexeme expected; Assign()");
	}
}

void Parser::If() {
	if (currentPair.lexeme == "if") {
		currentPair = getTokenLexemePair();
	} else {
		perror("if lexeme expected; If()");
	}

	if (currentPair.lexeme == "(") {
		currentPair = getTokenLexemePair();
	} else {
		perror("( lexeme expected; If()");
	}

	Cond();

	if (currentPair.lexeme == ")") {
		currentPair = getTokenLexemePair();
	} else {
		perror(") lexeme expected; If()");
	}

	Stmt();

	if (currentPair.lexeme == "else") {
		Stmt();
		if (currentPair.lexeme == "fi") {
			currentPair = getTokenLexemePair();
		} else {
			perror("fi lexeme expected; If()");
		}
	} else if (currentPair.lexeme == "fi") {
		currentPair = getTokenLexemePair();
	} else {
		perror("else or fi lexeme expected; If()");
	}
}

void Parser::Return() {
	if (currentPair.lexeme == "return") {
		currentPair = getTokenLexemePair();
		Expr();
		if (currentPair.lexeme == ";") {
			currentPair = getTokenLexemePair();
		} else {
			perror("; lexeme expected; Return()");
		}
	} else {
		perror("return lexeme expected; Return()");
	}
}

void Parser::Write() {
	if (currentPair.lexeme == "write") {
		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			currentPair = getTokenLexemePair();
			Expr();
			if (currentPair.lexeme == ")") {
				currentPair = getTokenLexemePair();
				if (currentPair.lexeme == ";") {
					currentPair = getTokenLexemePair();
				} else {
					perror("; lexeme expected; Write()");
				}
			} else {
				perror(") lexeme expected; Write()");
			}
		} else {
			perror("( lexeme expected; Write()");
		}
	} else {
		perror("write lexeme expected; Write()");
	}
}

void Parser::Read() {
	if (currentPair.lexeme == "read") {
		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			currentPair = getTokenLexemePair();
			IDs();
			if (currentPair.lexeme == ")") {
				currentPair = getTokenLexemePair();
				if (currentPair.lexeme == ";") {
					currentPair = getTokenLexemePair();
				} else {
					perror("; lexeme expected; Read()");
				}
			} else {
				perror(") lexeme expected; Read()");
			}
		} else {
			perror("( lexeme expected; Read()");
		}
	} else {
		perror("read lexeme expected; Read()");
	}
}

void Parser::While() {
	if (currentPair.lexeme == "while") {
		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			currentPair = getTokenLexemePair();
			Cond();
			if (currentPair.lexeme == ")") {
				currentPair = getTokenLexemePair();
				Stmt();
			} else {
				perror(") lexeme expected; While()");
			}
		} else {
			perror("( lexeme expected; While()");
		}
	} else {
		perror("while lexeme expected; While()");
	}
}

void Parser::Cond() {
	Expr();
	Relop();
	Expr();
}

void Parser::Relop() {
	if (currentPair.lexeme == "==" || currentPair.lexeme == "!=" || currentPair.lexeme == ">" || currentPair.lexeme == "<") {
		currentPair = getTokenLexemePair();
	} else {
		perror("==, !=, >, or < lexeme expected; Relop()");
	}
}

void Parser::Expr() {
	Term();
	ExprPrime();
}

void Parser::ExprPrime() { // removed else statement bc this production can be empty
	if (currentPair.lexeme == "+" || currentPair.lexeme == "-") {
		currentPair = getTokenLexemePair();
		Term();
		ExprPrime();
	} /*else {
		perror("+ or - lexeme expected; ExprPrime()");
	}*/
}

void Parser::Term() {
	Factor();
	TermPrime();
}

void Parser::TermPrime() { // removed else statement bc this production can be empty
	if (currentPair.lexeme == "*" || currentPair.lexeme == "/") {
		currentPair = getTokenLexemePair();
		Factor();
		TermPrime();
	} /*else {
		perror("* or / lexeme expected; TermPrime()");
	}*/
}

void Parser::Factor() {
	if (currentPair.lexeme == "-") { // print production with dash in it
		currentPair = getTokenLexemePair();
	} // else, print production without dash
	Primary();
}

void Parser::Primary() {
	if (currentPair.token == "identifier") {
		currentPair = getTokenLexemePair();
		if (currentPair.lexeme == "(") {
			currentPair = getTokenLexemePair();
			IDs();
			if (currentPair.lexeme == ")") {
				currentPair = getTokenLexemePair();
			}
			else {
				perror(") lexeme expected; Primary()");
			}
		}
	} else if (currentPair.token == "integer" || currentPair.token == "real" || currentPair.lexeme == "true" || currentPair.lexeme == "false") {
		currentPair = getTokenLexemePair();
	} else if (currentPair.lexeme == "(") {
		currentPair = getTokenLexemePair();
		Expr();
		if (currentPair.lexeme == ")") {
			currentPair = getTokenLexemePair();
		} else {
			perror(") lexeme expected; Primary()");
		}
	} else {
		perror("identifier, real, or integer token or (, true, or false lexeme expected; Primary()");
	}
}