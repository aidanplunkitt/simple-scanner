#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>

using namespace std;

struct Token {
	string lexeme;
	string category;
	/*
	char lexeme[20];
	char categoryStr[4];
	*/
};

enum Category {
	Other,
	Whitespace,
	Special,
	Alpha,
	Digit
};

string input;
int i = 0;
Token t;

int asciiToCategory[128] = {
0, 0, 0, 0, 0, 0, 0, 0,   0, 1, 1, 0, 0, 1, 0, 0, // \t \n \r
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
1, 0, 0, 0, 0, 0, 0, 0,   2, 2, 0, 2, 0, 0, 0, 0, // SPACE ( ) +
4, 4, 4, 4, 4, 4, 4, 4,   4, 4, 0, 2, 0, 2, 0, 0, // 0-9 ; =
0, 3, 3, 3, 3, 3, 3, 3,   3, 3, 3, 3, 3, 3, 3, 3, // A-O
3, 3, 3, 3, 3, 3, 3, 3,   3, 3, 3, 0, 0, 0, 0, 0, // P-Z
0, 3, 3, 3, 3, 3, 3, 3,   3, 3, 3, 3, 3, 3, 3, 3, // a-o
3, 3, 3, 3, 3, 3, 3, 3,   3, 3, 3, 0, 0, 0, 0, 0, // p-z
};

int keywordChars[128] = {
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 1, 2, 3, 0,   4, 5, 0, 0, 6, 0, 7, 0,   // d e f h i l n
0, 0, 8, 9, 10, 11, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // r s t u
};

// row = state
// col = input
// value = next state
int keywordStates[14][12] = {
//     d   e   f   h   i   l   n   r   s   t   u
  {0,  0,  1,  0,  0,  5,  0,  0,  6,  0, 11,  0}, // 0:  e i r t
  {0,  0,  0,  0,  0,  0,  2,  4,  0,  0,  0,  0}, // 1:  el, en
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0}, // 2:  els
  {0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 3:  else
  {0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 4:  end
  {0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0}, // 5:  if
  {0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 6:  re
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0}, // 7:  ret
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9}, // 8:  retu
  {0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0,  0}, // 9:  retur
  {0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0}, // 10: return
  {0,  0,  0,  0, 12,  0,  0,  0,  0,  0,  0,  0}, // 11: th
  {0,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 12: the
  {0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0}, // 13: then
};

vector<Token> tokenize();

void handleNum() {
	int start = i;
	while(asciiToCategory[input.at(++i)] == 4 && i < input.length());
	t.lexeme = input.substr(start, i - start);
	t.category = "NUM";

	/* undo lookahead */
	--i;
	cout << "Num: " << t.lexeme << "\n";
};

void handleAlpha() {
	int start = i;
	char c = input.at(i);
	int keywordLetter = keywordChars[c];
	int state = keywordStates[0][keywordLetter];
	int lastState;

	while(keywordLetter && state) {
		lastState = state; // keep track for check below
		c = input.at(++i);
		keywordLetter = keywordChars[c];
		state = keywordStates[state][keywordLetter];
	}

	if (lastState == -1) {
		t.category = "KW";
	} else {
		while(asciiToCategory[input.at(++i)] == 3 && i < input.length());
		t.category = "ID";
	}
	t.lexeme = input.substr(start, i - start);

	/* undo lookahead */
	--i;

	cout << "Alpha: " << t.lexeme << " category: " << t.category << "\n";
};

void handleSpecial() {
	t.lexeme = input.at(i);
	t.category = "SYM";
	cout << "Special: " << t.lexeme << "\n";
};

void handleWhitespace() {
	t.lexeme = input.at(i);
	t.category = "WS";
	cout << "Whitespace: " << t.lexeme << "\n";
};

void handleOther() {
	cerr << "ERROR: Unhandled char: " << (int) input.at(i) << "\n";
	exit(1);
};

void (*pf[])(void) = {handleOther, handleWhitespace, handleSpecial, handleAlpha, handleNum};

int main(int argc, char *argv[]) {
	vector<Token> tokens;

	/* Check that input and output file are given on commandline */
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
		exit(1);
	}

	/* Test input file */
	ifstream inputf(argv[1]);
	if (!inputf) {
		cerr << "error: could not open input file.\n";
		exit(1);
	}

	/* Test output file */
	ofstream outputf(argv[2]);
	if (!outputf) {
		cerr << "error: could not open output file.\n";
		exit(1);
	}
	
	/* Copy contents of inputfile to working buffer */
	getline(inputf, input, (char) inputf.eof());

	/* Output copy of the input */
	outputf << input;

	tokens = tokenize();
	
	for(int i=0; i < tokens.size(); i++) {
		cout << tokens[i].lexeme << "\n";
	}

	cout << input;

	cout << "Hello world\n";
}

vector<Token> tokenize() {
	vector<Token> tokens;

	int category;

	while (i < input.size()) {
		category = asciiToCategory[input.at(i)];
		pf[category]();		
		i++;
	}

	t.lexeme = "a lexeme";
	t.category = "a category";

	tokens.push_back(t);

	t.lexeme = "b";
	t.category = "b";

	tokens.push_back(t);

	return tokens;
}
