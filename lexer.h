/* lexer.h
 *
 * William Miller
 * Mar 19, 2020
 *
 * Lexer class with methods for parsing, categorizing, and 
 * extracting the desired keywords from the given code file
 * in a pseudocomprehensive fashion.
 *
 */

#ifndef LEXER_H
#define LEXER_H

#include "nanorc.h"

class Lexer {
private:
	static std::vector<std::string> onec_operators;
	static std::vector<std::string> twoc_operators;

	std::vector<std::string> specifiers;
	std::vector<LexContext> lexemes;

	std::string file;
	RCStreamBuf sb;

	int ctx_depth = 5;

public:
	Lexer(std::vector<std::string> specifiers = {});

	void lex(std::string file, std::string language);
	void cpp_lex();
	int find_new_keywords(std::vector<std::string> &keywords);
	int add_kw(LexContext context, std::vector<std::string> &keywords);
	std::string make_context(int start, int end);
	std::vector<LexContext> get_lexemes();
	void append(std::string &lexeme, lex_type type);

	bool isoperator(char c);
	bool isoperator(std::string s);
	int get_length(std::string file);
};

#endif // LEXER_H
