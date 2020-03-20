/* rcstreambuf.c++
 *
 * William Miller
 * Mar 3, 2020
 *
 */

#include "rcstreambuf.h"

std::streambuf::int_type RCStreamBuf::underflow() {
	return _streambuf->sgetc();
}

std::string RCStreamBuf::gets(int n) {
	std::string s("");
	for (int ii = 0; ii < n; ii ++) {
		s += sbumpc();
	}
	return s;
}

char RCStreamBuf::rget() {
	return sbumpc();
}

char RCStreamBuf::peek() {
	char c = sbumpc();
	sungetc();
	return c;
}

std::string RCStreamBuf::peek(int n) {
	std::string s = gets(n);
	for (int ii = 0; ii < n; ii ++) {
		sungetc();
	}
	return s;
}

void RCStreamBuf::seek(int pos) {
	if (pos > 0) {
		for (int ii = 0; ii < pos; ii ++) {
			sbumpc();
		}
	}
	else {
		for (int ii = pos; ii < 0; ii ++) {
			sungetc();
		}
	}
}

std::streambuf::int_type RCStreamBuf::uflow() {
	int_type rc = _streambuf->sbumpc();

	_prevline = _line;
	if (traits_type::eq_int_type(rc, traits_type::to_int_type('\n'))) {
		++_line;
		_prevcolumn = _column + 1;
		_column = static_cast<unsigned int>(-1);
	}
	++_column;
	++_pos;
	return rc;
}

std::streambuf::int_type RCStreamBuf::pbackfail(std::streambuf::int_type c) {
	int_type rc;
	if (c != traits_type::eof()) {
		rc = _streambuf->sputbackc(traits_type::to_char_type(c));
	}
	else {
		rc = _streambuf->sungetc();
	}
	if (traits_type::eq_int_type(rc, traits_type::to_int_type('\n'))) {
		--_line;
		_prevline = _line;
		_column = _prevcolumn;
		_prevcolumn = 0;
	}
	--_column;
	--_pos;
	return rc;
}
