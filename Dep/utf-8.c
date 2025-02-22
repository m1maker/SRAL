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

#include "utf-8.h"

#include <string.h>

//utf8_iter

void utf8_init(utf8_iter* iter, const char* ptr) {
	if (iter) {
		iter->ptr       = ptr;
		iter->codepoint = 0;
		iter->position  = 0;
		iter->next      = 0;
		iter->count     = 0;
		iter->length    = ptr == NULL ? 0 : strlen(ptr);
	}
}

void utf8_initEx(utf8_iter* iter, const char* ptr, uint32_t length) {
	if (iter) {
		iter->ptr       = ptr;
		iter->codepoint = 0;
		iter->position  = 0;
		iter->next      = 0;
		iter->count     = 0;
		iter->length    = length;
	}
}

uint8_t	utf8_next(utf8_iter* iter) {

	if (iter == NULL) return 0;
	if (iter->ptr == NULL) return 0;

	const char* pointer;

	if (iter->next < iter->length) {

		iter->position = iter->next;

		pointer 	= iter->ptr + iter->next; //Set Current Pointer
		iter->size 	= utf8_charsize(pointer);

		if (iter->size == 0) return 0;

		iter->next 		= iter->next + iter->size;
		iter->codepoint = utf8_converter(pointer, iter->size);

		if (iter->codepoint == 0) return 0;

		iter->count++;

		return 1;
	}
	else {
		iter->position = iter->next;
		return 0;
	}
}

uint8_t	utf8_previous(utf8_iter* iter) {

	if (iter == NULL) return 0;
	if (iter->ptr == NULL) return 0;

	if (iter->length != 0) {
		if (iter->position == 0 && iter->next == 0) {
			iter->position 	= iter->length;
			iter->count 	= utf8_strnlen(iter->ptr, iter->length);
		}
	}

	const char* pointer;

	if (iter->position > 0) {

		iter->next = iter->position;
		iter->position--;

		if ((iter->ptr[iter->position] & 0x80) == 0) {
			iter->size = 1;
		}
		else {
			iter->size = 1;
			while ((iter->ptr[iter->position] & 0xC0) == 0x80 && iter->size < 6) {
				iter->position--;
				iter->size++;
			}
		}

		pointer = iter->ptr + iter->position;

		iter->codepoint = utf8_converter(pointer, iter->size);

		if (iter->codepoint == 0) return 0;

		iter->count--;

		return 1;
	}
	else {
		iter->next = 0;
		return 0;
	}
}

const char* utf8_getchar(utf8_iter* iter) {

	static char str[10];

	str[0] = '\0';

	if (iter == NULL)		return str;
	if (iter->ptr == NULL)	return str;
	if (iter->size == 0)	return str;

	if (iter->size == 1) {
		str[0] = iter->ptr[ iter->position ];
		str[1] = '\0';
		return str;
	}

	const char* pointer = iter->ptr + iter->position;

	for (uint8_t i = 0; i < iter->size; i++) {
		str[i] = pointer[i];
	}

	str[iter->size] = '\0';

	return str;
}

//Utilities

uint32_t utf8_strlen(const char* string) {

	if (string == NULL) return 0;

	uint32_t length		= 0;
	uint32_t position	= 0;

	while (string[position]) {
		position = position + utf8_charsize(string + position);
		length++;
	}

	return length;
}

uint32_t utf8_strnlen(const char* string, uint32_t end) {

	if (string == NULL) return 0;

	uint32_t length		= 0;
	uint32_t position	= 0;

	while (string[position] && position < end) {
		position = position + utf8_charsize(string + position);
		length++;
	}

	return length;
}

uint32_t utf8_to_unicode(const char* character) {

	if (character == NULL) return 0;
	if (character[0] == 0) return 0;

	uint8_t size = utf8_charsize(character);

	if (size == 0) return 0;

	return utf8_converter(character, size);
}

const char* unicode_to_utf8(uint32_t codepoint) {
	return unicode_converter(codepoint, unicode_charsize(codepoint));
}

//Internal use / Advanced use.

////// UTF8 to Unicode

uint8_t utf8_charsize(const char* character) {

	if (character == NULL) return 0;
	if (character[0] == 0) return 0;

	if ((character[0] & 0x80) == 0) return 1;
	else if ((character[0] & 0xE0) == 0xC0) return 2;
	else if ((character[0] & 0xF0) == 0xE0) return 3;
	else if ((character[0] & 0xF8) == 0xF0) return 4;
	else if ((character[0] & 0xFC) == 0xF8) return 5;
	else if ((character[0] & 0xFE) == 0xFC) return 6;

	return 0;
}

static const uint8_t table_unicode[] = {0, 0, 0x1F, 0xF, 0x7, 0x3, 0x1};

uint32_t utf8_converter(const char* character, uint8_t size) {

	if (size == 0) return 0;
	if (character == NULL) return 0;
	if (character[0] == 0) return 0;

	static uint32_t codepoint = 0;

	if (size == 1) {
		return character[0];
	}

	codepoint = table_unicode[size] & character[0];

	for (uint8_t i = 1; i < size; i++) {
		codepoint = codepoint << 6;
		codepoint = codepoint | (character[i] & 0x3F);
	}

	return codepoint;
}

////// Unicode to UTF8

uint8_t unicode_charsize(uint32_t codepoint) {

	if (codepoint == 0) return 0;

	if (codepoint < 0x80) return 1;
	else if (codepoint < 0x800) return 2;
	else if (codepoint < 0x10000) return 3;
	else if (codepoint < 0x200000) return 4;
	else if (codepoint < 0x4000000) return 5;
	else if (codepoint <= 0x7FFFFFFF) return 6;

	return 0;
}

static const uint8_t table_utf8[] = {0, 0,  0xC0,  0xE0, 0xF0,  0xF8,  0xFC};

const char* unicode_converter(uint32_t codepoint, uint8_t size) {

	static char str[10];

	str[size] = '\0';

	if (size == 0) return str;

	if (size == 1) {
		str[0] = codepoint;
		return str;
	}

	for (uint8_t i = size - 1; i > 0; i--) {
		str[i] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
	}

	str[0] = table_utf8[size] | codepoint;

	return str;
}
