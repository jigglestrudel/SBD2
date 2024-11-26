#include "RecordFile.h"


RecordFile::RecordFile(int record_block_size) : RandomAccessFile(record_block_size * sizeof(Record))
{
	this->_record_block_size = record_block_size;
	delete this->_buffer;
	this->_buffer = new RecordBuffer(record_block_size);
}


RecordFile::~RecordFile()
{
}


Record RecordFile::getRecordFromFile(int block_number, int record_key)
{
	if (block_number <= _block_count)
		throw std::out_of_range("Not enough blocks in file");

	this->loadBlockToBuffer(block_number);

	for (int i = 0; i < this->_record_block_size; i++)
	{
		Record record_from_block = this->getRecordBuffer()[i];
		if (record_from_block.getKey() == record_key)
		{
			return record_from_block;
		}
	}

	throw std::invalid_argument("Record of this key not on the block");
}

void RecordFile::putRecordInFile(Record record)
{
	((RecordBuffer*)this->_buffer)->putNextRecord(record);

	if (((RecordBuffer*)this->_buffer)->howManyRecordsLeft() == 0)
	{
		this->offloadBlockToFile(_block_count++);
	}
}


Record* RecordFile::getRecordBuffer()
{
	return ((RecordBuffer*)this->_buffer)->getRecordBuffer();
}

