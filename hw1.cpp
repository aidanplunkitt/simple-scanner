#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>

using namespace std;

//char inputBuff[100];
char tokenBuff[40];

struct Token {
	string lexeme;
	string categoryStr;
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
0, 0, 0, 0, 1, 1, 1, 0,   1, 1, 0, 0, 1, 0, 1, 0, // d e f h i l n
0, 0, 1, 1, 1, 1, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // r s t y
};

// row = state
// col = input
// value = next state
int keywordStates[16][12] = {
//     d   e   f   h   i   l   n   r   s   t   u
  {0,  0,  1,  0,  0,  6,  0,  0,  7,  0, 12,  0}, // 0:  e i r t
  {0,  0,  0,  0,  0,  0,  2,  4,  0,  0,  0,  0}, // 1:  el, en
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0}, // 2:  els
  {0,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 3:  else
  {0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 4:  end
  {0,  0,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0}, // 6:  if
  {0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 7:  re
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0}, // 8:  ret
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10}, // 9:  retu
  {0,  0,  0,  0,  0,  0,  0,  0, 11,  0,  0,  0}, // 10: retur
  {0,  0,  0,  0,  0,  0,  0, 15,  0,  0,  0,  0}, // 11: return
  {0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0}, // 12: th
  {0,  0, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // 13: the
  {0,  0,  0,  0,  0,  0,  0, 15,  0,  0,  0,  0}, // 14: then
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // 15: ending state
};

vector<Token> tokenize(string str);

void handleNum(string &s, int *i) {
	cout << "Num: " << s.at(*i) << "\n";
};

void handleAlpha(string &s, int *i) {
	cout << "Alpha\n";
};

void handleSpecial(string &s, int *i) {
	cout << "Special\n";
};

void handleWhitespace(string &s, int *i) {
	cout << "Whitespace\n";
};

void handleOther(string &s, int *i) {
	cerr << "ERROR: Unhandled char: " << (int) s.at(*i) << "\n";
	exit(1);
};

void (*pf[])(string&, int*) = {handleOther, handleWhitespace, handleSpecial, handleAlpha, handleNum};

int main(int argc, char *argv[]) {
	string input;
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

	tokens = tokenize(input);
	
	for(int i=0; i < tokens.size(); i++) {
		cout << tokens[i].lexeme << "\n";
	}
	/*
	while (tokenize()) {
		sprintf(tokenBuff, "(\"%s\",%s),", token.lexeme, token.categoryStr);	
		cout << tokenBuff;
		output << tokenBuff;
	}
	*/
	/*
	while (*s != '\0') {
		printf("%c -> %d\n", *s, *s);
		s++;
	}
	*/

	cout << input;

	cout << "Hello world\n";
}

vector<Token> tokenize(string str) {
	vector<Token> tokens;

	Token t;
	int category;

	int i = 0;
	while (i < str.size()) {
		category = asciiToCategory[str.at(i)];
		pf[category](&str, &i);		
		i++;
	}

	t.lexeme = "a lexeme";
	t.categoryStr = "a category";

	tokens.push_back(t);

	t.lexeme = "b";
	t.categoryStr = "b";

	tokens.push_back(t);

	return tokens;
}
