#include "RandomAccessFile.h"

RandomAccessFile::RandomAccessFile(size_t block_size)
{
	this->_record_buffer = Buffer(block_size);
	this->_block_size = block_size;
	this->_file_path = nullptr;
}

RandomAccessFile::~RandomAccessFile()
{

}

void RandomAccessFile::open(const char* file_path)
{
	this->_file_stream = std::fstream(file_path, std::ios_base::out | std::ios_base::in | std::ios_base::binary);
	this->_file_path = file_path;
}

void RandomAccessFile::close()
{
	this->_file_stream.close();
}

void RandomAccessFile::loadBlockToBuffer(int block_number)
{
	this->_file_stream.seekg(block_number * this->_block_size, std::ios_base::beg);
	this->_file_stream.read((char*)(this->getBuffer()), this->_block_size);
	this->_record_buffer.resetCursor();

}

std::byte* RandomAccessFile::getBuffer()
{
	return this->_record_buffer.getBuffer();
}
