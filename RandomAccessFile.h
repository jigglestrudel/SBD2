#pragma once

#include <iostream>
#include <fstream>

#include "RecordBuffer.h"
#include "Record.h"


class RandomAccessFile
{
public:
	RandomAccessFile(size_t block_size);
	~RandomAccessFile();

	void open(const char* file_path);
	void close();

	void loadBlockToBuffer(int block_number);
	void offloadBlockToFile(int block_number);

	std::byte* getBuffer();

protected:
	std::fstream _file_stream;
	Buffer _record_buffer;
	const char* _file_path;
	size_t _block_size;
};