/* 
 * nanorc.c++
 *
 * William Miller
 * Jun 26, 2019
 *
 * Parses code files and adds keywords to desired nanorc 
 * Keywords included are type, structure, class, and 
 * namespace declarations. 
 *
 */

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

namespace po = boost::program_options;
namespace std{
	namespace filesystem = std::experimental::filesystem;
};

std::vector<std::string> recurse(std::vector<std::string> paths); 
std::vector<std::string> rcParse(std::string rcfile, std::string mode);
std::vector<std::string> lineParse(std::string line, std::vector<std::string>);
std::vector<std::string> codeParse(std::string filename);

void write(std::string filename, std::string mode);

std::string identify_keyword(std::string line, std::string indicator, 
									  int pos, const int ii);
size_t contains_indicator(std::string line, std::string &ind, int &ii);

void sort(std::vector<std::string> &keywords);
void sort(std::vector<std::string> &keywords, std::vector<bool> &changed);

void print_table(std::vector<std::string> strings, 
					  std::vector<bool> changed = std::vector<bool>(), 
					  std::string mode = "user", int tabsize=4);
void tab(int tabsize=8);
std::string tolower(std::string str);

bool verbose;
std::vector<std::string> files;
std::vector<std::string> keywords;
std::string current_file;
int current_line;
std::string keywordColor;
std::vector<std::string> indicators;

static std::vector<std::string> extensions = {"h", "h++", "hpp", "c", "c++", "cpp"};
static std::vector<std::string> rgx = {"[^A-Za-z0-9\\_]", "^", "[^A-Za-z0-9\\_]", "^"};
static std::vector<std::string> sfx = {"[^A-Za-z0-9\\_]*", "[^A-Za-z0-9\\_]", "$", "$"};

