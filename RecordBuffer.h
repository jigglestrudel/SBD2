#pragma once
#include "Record.h"
#include "Buffer.h"
#include <stdexcept>

class RecordBuffer : public Buffer {
public:
	RecordBuffer(size_t size) : Buffer(size) {};
	~RecordBuffer();

	Record getNextRecord();
	void putNextRecord(Record record);

protected:
};
