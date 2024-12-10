#pragma once

#include <queue>

#include "RandomAccessFile.h"
#include "RecordBuffer.h"

class RecordFile : public RandomAccessFile
{
public:
	RecordFile(const char* file_path, int record_page_size);
	~RecordFile();

	Record getRecordFromFile(unsigned int record_number);
	void replaceRecordInFile(unsigned int record_number, Record new_value);
	unsigned int putRecordInFile(Record record);
	void deleteRecordFromFile(unsigned int record_number);
	void findEmptyPlaces();

	Record* getRecordBuffer();

	void printFile();

	void cleanBack();
	bool shouldRebuild(double empty_limit);

	unsigned int getRecordCount();
	int _record_page_size;

protected:
	RecordBuffer* getRecordBufferObject();
	
	std::queue<unsigned int> _empty_place_queue;
	

};