int main(int argn, char** argv) {
	std::string ofile("");
	bool lib;
	bool user;
	bool recursive;
	std::string mode;
	po::options_description description("Allowed Options");
	po::positional_options_description positional;

	current_file = "";
	current_line = 0;
	
	std::cout << white;

	try {
		description.add_options()
			("files,f", po::value<std::vector<std::string> >()->multitoken(), 
					"The files to parse")
			("rcfile,o", po::value<std::string>(&ofile)->default_value(".rc"),
					"The rc (output) file.")
			("color,c", po::value<std::string>(&keywordColor)->
					default_value("default"), "The desired color to use for" 
					" syntax highlighting.")
			("user, u", po::bool_switch()->default_value(false), 
					"Parsing mode for user defined keywords.")
			("lib, l", po::bool_switch()->default_value(false), 
					"Parsing mode for library keywords.")
			("indicators,i",po::value<std::vector<std::string> >()->multitoken(), 
					"The keyword indicators to include - C++ defaults are 'typedef',"
					"'struct', 'class', 'namespace'")
			("verbose,v", po::bool_switch()->default_value(false), "Enable verbose"
					" output.")
			("recursive,r", po::bool_switch()->default_value(false), "Enable recursive"
					"searcing.") 
	    ;
	    positional.add("color", 1);
	    positional.add("rcfile", 1);
	    
	}
	catch (...) {
		std::cout << "Error in boost initialization\n" << std::endl; 
	}

	po::variables_map vm;
	po::store(po::command_line_parser(argn, argv).options(description)
			.positional(positional).run(), vm);
	po::notify(vm);
	
	
	lib = vm["lib"].as<bool>();
	user = vm["user"].as<bool>();
	verbose = vm["verbose"].as<bool>();
	recursive = vm["recursive"].as<bool>();
	if (vm.count("files")) files = vm["files"].as<std::vector<std::string> >();
	if (vm.count("indicators")) indicators = vm["indicators"].as<std::vector<std::string> >();
	if (lib == false && user == false) user = true;
	if (lib && user) lib = false;
	if (keywordColor == "default" && lib) keywordColor = "brightyellow";
	if (keywordColor == "default" && user) keywordColor = "brightcyan";
	if (indicators.empty()) {
		indicators = {"typedef", "struct", "class", "namespace"};
	}
	
	if (lib) {
		mode = "lib";
	}
	else {
		mode = "user";
	}
	
	std::string pref = "User";
	if (std::filesystem::exists(ofile)) {
		keywords = rcParse(ofile, mode);
		if (lib) {
			pref = "Library";
		}
		if (keywords.size() != 0 && verbose) {
			std::cout << "Current "+bright+pref+" Keyword"+res+white+" set is as follows.\n";
			sort(keywords);
			print_table(keywords, std::vector<bool>(), mode);
		}
		else {
			if (verbose) {
				std::cout << "No "+bright+pref+" Keywords"+res+white+" found." << std::endl;
			}
		}
	}
	std::vector<bool> changed;
	for (int ii = 0; ii < keywords.size(); ii ++) {
		changed.push_back(false);
	}

	int nkeywords = keywords.size();
	
	if (recursive) {
		files = recurse(files);
		std::cout << "Recursive search found the following files:\n";
		for (int ii = 0; ii < files.size(); ii ++) {
			std::cout << bright+yellow << files[ii] << res+white << std::endl;
		}	
	}
	
	for (int ii = 0; ii < files.size(); ii ++) {
		if (std::filesystem::exists(files[ii])) {
			std::vector<std::string> newKeywords = codeParse(files[ii]);
			keywords.insert(keywords.end(), newKeywords.begin(), 
					newKeywords.end());
		}
	}
	int unchanged = changed.size();
	for (int ii = unchanged; ii < keywords.size(); ii ++) {
		changed.push_back(true);
	}

	sort(keywords, changed);

	if (nkeywords != keywords.size()) {
		std::cout << "After parsing "+bright+magenta << files.size() << res+white+" files";
		std::cout << " the "+bright+pref+" Keyword"+res+white+" set is now\n";
		print_table(keywords, changed, mode);
	
		std::string in;
		
		std::cout << "The output path is "+bright+yellow+ofile+res+white+"\n";
		std::cout << "Would you like to commit these changes to file? [y/n] "
				<< std::flush;
		std::cin >> in;
		if (tolower(in) == "y" || tolower(in) == "yes") {
			write(ofile, mode);
		}
		
	}
	else {
		std::cout << "After parsing "+bright+magenta << files.size() << res+white+
				" files, no new keywords were found - the "+bright+pref+" Keyword"
				+res+white+" set is unchanged." << std::endl;
	}
	
}

std::vector<std::string> recurse(std::vector<std::string> paths) {
	std::vector<std::string> files;
	std::error_code ec;
	size_t pos;
	for (int ii = 0; ii < paths.size(); ii ++) {
		try {
			if (std::filesystem::exists(paths[ii])) {
				if (std::filesystem::is_directory(paths[ii])) {
					if (verbose) {
						std::cout << bright+red << paths[ii] << res+white;
						std::cout << " is directory, searching...\n";
					}
					std::filesystem::recursive_directory_iterator iter(paths[ii]);
					std::filesystem::recursive_directory_iterator end;
						// Default constructor points to std::end
					while (iter != end) {
						if (verbose) {
							std::cout << "    " << iter->path().string() << std::endl;
						}
						pos = iter->path().string().find_last_of(".") + 1;
						if (pos != std::string::npos && (std::find(extensions.begin(), 
							  extensions.end(), tolower(iter->path().string().substr(pos))) 
							  != extensions.end())) {
							files.push_back(iter->path().string());	
						}
						iter.increment(ec);
						if (ec) {
							std::cout << "Error while accessing " << 
								iter->path().string() << " -- " << ec.message() << "\n";
								
						}
					}
					
				}
				else {
					pos = paths[ii].find_last_of(".") + 1;
					if (pos != std::string::npos && (std::find(extensions.begin(), 
						  extensions.end(), tolower(paths[ii].substr(pos))) 
						  != extensions.end())) {
						files.push_back(paths[ii]);
					}
				}
			}
		}
		catch (std::system_error &error) {
			std::cerr << "Exception -- " << error.what();
		}
		catch (...) {
			std::cerr << "Exception -- " << " Unknown Exception caught on line " <<
				__LINE__ << " in file "+bright+yellow << __FILE__ << res+white+"." 
				<< std::endl;
		}
	}

	return files;
}

