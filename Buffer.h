#pragma once

#include <stdexcept>
#include <cstddef>

class Buffer {
public:
	Buffer();
	Buffer(size_t size);
	~Buffer();

	std::byte* getBuffer();
	size_t getSize();

	void resetCursor();

protected:
	void incCursor(size_t inc);

	std::byte* _buffer;
	size_t _size;
	size_t _cursor;
};