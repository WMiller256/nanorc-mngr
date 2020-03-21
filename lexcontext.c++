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
	while (s > 0) {
		if (lexemes[s - 1].lex.find("\n") != std::string::npos) {
			break;
		}
		s--;
	}
	while (e < lsize && lexemes[e++ + 1].lex.find("\n") == std::string::npos) {}
	std::string context("    ");
	std::string lex;
	bool highlighted = false;	
	int rs, re;
	for (int ii = s; ii < e; ii ++) {
		if (lexemes[ii].isspecifier && !highlighted) context += green;
		if ((rs = lexemes[ii].lex.find("\n")) != std::string::npos) {
			lex = lexemes[ii].lex;
			re = lex.find_last_of("\n");
			lex.replace(rs, re - rs + 1, "\n    ");
			context += lex;
		}
		else {
			context += lexemes[ii].lex;
		}
		if (lexemes[ii].isspecifier && !highlighted) {
			context += white+bright;
			highlighted = true;			
		}
	}
	this->ctx = context+"\n";
}

void LexContext::print_context() {
	if (ctxverbose) std::cout << " in the context\n" << bright+ctx+res << std::flush;	
}