std::vector<std::string> rcParse(std::string rcfile, std::string mode) {
	std::ifstream file(rcfile);
	std::string line("");
	std::vector<std::string> keywords;
	std::string key = "## custom keywords";
	if (verbose) std::cout << "Parsing existing keywords... \n"; 
	if (mode == "lib") {
		key = "## library keywords";
	}
	while (true) {
		if (std::getline(file,line)) {
			size_t start = line.find_first_not_of(" \t");
			if (start != std::string::npos) {
				line = line.substr(start); 
			}
			if (tolower(line) == key) {
				std::vector<std::string> parsed;
				while (true) {
					std::getline(file, line);
					size_t start = line.find_first_not_of(" \t");
					size_t end = line.find_first_of("\"");
					std::string prefix("");
					if (start != std::string::npos && 
						end != std::string::npos) {
						prefix = line.substr(start, end-start-1);
					}
					else if (line.length() > start && line[start] == '#') {
						continue;	
					}
					else {
						if (verbose) {
							std::cout << "End of existing keywords found - "
									 "no matching quotations.\n";
						}
						break;
					}
					if (prefix == "color "+keywordColor) {
						parsed = lineParse(line, keywords);
						keywords.insert(keywords.end(), 
							parsed.begin(), parsed.end());
					}
					else {
						if (verbose) {
							std::cout << "End of existing keywords found - "
									 "prefix ("+prefix+") does not match 'color "
									 +keywordColor+"'.\n"; 
						}
						break; 
					}
				}
				break;
			}
		}
		else {
			break;
		}	
	}
	file.close();
	return keywords;
}

std::vector<std::string> lineParse(std::string line, std::vector<std::string> keywords) {
	std::vector<std::string> parsed;
	size_t start = line.find_first_of("(")+1;
	size_t end = line.find_last_of(")");
	size_t pos = 0;
	std::string token;
	if (start != std::string::npos && end != std::string::npos) {
		line = line.substr(start, end-start);
		while ((pos = line.find("|")) != std::string::npos) {
		    token = line.substr(0, pos);
		    line.erase(0, pos + 1);
		    if (std::find(keywords.begin(), keywords.end(), token) == keywords.end()) {
		    	if (verbose) {
		    		std::cout << "Keyword "+bright+magenta+token+res+white+" added.\n";
		    	}
		    	parsed.push_back(token);
		    }
		    else {
		    	if (verbose) {
		    		std::cout << "Duplicate keyword "+bright+magenta+token+res+white
		    			+" found.\n";
		    	}
		    }
		}
		token = line;		
		if (std::find(keywords.begin(), keywords.end(), token) == keywords.end()) {
			if (verbose) {
		    	std::cout << "Keyword "+bright+magenta+token+res+white+" added.\n";
			}
		    parsed.push_back(token);
		}
		else {
			if (verbose) {
		    	std::cout << "Duplicate keyword "+bright+magenta+token+res+white
		    		+" found.\n";
			}
		}
	}
	return parsed;
}

std::vector<std::string> codeParse(std::string filename) {
	std::ifstream file(filename);
	current_file = filename;
	std::vector<std::string> parsed;
	std::string line;
	std::string keyword("");
	current_line = 0;
	while (std::getline(file, line)) {
		int ii = 0;
		size_t pos;
		std::string ind("");
		size_t comment;
		keyword = "";
		current_line ++;
		if ((comment = line.find("/*")) != std::string::npos) {
			if ((pos = contains_indicator(line, ind, ii)) < comment) {
				keyword = identify_keyword(line.substr(0, comment), ind, pos, ii);
			}
			while (std::getline(file, line)) {
				current_line ++;
				if ((comment = line.find("*/")) != std::string::npos) {
					if ((pos = contains_indicator(line, ind, ii)) > comment &&
						 pos != std::string::npos) {
						pos = contains_indicator(line.substr(comment), ind, ii);
						keyword = identify_keyword(line.substr(comment), ind, pos, ii);
					}
					std::getline(file, line);
					current_line ++;
					break;
				}
			}
		}
		if ((pos = contains_indicator(line, ind, ii)) != std::string::npos) {
			keyword = identify_keyword(line, ind, pos, ii);
		}
		if (keyword != "") { 
			parsed.push_back(keyword);
		}
	}
	file.close();
	return parsed;
}

