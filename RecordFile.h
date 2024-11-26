#pragma once

#include "RandomAccessFile.h";
#include "RecordBuffer.h"

class RecordFile : RandomAccessFile
{
public:
	RecordFile(int record_block_size);
	~RecordFile();

	Record getRecordFromFile(int block_number, int record_key);
	void putRecordInFile(Record record);

	Record* getRecordBuffer();

protected:
	int _record_block_size;
	int _block_count;

};