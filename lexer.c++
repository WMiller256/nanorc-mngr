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
	language = tolower(language);
	if (language == "c++") {
		cpp_lex(file);
	}
}

void Lexer::cpp_lex(std::string file) {
//------------------------------------------------------------------------
// Lexer for C++ files
//
// Assumes proper closure of all parenthesis, bracket, brace pairs and
// all strings and multitline comments.
//------------------------------------------------------------------------
	std::ifstream fp(file);
	std::string lexeme("");

	int length = get_length(file);
	int idx = 0;
	int prev = -1;

	char in;
	char* code = new char[length];
	while (fp.get(in) ) {					// Read input into character array
		code[idx++] = in;
	}
	idx = 0;
	
	while (idx < length - 1) {
		if (prev == idx) {
			std::cout << "Problem with character "+red << code[idx] << res+" in the context "+red;
			for (int ii = idx-10; ii < idx+10; ii++) {
				if (ii >= 0 && ii < length - 1) {
					std::cout << code[ii];
				}
			}
			std::cout << res << std::endl;
			exit(1);
		}
		prev = idx;
		if (code[idx] == '\'') {
			lexeme += code[idx++];
			do {
				if (code[idx] == '\\') {
					lexeme += code[idx++];
					lexeme += code[idx++];
				}
				lexeme += code[idx++];
			} while(lexeme.back() != '\'');
			if (lexverbose) std::cout << "Character literal " << lexeme << std::endl;
			if (lexeme.length() > 5) {
				for (int ii = idx-10; ii < idx+10; ii++) {
					if (ii >= 0 && ii < length - 1) {
						std::cout << code[ii];
					}
				}				
				exit(-1);
			}
			this->append(lexeme);
		}
		else if (code[idx] == '\"') {
			lexeme = std::string({code[idx++], code[idx++]});
			while (lexeme.back() != '\"') {
				if (lexeme.back() == '\\') {
					lexeme += code[idx++];
				}
				lexeme += code[idx++];
			}
			if (lexverbose) std::cout << "String literal " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (code[idx] == '/' && code[idx + 1] == '*') {
			lexeme = std::string({code[idx++], code[idx++]});
			
			while (lexeme.find("*/") == std::string::npos) {
				lexeme += code[idx++];
			}
			if (lexverbose) std::cout << "Multiline comment " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (idx < length - 1 && code[idx] == '/' && code[idx] == '/') {
			while (code[idx] != EOF && code[idx] != '\n') {
				idx++;
			}
		}
		else if (isalpha(code[idx]) || code[idx] == '_' || code[idx] == '~') {
			while (isalpha(code[idx]) || isdigit(code[idx]) || code[idx] == '_' || code[idx] == '~') {
				lexeme += code[idx++];
			}
			if (lexverbose) std::cout << "Variable or keyword " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (code[idx] == '#') {
			lexeme = code[idx++];
			while (isalpha(code[idx])) {
				lexeme += code[idx++];
			}
			this->append(lexeme);
		}
		else if (isdigit(code[idx])) {
			while (isdigit(code[idx]) || code[idx] == '.') {
				lexeme += code[idx++];
			}
			if (lexverbose) std::cout << "Numeric literal " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (idx < length - 1 && isoperator(std::string({code[idx], code[idx + 1]})) ) {
			lexeme = {code[idx++], code[idx++]};
			if (lexverbose) std::cout << "Double character operator " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (isoperator(code[idx])) {
			lexeme = code[idx++];
			if (lexverbose) std::cout << "Single character operator " << lexeme << std::endl;
			this->append(lexeme);
		}
		else if (isspace(code[idx])) {
			while (isspace(code[idx])) {
				idx++;
			}
		}
		else if (code[idx] == '\\') {
			while(isspace(code[++idx])) {}
		}
		else if (code[idx] == ';') {
			lexeme = code[idx++];
			this->append(lexeme);
		}
	}
}

int Lexer::find_new_keywords(std::vector<std::string> &keywords) {
	size_t size = lexemes.size();
	int n = 0;
	for (int ii = 0; ii < size; ii ++) {
		if (lexemes[ii].first == "class" || lexemes[ii].first == "namespace") {
			if (ii < size - 1) {
				n += add_kw(lexemes[++ii], keywords);
			}
			else {
				std::cout << "Extraction of new keywords terminated: ";
				std::cout << "last lexeme was specifier.";
			}
		}
		else if (lexemes[ii].first == "typedef") {
			while (lexemes[ii + 1].first != ";") {
				ii++;
				if (ii + 1 > size - 1) {
					std::cout << "Extraction of new keywords terminated: ";
					std::cout << "last lexeme was specifier.";
				}
			}
			n += add_kw(lexemes[ii], keywords);
		}
	}
	return n;
}

int Lexer::add_kw(std::pair<std::string, std::tuple<std::string, std::string, int> > lexeme, std::vector<std::string> &keywords) {
	if (!contains(keywords, lexeme.first)) {
		keywords.push_back(lexeme.first);
		auto [context, file, line] = lexeme.second;
		if (verbose) {
			std::cout << "Keyword specifier found in line "
				<< magenta << line << res+white+" of file " << yellow+file+res+white 
				<< std::endl;
			std::cout << res+white << std::endl;
			std::cout << "  Keyword identified as "+bright+cyan+lexeme.first+res+white+"\n";
		}
		return 1;
	}
	return 0;
}

std::vector<std::pair<std::string, std::tuple<std::string, std::string, int> > > Lexer::get_lexemes() {
	return lexemes;
}

void Lexer::append(std::string &lexeme) {
	if (lexeme == "") return;
	std::tuple meta = std::make_tuple("", "", 0);
	lexemes.push_back(std::make_pair(lexeme, meta));
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