void write(std::string filename, std::string mode) {
	std::ifstream file(filename);
	std::string line("");
	std::string orig("");
	std::string key = "## custom keywords";
	std::vector<std::string> lines;
	if (mode == "lib") {
		key = "## library keywords";
	}
	while (true) {
		if (std::getline(file, line)) {
			orig = line;
			size_t start = line.find_first_not_of(" \t");
			if (start != std::string::npos) {
				line = line.substr(start);
			}
			if (tolower(line) == key) {
				lines.push_back(line);
				while (true) {
					std::getline(file, line);
					start = line.find_first_not_of(" \t");
					size_t end = line.find_first_of("\"");
					std::string prefix("");
					if (start != std::string::npos && end != std::string::npos) {
						prefix = line.substr(start, end-start-1);
					}
					else if (line.length() > start && line[start] == '#') {
						continue;
					}
					else {
						break;
					}
					if (prefix == "color "+keywordColor) {
						while (prefix == "color "+keywordColor) {
							std::getline(file, line);
							start = line.find_first_not_of(" \t");
							end = line.find_first_of("\"");
							prefix = "";
							if (start != std::string::npos && end != std::string::npos) {
								prefix = line.substr(start, end-start-1);
							}
							else if (line.length() > start && line[start] =='#') {
								continue;
							}
							else {
								break;
							}
						}
						std::string out("");
						for (int kk = 0; kk < rgx.size(); kk ++) {
							std::string prefix("\tcolor "+keywordColor+" \""
											   +rgx[kk]+"(");
							std::string suffix(")"+sfx[kk]+"\"");
							out = prefix;
							for (int ii = 0; ii < keywords.size(); ii ++) {
								if (ii != 0 && ii % 10 != 0) {
									out = out+"|";
								}
								out = out+keywords[ii];
								if (ii % 10 == 9) {
									lines.push_back(out+suffix);
									out = prefix;
								}
							}
							lines.push_back(out+suffix);
						}
						lines.push_back("");
						break;
					}
				}
			}
			else {
				lines.push_back(orig);
			}
		}
		else {
			break;
		}
	}
	std::ofstream ofile(filename);
	for (int ii = 0; ii < lines.size(); ii ++) {
		ofile << lines[ii] << "\n";		
	}
	
	file.close();
	return;
}

std::string identify_keyword(std::string line, std::string indicator, int pos, const int ii) {
	std::string keyword;
	std::string _keyword;
	try {
		keyword = line.substr(pos);
		keyword = keyword.substr(keyword.find_first_of(indicator));
		_keyword = keyword.substr(keyword.find_first_of(" \t") + 1);
		pos = _keyword.find_first_of(":{;");
		if (pos != std::string::npos) {
			_keyword = _keyword.substr(0, pos);
		}
		while (_keyword.find_first_of("<>::") != std::string::npos
				 && _keyword.find_first_of(" \t") != std::string::npos) {
			keyword = keyword.substr(keyword.find_first_of(" \t")+1);
			_keyword = keyword;
		}
		keyword = _keyword;
//		}
//		else if (std::isspace(line[pos - 1])) {
/*			if (keyword.find("{") > keyword.find(";")) {  // }
				keyword = keyword.substr(0, keyword.find_first_of(";"));
				keyword = keyword.substr(keyword.find_last_of(" \t"));
			}
			else if (keyword.find("{") < keyword.find(";")) {  // }
				keyword = keyword.substr(0, keyword.find_first_of(" \t;"));		
			}
*/
	}
	catch (std::system_error &error) {
		std::cerr << "Exception -- " << error.what();
	}
	catch (...) {
		std::cerr << "Exception -- " << " Unknown Exception caught on line " <<
			__LINE__ << " in file "+bright+yellow << __FILE__ << res+white+"." 
			<< std::endl;
		std::cerr << "Exception raised while parsing the line\n" << line << std::endl;
	}
	if (std::find(keywords.begin(), keywords.end(), 
		keyword) == keywords.end()) {	
		if (verbose) {
			std::cout << "Keyword indicator "+bright+green+indicators[ii]+res+white+" found in line "
				<< magenta << current_line << res+white+" of file " << yellow+current_file+res+white 
				<< "\n    " << bright+line.substr(line.find_first_not_of(" \t"))+res+white
				<< std::endl;
			std::cout << "    ";
			std::cout << green;
			for (int kk = 0; kk < indicators[ii].size(); kk ++) {
				std::cout << "*";
			}
			std::cout << res+white << std::endl;
			std::cout << "  Keyword identified as "+bright+cyan+keyword+res+white+"\n";
		}
	}
	else {
		keyword = "";
	}
	return keyword;
}

