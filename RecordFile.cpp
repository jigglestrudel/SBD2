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

Record RecordFile::getRecordFromFile(unsigned int record_number)
{
	unsigned int page_number = record_number / this->_record_page_size;
	unsigned int record_index = record_number % this->_record_page_size;

	this->loadBlockToBuffer(page_number);
	Record* record_buffer = this->getRecordBuffer();
	return record_buffer[record_index];
}

void RecordFile::replaceRecordInFile(unsigned int record_number, Record new_value)
{
	unsigned int page_number = record_number / this->_record_page_size;
	unsigned int record_index = record_number % this->_record_page_size;

	this->loadBlockToBuffer(page_number);
	Record* record_buffer = this->getRecordBuffer();
	record_buffer[record_index] = new_value;
	this->buffer_changed = true;
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
		this->buffer_changed = true;
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
		this->buffer_changed = true;
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
	this->buffer_changed = true;
}

void RecordFile::findEmptyPlaces()
{
	for (int i = this->_empty_place_queue.size(); i > 0; i--)
		this->_empty_place_queue.pop();
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

//void RecordFile::printFile()
//{
//	for (int i = 0; i < this->_allocated_page_count; i++)
//	{
//		std::cout << "Page " << i << " ============\n";
//		for (int j = 0; j < this->_record_page_size; j++)
//		{
//			Record record = this->getRecordFromFile(i * this->_record_page_size + j);
//			if (record.getKey() != UINT64_MAX)
//				std::cout << record << "\n";
//	3		else
//				std::cout << "EMPTY\n";
//		}
//	}
//}
void RecordFile::printFile()
{
	this->offloadBlockToFile(this->_loaded_page_number);
	for (int i = 0; i < this->_allocated_page_count; i++)
	{
		std::cout << "Page " << i << " ============\n";
		this->loadBlockToBuffer(i);
		for (int j = 0; j < this->_record_page_size; j++)
		{
			Record record = this->getRecordBufferObject()->getRecordAtIndex(j);
			if (record.getKey() != UINT64_MAX)
				std::cout << record << "\n";
			else
				std::cout << "EMPTY\n";
		}
	}
}

void RecordFile::cleanBack()
{
	int empty_back_count = 0;
	for (int i = this->_allocated_page_count - 1; i >= 0; i--)
	{
		this->loadBlockToBuffer(i);
		bool page_empty = true;
		for (int j = 0; j < this->_record_page_size; j++)
		{
			if (this->getRecordBufferObject()->getRecordAtIndex(j).getKey() != -1)
			{
				page_empty = false;
				break;
			}
		}
		if (page_empty)
			empty_back_count++;
		else
			break;
	}
	this->close();
	std::filesystem::resize_file(this->_file_path, (_allocated_page_count - empty_back_count) * this->_page_size);
	this->_allocated_page_count -= empty_back_count;
	this->open(this->_file_path);
}

bool RecordFile::shouldRebuild(double empty_limit)
{
	return (this->_empty_place_queue.size() / ((double)this->_allocated_page_count * this->_record_page_size)) > empty_limit;;
}

unsigned int RecordFile::getRecordCount()
{
	return this->_allocated_page_count* this->_record_page_size;
}

