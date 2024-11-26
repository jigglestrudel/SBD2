#pragma once

#include <stdexcept>
#include <cstddef>

class Buffer {
public:
	Buffer();
	Buffer(size_t size);
	virtual ~Buffer();

	virtual std::byte* getBuffer();
	size_t getSize();

	void resetCursor(size_t cur, size_t bytes_read);


protected:
	void incCursor(size_t inc);

	std::byte* _buffer;
	size_t _size;
	size_t _cursor;
	size_t _bytes_read;
};