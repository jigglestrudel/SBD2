#include "RandomAccessFile.h"

RandomAccessFile::RandomAccessFile(const char* file_path, size_t block_size)
{
	bool buffer_exists = false;
	while (!buffer_exists){
		try
		{
			this->_buffer = new Buffer(block_size);
			buffer_exists = true;
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << "\n";
		}
	}
	this->_page_size = block_size;
	this->_file_path = nullptr;
	open(file_path);
	// calculating page count
	this->_allocated_page_count = (std::uint64_t)std::ceil(std::filesystem::file_size(file_path) / (double)this->_page_size);
	this->_loaded_page_number = UINT64_MAX;
	this->limit_drive_reads = true;
	this->buffer_changed = true;
	
}

RandomAccessFile::~RandomAccessFile()
{
	if (this->limit_drive_reads)
		this->offloadBlockToFile(this->_loaded_page_number);
	close();
	delete this->_buffer;
}

void RandomAccessFile::open(const char* file_path)
{
	std::ofstream ofstream(file_path, std::ofstream::binary | std::ofstream::app);
	ofstream.close();
	
	this->_file_stream = std::fstream(file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
	this->_file_path = new char[strlen(file_path) + 1];
	strcpy(this->_file_path, file_path);
}

int RandomAccessFile::getDriveReads()
{
	int dr = this->reads_count;
	this->reads_count = 0;
	return dr;
}
int RandomAccessFile::getDriveWrites()
{
	int dw = this->writes_count;
	this->writes_count = 0;
	return dw;
}

void RandomAccessFile::close()
{
	this->_file_stream.close();
}

void RandomAccessFile::loadBlockToBuffer(std::uint64_t page_number)
{
	if (this->limit_drive_reads && this->_allocated_page_count != 0 && this->_loaded_page_number == page_number)
	{
		//std::cout << "FILE: page " << page_number << " in " << this->_file_path << " already in buffer, no need to load\n";
		return;
	}
	
	// writing buffer to file before 
	if (this->limit_drive_reads && this->_loaded_page_number != UINT64_MAX && this->buffer_changed)
		this->offloadBlockToFile(_loaded_page_number);

	this->_file_stream.seekg(page_number * this->_page_size, this->_file_stream.beg);
	
	this->_file_stream.read((char*)(this->getBuffer()), this->_page_size);
	this->reads_count++;
	
	this->_buffer->resetCursor(0);
	//if read() didn't return full page_size, allocate the page in buffer;
	//std::cout << "read() read " << this->_file_stream.gcount() << " bytes\n";
	//std::cout << "file is " << this->_file_stream.is_open() << " open\n";
	if (this->_file_stream.gcount() != this->_page_size)
	{
		std::cout << "FILE: allocating new page " << page_number << " in " << this->_file_path << "\n";
		memset(this->_buffer->getBuffer(), UINT32_MAX, this->_page_size);
		this->_allocated_page_count++;
		this->_file_stream.clear();
	}
	this->_loaded_page_number = page_number;
	this->buffer_changed = false;

}

void RandomAccessFile::offloadBlockToFile(std::uint64_t page_number)
{
	if (page_number == UINT64_MAX)
		return;
	this->_file_stream.seekp(page_number * this->_page_size, this->_file_stream.beg);
	this->_file_stream.write((char*)(this->getBuffer()), this->_page_size);
	this->writes_count++;
	this->_buffer->resetCursor(0);
}

std::byte* RandomAccessFile::getBuffer()
{
	return this->_buffer->getBuffer();
}