size_t contains_indicator(std::string line, std::string &ind, int& ii) {
	size_t pos = std::string::npos;
	for (ii = 0; ii < indicators.size(); ii ++) {
		if ((pos = line.find(indicators[ii]+" ")) != std::string::npos
			&& line.find("//") > pos) {
			ind = indicators[ii];
			break;
		}
		else {
			ind = "";
			pos = std::string::npos;
		}
	}
	return pos;
}

void sort(std::vector<std::string> &keywords) {
	for (int ii = 0; ii < keywords.size(); ii ++) {
		for (int jj = ii; jj < keywords.size(); jj ++) {
			if (keywords[ii].compare(keywords[jj]) > 0) {
				std::swap(keywords[ii], keywords[jj]);
			} 
		}
	}
	return;
}

void sort(std::vector<std::string> &keywords, std::vector<bool> &changed) {
	int nkeywords = keywords.size();
	int nchanged = changed.size();
	if (nchanged != nkeywords) {
		std::cout << "There was a problem, the number of keywords does not";
		std::cout << " match the size of the change indexor" << std::endl;
		return;
	}
	for (int ii = 0; ii < keywords.size(); ii ++) {
		for (int jj = ii; jj < keywords.size(); jj ++) {
			if (keywords[ii].compare(keywords[jj]) > 0) {
				std::swap(keywords[ii], keywords[jj]);
				if (ii < nchanged && jj < nchanged) {
					std::swap(changed[ii], changed[jj]);
				}
			} 
		}
	}
	return;
}

void print_table(std::vector<std::string> strings, std::vector<bool> changed, 
		std::string mode, int tabsize) {
    int nelements = strings.size();
    int max_length = 0;
    for (int ii = 0; ii < nelements; ii ++) {
        if (strings[ii].length() > max_length) {
            max_length = strings[ii].length();
        }
    }
    // Get the screen width and height to print driver names more cleanly
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
    int ncolumns = width/max_length;
    if (ncolumns == 0) ncolumns = 1;
    int nrows = nelements/ncolumns;

    tab(tabsize);
    while (ii < nrows) {
        for (jj = 0; jj <= ncolumns; jj ++) {
            if (ii+jj*nrows >= strings.size()) {
                ncolumns--;
                std::cout << std::endl;
                tab(tabsize);
                break;
            }
            if (mode == "user") {
       			std::cout << bright+cyan;
       		}
       		else if (mode == "lib") {
       			std::cout << bright+yellow;
       		}
			if (keywords.begin(), std::find(keywords.begin(), 
				  keywords.end(), strings[ii+jj*nrows]) != keywords.end()) {
				int pos = std::distance(keywords.begin(), std::find(
				  keywords.begin(), keywords.end(), strings[ii+jj*nrows]));
				if (pos < changed.size() && changed[pos]) {	
            		std::cout << bright+green;
            	}
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
