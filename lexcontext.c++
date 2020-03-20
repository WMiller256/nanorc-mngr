/* lexcontext.c++
 *
 * William Miller
 * Mar 19, 2020
 *
 * Implementation of lexeme context class, includes type 
 * information, which file and line the lexeme was extracted
 * from, and a string of the lexemes which surrounded this
 * lexeme.
 *
 */

#include "lexcontext.h"

LexContext::LexContext(std::string lex, lex_type type, std::string ctx, std::string file, int line) {
	this->lex = lex;
	this->type = type;
	this->ctx = ctx;
	this->file = file;
	this->line = line;
}

void LexContext::depth(int d) {
	ctx_depth = d;
}

void LexContext::make(int s, int e, std::vector<LexContext> lexemes) {
	int lsize = lexemes.size() - 1;
	int ctx_s = s - ctx_depth > 0 ? s - ctx_depth : 0;
	int ctx_e = e + ctx_depth < lsize ? e + ctx_depth : lsize;
	while (ctx_s > 0 && lexemes[ctx_s-- - 1].lex.find("\n") == std::string::npos) {}
	while (ctx_e < lsize && lexemes[ctx_e++ + 1].lex.find("\n") == std::string::npos) {}
	std::string context("");
	for (int ii = ctx_s; ii < ctx_e; ii ++) {
		context += lexemes[ii].lex;
	}
	this->ctx = context;
}
