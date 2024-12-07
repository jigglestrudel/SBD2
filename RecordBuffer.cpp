#include "RecordBuffer.h"


RecordBuffer::~RecordBuffer()
{

}

Record RecordBuffer::getNextRecord()
{
	if (this->_size - this->_cursor < sizeof(Record))
	{
		throw std::out_of_range("Not enough bytes in buffer for a record");
	}

	Record output = *(Record*)(this->_buffer + this->_cursor);
	this->incCursor(sizeof(Record));

	return output;
}


int RecordBuffer::putRecordInFirstEmpty(Record record)
{
	int index = 0;
	while (index < this->_record_size &&
		this->getRecordAtIndex(index).getKey() != -1)
		index++;
	if (index < this->_record_size)
	{
		this->putRecordAtIndex(record, index);
		return index;
	}
	else
		return -1;
	
}

void RecordBuffer::putRecordAtIndex(Record record, int index)
{
	if (index >= this->_record_size)
	{
		throw std::out_of_range("outside the buffer");
	}

	this->getRecordBuffer()[index] = record;
}


Record* RecordBuffer::getRecordBuffer()
{
	return (Record*)(this->_buffer);
}

int RecordBuffer::howManyRecordsLeft()
{
	return this->_record_size - (this->_cursor / sizeof(Record));
}

Record RecordBuffer::getRecordAtIndex(int index)
{
	return ((Record*)(this->_buffer))[index];
}
