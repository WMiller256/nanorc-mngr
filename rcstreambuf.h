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

#include <streambuf>

class RCStreamBuf : public std::streambuf {
public:
	RCStreamBuf() {}
	RCStreamBuf(std::streambuf* buf) : _streambuf(buf), _line(1) {}

	unsigned int line() const { return _line; }

private:
	std::streambuf* _streambuf;
	unsigned int _line;
};

#endif // RCSTREAMBUF_H
