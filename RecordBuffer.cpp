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

void RecordBuffer::putNextRecord(Record record)
{
	if (this->_size - this->_cursor < sizeof(Record))
	{
		throw std::out_of_range("Not enough bytes in buffer to fit a record");
	}

	Record* record_place = (Record*)(this->_buffer + this->_cursor);
	*record_place = record;

	this->incCursor(sizeof(Record));
}
