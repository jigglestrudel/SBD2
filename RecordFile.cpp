#include "RecordFile.h"


RecordFile::RecordFile(const char* file_path,int record_block_size) : RandomAccessFile(file_path, record_block_size * sizeof(Record))
{
	this->_record_page_size = record_block_size;
	delete this->_buffer;
	this->_buffer = new RecordBuffer(record_block_size);
	this->_empty_place_queue = std::queue<unsigned int>();
}


RecordFile::~RecordFile()
{

}

/*
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
*/

Record RecordFile::getRecordFromFile(unsigned int record_number)
{
	unsigned int page_number = record_number / this->_record_page_size;
	unsigned int record_index = record_number % this->_record_page_size;

	this->loadBlockToBuffer(page_number);
	Record* record_buffer = this->getRecordBuffer();
	return record_buffer[record_index];
}

unsigned int RecordFile::putRecordInFile(Record record)
{
	unsigned int chosen_place;
	// check if empty spots
	if (!this->_empty_place_queue.empty())
	{
		chosen_place = this->_empty_place_queue.front();
		this->_empty_place_queue.pop();
		this->loadBlockToBuffer(chosen_place / this->_record_page_size);
		this->getRecordBuffer()[chosen_place % this->_record_page_size] = record;
		return chosen_place;
	}
	else
	{
		if (this->_allocated_page_count != 0)
			chosen_place = (this->_allocated_page_count-1) * this->_record_page_size;
		else
			chosen_place = 0;
		// if not go to last block
		this->loadBlockToBuffer(chosen_place / this->_record_page_size);
		// find first empty spot
		int index = this->getRecordBufferObject()->putRecordInFirstEmpty(record);
		// put the record there
		if (index == -1)
		{
			this->loadBlockToBuffer(this->_allocated_page_count);
			this->getRecordBufferObject()->putRecordAtIndex(record, 0);
			return (this->_allocated_page_count - 1) * this->_record_page_size;
		}
		return chosen_place + index;
	}
	
}

void RecordFile::deleteRecordFromFile(unsigned int record_number)
{
	unsigned int page_number = record_number / this->_record_page_size;
	unsigned int record_index = record_number % this->_record_page_size;

	this->loadBlockToBuffer(page_number);
	Record* record_buffer = this->getRecordBuffer();
	this->_empty_place_queue.push(record_number);
	record_buffer[record_index] = Record(-1, {0,0,0});
}

void RecordFile::findEmptyPlaces()
{
	for (int i = 0; i < this->_allocated_page_count; i++)
	{
		this->loadBlockToBuffer(i);
		for (int j = 0; j < this->_record_page_size; j++)
		{
			if (this->getRecordBufferObject()->getRecordAtIndex(j).getKey() == -1)
				this->_empty_place_queue.push(i * this->_record_page_size + j);
		}
	}
}

/*
void RecordFile::putRecordInFile(Record record)
{
	((RecordBuffer*)this->_buffer)->putNextRecord(record);

	if (((RecordBuffer*)this->_buffer)->howManyRecordsLeft() == 0)
	{
		this->offloadBlockToFile(_block_count++);
	}
}
*/

RecordBuffer* RecordFile::getRecordBufferObject()
{
	return ((RecordBuffer*)this->_buffer);
}


Record* RecordFile::getRecordBuffer()
{
	return ((RecordBuffer*)this->_buffer)->getRecordBuffer();
}

