/*
UTF-8 Iterator. Version 0.1.3

Original code by Adrian Guerrero Vera (adrianwk94@gmail.com)
MIT License
Copyright (c) 2016 Adrian Guerrero Vera

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef utf8_iter_H
#define utf8_iter_H

#include <stdint.h>

typedef struct utf8_iter {

	const char* ptr;
	uint32_t	codepoint;

	uint8_t		size; 		// character size in bytes
	uint32_t 	position; 	// current character position
	uint32_t 	next; 		// next character position
	uint32_t 	count; 		// number of counter characters currently
	uint32_t 	length;		// strlen()

} utf8_iter;

void 			utf8_init			(utf8_iter* iter, const char* ptr); // all values to 0 and set ptr.
void			utf8_initEx			(utf8_iter* iter, const char* ptr, uint32_t length); // allows you to set a custom length.

uint8_t			utf8_next			(utf8_iter* iter); // returns 1 if there is a character in the next position. If there is not, return 0.
uint8_t			utf8_previous		(utf8_iter* iter); // returns 1 if there is a character in the back position. If there is not, return 0.

const char* 	utf8_getchar		(utf8_iter* iter); // return current character in UFT8 - no same that iter.codepoint (not codepoint/unicode)

// Utilities
uint32_t 		utf8_strlen			(const char* string);
uint32_t 		utf8_strnlen		(const char* string, uint32_t end);
uint32_t		utf8_to_unicode		(const char* character); // UTF8 to Unicode.
const char* 	unicode_to_utf8		(uint32_t codepoint); // Unicode to UTF8.

// Internal use / Advanced use.
uint8_t			utf8_charsize		(const char* character); // calculate the number of bytes a UTF8 character occupies in a string.
uint8_t			unicode_charsize	(uint32_t codepoint); // calculates the number of bytes occupied by a Unicode character in UTF8.

uint32_t 		utf8_converter		(const char* character, uint8_t size);
const char* 	unicode_converter	(uint32_t codepoint, uint8_t size);

#endif
