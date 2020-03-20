/* lexcontext.h
 *
 * William Miller
 * Mar 19, 2020
 *
 * Class for storing parameters relating to the context of a 
 * lexeme.
 *
 */

#ifndef LEXCONTEXT_H
#define LEXCONTEXT_H

#include <iostream>
#include <string>
#include <vector>

enum lex_type {
	Operator,
	Character,
	String,
	Number,
	WSpace,
	Keyword,
	Comment,
	Unspecified
};

class LexContext {
public:
	LexContext(std::string lex, lex_type type, std::string ctx = "", std::string file = "", int line = 0);

	std::string lex;
	lex_type type;
	std::string ctx;
	std::string file;
	int line;

	int ctx_depth = 0;

	void depth(int d);
	void make(int s, int e, std::vector<LexContext> lexemes);
};

#endif // LEXCONTEXT_H
