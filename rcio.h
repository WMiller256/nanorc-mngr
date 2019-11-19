/*
 * rcio.h
 *
 * William Miller
 * Nov 18, 2019
 *
 * Header file for I/O functions for the nrc-mgmt library
 *
 */

#ifndef RCIO_H
#define RCIO_H

#include "nanorc.h"

void print_table(std::vector<std::string> strings, 
					  std::vector<bool> changed = std::vector<bool>(), 
					  std::string mode = "user", int tabsize=4);
void tab(int tabsize=8);
std::string tolower(std::string str);

#endif // RCIO_H
