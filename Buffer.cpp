#include "Buffer.h"

Buffer::Buffer()
{
	this->_buffer = nullptr;
	this->_size = 0;
	this->_cursor = 0;
}

Buffer::Buffer(size_t size)
{
	this->_buffer = new std::byte[size];
	this->_size = size;
	this->_cursor = 0;
}

Buffer::~Buffer()
{
	delete[] this->_buffer;
}

std::byte* Buffer::getBuffer()
{
	return this->_buffer;
}

size_t Buffer::getSize()
{
	return this->_size;
}

void Buffer::clear()
{
	memset(this->_buffer, 0xff, this->_size);
}

void Buffer::resetCursor(size_t cur)
{
	this->_cursor = cur;
}

void Buffer::incCursor(size_t inc)
{
	this->_cursor += inc;
	if (this->_cursor > this->_size)
	{
		throw std::out_of_range("Buffer cursor exceeded the size");
	}
}
