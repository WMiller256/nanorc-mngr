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

#include "nanorc.h"

bool verbose;
bool lexverbose;
bool ctxverbose;
std::vector<std::string> files;
std::vector<std::string> keywords;
std::string keywordColor;
std::vector<std::string> specifiers;

static std::vector<std::string> extensions = {"h", "h++", "hpp", "c", "c++", "cpp"};
static std::vector<std::string> rgx = {"[^A-Za-z0-9\\_]", "^", "[^A-Za-z0-9\\_]", "^"};
static std::vector<std::string> sfx = {"[^A-Za-z0-9\\_]*", "[^A-Za-z0-9\\_]", "$", "$"};

static std::vector<std::string> cpp_types = {"bool", "int", "char", "true", "false", "float", 
											  "double", "long", "signed", "unsigned"};
static std::vector<std::string> cpp_kwords = {"for", "if", "while", "do while", "break", 
											  "namespace", "typedef", "enum", "exit", 
											  "return", "try", "catch", "else", "continue"};

int main(int argn, char** argv) {
	std::string ofile("");
	bool lib;
	bool user;
	bool recursive;
	std::string mode;
	std::vector<std::string> to_add;
	po::options_description description("Allowed Options");
	po::positional_options_description positional;

	std::cout << white;

	// TODO - add disinclusion option
	// TODO - add option to edit the new keyword set before writing to file
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
			("specifiers,i",po::value<std::vector<std::string> >()->multitoken(), 
				"The keyword specifiers to include - e.g. for C++: 'typedef',"
				"'class', 'namespace'")
			("verbose,v", po::bool_switch()->default_value(false), "Enable verbose"
				" output.")
			("lexverbose,l", po::bool_switch()->default_value(false), "Enable verbose"
				" lexing output.")
			("ctxverbose,x", po::bool_switch()->default_value(false), "Enable verbose"
				" keyword context output.")
			("recursive,r", po::bool_switch()->default_value(false), "Enable recursive"
				"searcing.") 
			("add", po::value<std::vector<std::string> >()->multitoken(),
				"Add a given keyword or set of keywords to the rc file")
	    ;
	    positional.add("color", 1);
	    positional.add("rcfile", 1);
	    
	}
	catch (...) {
		std::cout << "Error in boost initialization\n" << std::endl; 
	}

	po::variables_map vm;
	po::store(po::command_line_parser(argn, argv).options(description)
			. positional(positional).run(), vm);
	po::notify(vm);
	
	
	lib = vm["lib"].as<bool>();
	user = vm["user"].as<bool>();
	verbose = vm["verbose"].as<bool>();
	lexverbose = vm["lexverbose"].as<bool>();
	ctxverbose = vm["ctxverbose"].as<bool>();
	recursive = vm["recursive"].as<bool>();
	if (vm.count("files")) files = vm["files"].as<std::vector<std::string> >();
	if (vm.count("specifiers")) specifiers = vm["specifiers"].as<std::vector<std::string> >();
	if (vm.count("add")) to_add = vm["add"].as<std::vector<std::string> >();
	if (lib == false && user == false) user = true;
	if (lib && user) lib = false;
	if (keywordColor == "default" && lib) keywordColor = "brightyellow";
	if (keywordColor == "default" && user) keywordColor = "brightcyan";
	if (specifiers.empty()) {
		specifiers = {"typedef", "class", "namespace"};
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
	if (!to_add.empty()) {
		for (auto a : to_add) {
			keywords.push_back(a);
			changed.push_back(true);
		}
		std::cout << "After processing "+bright+magenta << to_add.size() << res+white+" new keywords, " << std::flush;
	}
	else {		
		if (recursive) {
			files = recurse(files);
			std::cout << "Recursive search found the following files:\n";
			for (int ii = 0; ii < files.size(); ii ++) {
				std::cout << bright+yellow << files[ii] << res+white << std::endl;
			}
		}
		else {
			auto f = std::begin(files);
			while (f != std::end(files)) {
				if (!contains(extensions, tolower(f->substr(f->find_last_of(".") + 1))) ) {
					f = files.erase(f);
				}
				else {
					f++;
				}
			}
		}
		Lexer lexer = Lexer(specifiers);
		int nchanged;
		for (auto file: files) {
			if (std::filesystem::exists(file)) {
				std::cout << "Lexing "+yellow << file << res+white << " ... " << std::flush;
				if (verbose) std::cout << "\n";
				lexer.lex(file, "c++");
				nchanged = lexer.find_new_keywords(keywords);
				if (!verbose) std::cout << bright+green+" complete"+res+white+".\n" << std::flush;
			}
		}
		int unchanged = changed.size();
		for (int ii = unchanged; ii < keywords.size(); ii ++) {
			changed.push_back(true);
		}
		std::cout << "After parsing "+bright+magenta << files.size() << res+white+" files, ";
	}

	sort(keywords, changed);

	if (nkeywords != keywords.size()) {
		std::cout << "the "+bright+pref+" Keyword"+res+white+" set is now\n";
		print_table(keywords, changed, mode);
	
		std::string in;
		
		std::cout << "The output path is "+bright+yellow+ofile+res+white+"\n";
		std::cout << "Would you like to commit these changes to file? [y/n] \n"
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
						std::cout << tolower(iter->path().string().substr(pos)) << std::endl;
						if (pos != std::string::npos && contains(extensions, tolower(iter->path().string().substr(pos))) ) {
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
void write(std::string filename, std::string mode) {
	std::ifstream file(filename);
	std::string line("");
	std::string orig("");
	std::string key = "## custom keywords";
	std::vector<std::string> lines;
	if (mode == "lib") {
		key = "## library keywords";
	}
	while (std::getline(file, line)) {
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
	std::ofstream ofile(filename);
	for (int ii = 0; ii < lines.size(); ii ++) {
		ofile << lines[ii] << "\n";		
	}
	
	file.close();
	return;
}

bool contains(std::vector<std::string> v, std::string item) {
	return (std::find(v.begin(), v.end(), item) != v.end());
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

