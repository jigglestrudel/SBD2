#pragma once

#include "RandomAccessFile.h";

class RecordFile : RandomAccessFile
{
public:
	RecordFile(size_t block_size) : RandomAccessFile(block_size) {};
	~RecordFile();

	Record getRecordFromFile(int block_number, int record_key);
	void putRecordInFile(Record record);

private:

};