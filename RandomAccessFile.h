#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>

#include "RecordBuffer.h"
#include "Record.h"


class RandomAccessFile
{
public:
	RandomAccessFile(const char* file_path, size_t block_size);
	~RandomAccessFile();

	void loadBlockToBuffer(std::uint64_t _loaded_page_number);
	void offloadBlockToFile(std::uint64_t _loaded_page_number);

	std::byte* getBuffer();

protected:
	void open(const char* file_path);
	void close();

	std::fstream _file_stream;
	Buffer* _buffer;
	const char* _file_path;
	size_t _page_size;
	std::uint64_t _loaded_page_number;
	std::uint64_t _allocated_page_count;
	bool limit_drive_reads;
};