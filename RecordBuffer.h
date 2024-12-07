#pragma once
#include "Record.h"
#include "Buffer.h"
#include <stdexcept>

class RecordBuffer : public Buffer {
public:
	RecordBuffer(int record_size) : Buffer(record_size * sizeof(Record)), _record_size(record_size) {};
	~RecordBuffer();

	Record getNextRecord();
	int putRecordInFirstEmpty(Record record);
	void putRecordAtIndex(Record record, int index);
	Record* getRecordBuffer();
	Record getRecordAtIndex(int index);
	int howManyRecordsLeft();

protected:
	int _record_size;
};
