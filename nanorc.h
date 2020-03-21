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
#include <cctype>
#include <experimental/filesystem>

#include <boost/program_options.hpp>

#include <ncurses.h>
#include <unistd.h>

#include "colors.h"
#include "rcio.h"
#include "rcstreambuf.h"
#include "lexcontext.h"
#include "lexer.h"

namespace po = boost::program_options;
namespace std{
	namespace filesystem = std::experimental::filesystem;

};

extern std::vector<std::string> keywords;
extern bool verbose;
extern bool lexverbose;
extern bool ctxverbose;

std::vector<std::string> recurse(std::vector<std::string> paths); 
std::vector<std::string> rcParse(std::string rcfile, std::string mode);
std::vector<std::string> lineParse(std::string line, std::vector<std::string>);

void write(std::string filename, std::string mode);

bool contains(std::vector<std::string> v, std::string item);					// Return if {v} contains {item}

void sort(std::vector<std::string> &keywords);
void sort(std::vector<std::string> &keywords, std::vector<bool> &changed);

#endif // NANORC_H
