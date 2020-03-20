#include "nanorc.h"

Lexer::Lexer(std::vector<std::string> specifiers) {
	this->specifiers = specifiers;
}

std::vector<std::string> Lexer::onec_operators = {"(", ")", "[", "]", "{", "}", ",", ".", "!",
												  "-", "+", "/", "*", "<", ">", "=", "&", ":",
												  "?", "%", "|", "^"};
std::vector<std::string> Lexer::twoc_operators = {">>", "<<", "::", "==", "<=", ">=", "!=", 
												  "->", "&&", "||"};

void Lexer::lex(std::string file, std::string language) {
//--------------------------------------------------------------------
// Front end for language specific lexing functions. Supported 
// languages are: C++.
//--------------------------------------------------------------------
	this->file = file;
	language = tolower(language);
	if (language == "c++") {
		cpp_lex();
	}
}

void Lexer::cpp_lex() {
//------------------------------------------------------------------------
// Lexer for C++ files
//
// Assumes proper closure of all parenthesis, bracket, brace pairs and
// all strings and multitline comments. 
//
// WARNING: Does not support multiline comment nesting.
//------------------------------------------------------------------------
	std::ifstream f(file);
	sb = RCStreamBuf(f.rdbuf());
	std::string lexeme("");

	int length = get_length(file);
	int prev = -1;

	while (sb.sgetc() != EOF) {
		if (prev == sb.pos()) {
			std::string problem(1, sb.sgetc());
	 		switch (sb.sgetc()) {
	            case '\a':  problem = "\\a";        break;
	            case '\b':  problem = "\\b";        break;
	            case '\f':  problem = "\\f";        break;
	            case '\n':  problem = "\\n";        break;
	            case '\r':  problem = "\\r";        break;
	            case '\t':  problem = "\\t";        break;
	            case '\v':  problem = "\\v";        break;
	            default  :							break;
	        }
			std::cout << "Problem with character "+red << problem << res+" at index "+magenta << sb.pos() << res+" in the context "+red;
			problem = "";
			int s = sb.pos() - 10 > 0 ? sb.pos() - 10 : 0;
			int e = sb.pos() + 10 < length - 1 ? sb.pos() + 10 : length - 1;
			sb.seek(e - s);
			std::cout << sb.gets(e-s) << res << std::endl;
			exit(1);
		}
		prev = sb.pos();
		if (sb.sgetc() == '\'') {
			lexeme += sb.rget();
			do {
				if (sb.sgetc() == '\\') {
					lexeme += sb.rget();
					lexeme += sb.rget();
				}
				lexeme += sb.rget();
			} while(lexeme.back() != '\'');
			if (lexverbose) std::cout << lexeme.size() << " Character literal " << lexeme << std::endl;
			this->append(lexeme, lex_type::Character);
		}
		else if (sb.sgetc() == '\"') {
			lexeme = sb.gets(2);
			while (lexeme.back() != '\"') {
				if (lexeme.back() == '\\') {
					lexeme += sb.rget();
				}
				lexeme += sb.rget();
			}
			if (lexverbose) std::cout << lexeme.size() << " String literal " << lexeme << std::endl;
			this->append(lexeme, lex_type::String);
		}
		else if (sb.sgetc() == '/' && sb.peek() == '*') {
			lexeme = sb.gets(2);
			while (lexeme.find("*/") == std::string::npos) {
				lexeme += sb.rget();
			}
			if (lexverbose) std::cout << lexeme.size() << " Multiline comment " << lexeme << std::endl;
			this->append(lexeme, lex_type::Comment);
		}
		else if (sb.pos() < length - 1 && sb.sgetc() == '/' && sb.peek() == '/') {
			while (sb.sgetc() != EOF && sb.sgetc() != '\n') {
				sb.sbumpc();
			}
		}
		else if (isalpha(sb.sgetc()) || sb.sgetc() == '_' || sb.sgetc() == '~') {
			while (isalpha(sb.sgetc()) || isdigit(sb.sgetc()) || sb.sgetc() == '_' || sb.sgetc() == '~') {
				lexeme += sb.rget();
			}
			if (lexverbose) std::cout << lexeme.size() << " Variable or keyword " << lexeme << std::endl;
			this->append(lexeme, lex_type::Keyword);
		}
		else if (sb.sgetc() == '#') {
			lexeme = sb.rget();
			while (isalpha(sb.sgetc())) {
				lexeme += sb.rget();
			}
			this->append(lexeme, lex_type::Unspecified);
		}
		else if (isdigit(sb.sgetc())) {
			while (isdigit(sb.sgetc()) || sb.sgetc() == '.') {
				lexeme += sb.rget();
			}
			if (lexverbose) std::cout << lexeme.size() << " Numeric literal " << lexeme << std::endl;
			this->append(lexeme, lex_type::Number);
		}
		else if (sb.pos() < length - 1 && isoperator(sb.peek(2)) ) {
			lexeme = sb.gets(2);
			if (lexverbose) std::cout << lexeme.size() << " Double character operator " << lexeme << std::endl;
			this->append(lexeme, lex_type::Operator);
		}
		else if (isoperator(sb.sgetc())) {
			lexeme = sb.rget();
			if (lexverbose) std::cout << lexeme.size() << " Single character operator " << lexeme << std::endl;
			this->append(lexeme, lex_type::Operator);
		}
		else if (isspace(sb.sgetc())) {
			lexeme = sb.rget();
			while (isspace(sb.sgetc())) {
				lexeme += sb.rget();
			}
			std::string wspace("");
			for (auto l : lexeme) {
		 		switch (l) {
		            case '\a':  wspace += "\\a";        break;
		            case '\b':  wspace += "\\b";        break;
		            case '\f':  wspace += "\\f";        break;
		            case '\n':  wspace += "\\n";        break;
		            case '\r':  wspace += "\\r";        break;
		            case '\t':  wspace += "\\t";        break;
		            case '\v':  wspace += "\\v";        break;
		            case ' ' :  wspace += ".";		    break;
		            default  :	wspace += l;		    break;
				}
			}
			if (lexverbose) std::cout << lexeme.size() << " Whitespace " << wspace << std::endl;
			this->append(lexeme, lex_type::WSpace);
		}
		else if (sb.sgetc() == '\\') {
			sb.sbumpc();
			while(isspace(sb.rget())) {}
		}
		else if (sb.sgetc() == ';') {
			lexeme = sb.rget();
			this->append(lexeme, lex_type::Operator);
		}
	}
}

