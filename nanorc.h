/* 
 * nanorc.h
 *
 * William Miller
 * Aug 12, 2019
 *
 * Header file for nanorc management library. Designed to parse 
 * code files into lexumes, identify and classify keywords to add
 * to nano syntax highlighting by amending existing nanorc file
 * for that language, or by creating a new one. 
 *
 *
 */

#ifndef NANORC_H
#define NANORC_H

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <experimental/filesystem>

#include <boost/program_options.hpp>

#include <ncurses.h>
#include <unistd.h>

#include "colors.h"
#include "rcio.h"

namespace po = boost::program_options;
namespace std{
	namespace filesystem = std::experimental::filesystem;

};

enum lex_type {
	operators,
	characters,
	strings,
	comment,
	keyword
};

extern std::vector<std::string> keywords;

std::vector<std::string> recurse(std::vector<std::string> paths); 
std::vector<std::string> rcParse(std::string rcfile, std::string mode);
std::vector<std::string> lineParse(std::string line, std::vector<std::string>);
std::vector<std::string> codeParse(std::string filename);

void write(std::string filename, std::string mode);

std::vector<std::pair<std::string, lex_type> > lex(std::string file, std::string language);
std::vector<std::pair<std::string, lex_type> > cpp_lex(std::string file);
std::string identify_keyword(std::string line, std::string indicator, 
									  int pos, const int ii);
size_t contains_indicator(std::string line, std::string &ind, int &ii);
bool contains(std::vector<std::string> v, std::string item);					// Return if {v} contains {item}

void sort(std::vector<std::string> &keywords);
void sort(std::vector<std::string> &keywords, std::vector<bool> &changed);

#endif // NANORC_H
