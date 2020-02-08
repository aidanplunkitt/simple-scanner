#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>

using namespace std;

struct Token {
	string lexeme;
	string category;
};

/* Categories:
 * 0: Other
 * 1: Whitespace
 * 2: Special Characters
 * 3: Alpha
 * 4: Digit
 */
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

/* Used for state machine table keywordStates below */
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

void tokenize(ostream&);

void handleNum();
void handleAlpha();
void handleSpecial();
void handleWhitespace();
void handleOther();

/* Jump table for the above functions */
void (*pf[])(void) = {handleOther, handleWhitespace, handleSpecial, handleAlpha, handleNum};

Token t;
string input;
int i, branchCount;
int counts[5];

int main(int argc, char *argv[]) {
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

	/* pass ostream reference to avoid having to pass tokens back
	 * and iterate through them again, minimizing branchCount */
	tokenize(outputf);

	/* hack to replace last comma in token list with newline */
	outputf.seekp(outputf.tellp() - (long) 2);
	outputf << "\n";

	cout << "Total number of branches: " << branchCount << "\n";
	cout << counts[0] << " " << counts[1] << " " << counts[2] 
	       	       	  << " " << counts[3] << " " << counts[4] << "\n";
}

void tokenize(ostream& f) {
	int category;

	i = 0;
	while (i < input.size()) {
		category = asciiToCategory[input.at(i)];
		pf[category]();	// jump table
		f << "(\"" << t.lexeme << "\", " << t.category << "), ";
		i++;

		counts[0]++;
		branchCount++;
	}
}

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

		counts[1]++;
		branchCount++;
	}

	counts[2]++;
	branchCount ++; // increment once for if/else
	if (lastState == -1) {
		t.category = "KW";
	} else {
		while(asciiToCategory[input.at(++i)] == 3 && i < input.length()) {
			counts[3]++;
			branchCount++;
		}
		t.category = "ID";
	}

	t.lexeme = input.substr(start, i - start);

	/* undo lookahead */
	--i;
};

void handleNum() {
	int start = i;
	while(asciiToCategory[input.at(++i)] == 4 && i < input.length()) {
		counts[4]++;
		branchCount++;
	}
	t.lexeme = input.substr(start, i - start);
	t.category = "NUM";

	/* undo lookahead */
	--i;
};

void handleWhitespace() {
	/* hack to avoid branch statement
	 * SPACE = 0x20 
	 * 0x20 >> 5 == 1
	 */
	string ws_lexemes[2] = {"\\n", " "};
	t.lexeme = ws_lexemes[input.at(i) >> 5];
	t.category = "WS";
};

void handleSpecial() {
	t.lexeme = input.at(i);
	t.category = "SYM";
};

void handleOther() {
	cerr << "ERROR: Unhandled char: " << (int) input.at(i) << "\n";
	exit(1);
};
