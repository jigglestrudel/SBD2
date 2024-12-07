#pragma once

#include <queue>

#include "RandomAccessFile.h";
#include "RecordBuffer.h"

class RecordFile : RandomAccessFile
{
public:
	RecordFile(const char* file_path, int record_page_size);
	~RecordFile();

	Record getRecordFromFile(unsigned int record_number);
	unsigned int putRecordInFile(Record record);
	void deleteRecordFromFile(unsigned int record_number);
	void findEmptyPlaces();

	Record* getRecordBuffer();

protected:
	RecordBuffer* getRecordBufferObject();

	std::queue<unsigned int> _empty_place_queue;
	int _record_page_size;

};