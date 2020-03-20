/* rcstreambuf.h
 * 
 * William Miller
 * Mar 3, 2020
 *
 * Implementation of class derived from std::streambuf through which a
 * std::ifstream can be 'piped' and will track the line number irrespective
 * of the method for reading (i.e. getline, getc, etc)
 *
 */

#ifndef RCSTREAMBUF_H
#define RCSTREAMBUF_H

#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>

class RCStreamBuf : public std::streambuf {
public:
	RCStreamBuf() {}
	RCStreamBuf(std::streambuf* buf) : 
		_streambuf(buf), _line(1), _prevline(0), _column(0),
		_prevcolumn(static_cast<unsigned int>(-1)), _pos(0) {}

	unsigned int line() const { return _line; }
	unsigned int prevline() const { return _prevline; }
	unsigned int column() const { return _column; }
	std::streamsize pos() const { return _pos; }

	std::string gets(int n = 1);
	char rget();
	char peek();
	std::string peek(int n);

	void seek(int pos);

protected:
	RCStreamBuf(const RCStreamBuf&);
	RCStreamBuf& operator=(const RCStreamBuf*);

	std::streambuf::int_type underflow();
	std::streambuf::int_type uflow();
	std::streambuf::int_type pbackfail(std::streambuf::int_type c);

private:
	std::streambuf* _streambuf;
	unsigned int _line;
	unsigned int _prevline;
	unsigned int _column;
	unsigned int _prevcolumn;
	std::streamsize _pos;
};

#endif // RCSTREAMBUF_H