int Lexer::find_new_keywords(std::vector<std::string> &keywords) {
	size_t size = lexemes.size();
	int n, ctx_s, ctx_e = 0;
	for (int ii = 0; ii < size; ii ++) {
		if (lexemes[ii].lex == "class" || lexemes[ii].lex == "namespace") {
			while (ii < size && lexemes[ii++].type != lex_type::Keyword) {}
			if (ii < size) {
				lexemes[++ii].make(ii - 1, ii, lexemes);
				n += add_kw(lexemes[ii], keywords);
			}
			else {
				std::cout << "Extraction of new keywords terminated: ";
				std::cout << "no valid keyword found for specifier.\n";
				return 0;
			}
		}
		else if (lexemes[ii].lex == "typedef") {
			ctx_s = ii;
			while (lexemes[ii].lex != ";") {
				ii++;
				if (ii >= size) {
					std::cout << "Extraction of new keywords terminated: ";
					std::cout << "no teminating semicolon for typedef sentence.\n";
					return 0;
				}
			}
			ctx_e = ii;
			while (lexemes[--ii].type == lex_type::WSpace) {}
			lexemes[ii].make(ctx_s, ii, lexemes);
			n += add_kw(lexemes[ii], keywords);
			ii = ctx_e;
		}
	}
	return n;
}

int Lexer::add_kw(LexContext ctx, std::vector<std::string> &keywords) {
	if (!contains(keywords, ctx.lex)) {
		keywords.push_back(ctx.lex);
		if (verbose) {
			std::cout << "  New keyword found on line " << magenta << ctx.line << res+white+" of file " 
				<< yellow+ctx.file+res+white << ",\n    identified as "
				<< bright+cyan+ctx.lex+res+white+"\n";
		}
		return 1;
	}
	else {
		if (verbose) {
			std::cout << "  Duplicate keyword "+bright+cyan+ctx.lex+res+white+" found on line "+magenta 
				<< ctx.line << res+white+" of file "+yellow+ctx.file+res+white << ",\n    moving on.\n";
		}
	}
	return 0;
}

std::vector<LexContext> Lexer::get_lexemes() {
	return lexemes;
}

void Lexer::append(std::string &lexeme, lex_type type) {
	if (lexeme == "") return;
	lexemes.push_back(LexContext(lexeme, type, "", file, sb.line()));
	lexeme = "";
}

bool Lexer::isoperator(char c) {
	return contains(onec_operators, std::string(1, c));
}

bool Lexer::isoperator(std::string s) {
	return contains(twoc_operators, s);
}

int Lexer::get_length(std::string file) {
	std::ifstream fp(file);
	int length = 0;
	char in;
	while (fp.get(in)) {
		length ++;
	}
	return length;
}

