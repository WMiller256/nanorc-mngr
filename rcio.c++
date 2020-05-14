/*
 * rcio.c++
 * 
 * William Miller
 * Nov 18, 2019
 *
 * I/O functions for the nrc-mgmt library
 *
 */

#include "rcio.h"

void print_table(std::vector<std::string> strings, std::vector<bool> changed, 
		std::string mode, int tabsize) {
	int nelements = strings.size();
	if (nelements == 0) {
		std::cout << "Table is empty" << std::endl;
		return;
	}
	int max_length = 0;
	for (int ii = 0; ii < nelements; ii ++) {
		if (strings[ii].length() > max_length) {
			max_length = strings[ii].length();
		}
	}
	// Get the screen width and height to print table cleanly
	int width;
	int height;
	initscr();
	getmaxyx(stdscr, height, width);
	endwin();
	width = width - max_length - 10;                // Put {tabsize} plus one column width buffer
	if (width <= 0) {
		width = 100;
	}
	if (max_length <= 0) {
		max_length = 10;
	}
	int ii = 0;
	int jj = 0;
	int pos;
	int ncolumns = (width/max_length == 0 ? 1 : width/max_length);
	int nrows    = (nelements/ncolumns == 0 ? 1 : nelements/ncolumns);

	tab(tabsize);
	while (ii < nrows) {
		for (jj = 0; jj <= ncolumns; jj ++) {
			if (ii+jj*nrows >= strings.size()) {
				ncolumns--;
				std::cout << std::endl;
				tab(tabsize);
				break;
			}
			if (mode == "user") std::cout << bright+cyan;
			else if (mode == "lib") std::cout << bright+yellow;
			else if (mode == "builtin") std::cout << green;
			if (keywords.begin(), std::find(keywords.begin(), 
				 keywords.end(), strings[ii+jj*nrows]) != keywords.end()) {
				int pos = std::distance(keywords.begin(), std::find(
				keywords.begin(), keywords.end(), strings[ii+jj*nrows]));
				if (pos < changed.size() && changed[pos]) std::cout << bright+green;
			}
			std::cout << std::left << std::setw(max_length) << strings[ii+jj*nrows];
			std::cout << " " << white+res;
			if (jj % (ncolumns+1) == ncolumns) {
				std::cout << std::endl;
				tab(tabsize);
			}
		}
		ii ++;
	}
	std::cout << std::endl;
}

void tab(int tabsize) {
	for (int ii = 0; ii < tabsize; ii ++) {
		std::cout << " ";
	}
}

std::string tolower(std::string str) {
	std::string ret;
	for (int ii = 0; ii < str.size(); ii ++) {
		ret += std::tolower(str[ii]);
	}
	return ret;
}

