#include "BTreeNode.h"


bool KeyStruct::operator==(KeyStruct& other)
{
	return this->key == other.key;
}

BTreeNode::BTreeNode()
{
	this->page_number = UINT64_MAX;
	this->children = std::deque<PageN>();
	this->keys = std::deque<KeyStruct>();
	this->parent = UINT64_MAX;
	this->is_full = false;
}
//
//BTreeNode::BTreeNode(const BTreeNode& btn)
//{
//	this->page_number = btn.page_number;
//	this->parent = btn.parent;
//	this->children = std::deque<PageN>(btn.children.begin(), btn.children.end());
//	this->keys = std::deque<KeyStruct>(btn.keys.begin(), btn.keys.end());
//}
//
//BTreeNode& BTreeNode::operator=(const BTreeNode& other)
//{
//	// Check for self-assignment
//	if (this == &other)
//		return *this;
//
//	// Copy primitive types
//	this->page_number = other.page_number;
//	this->parent = other.parent;
//
//	// Copy complex types (containers)
//	this->children = other.children;
//	this->keys = other.keys;
//
//	// Return *this to allow chaining
//	return *this;
//}


BTreeNode::~BTreeNode()
{
	//std::cout << "deleting node " << this->page_number << "!\n";
}

void BTreeNode::readPage(std::byte* buffer_page, size_t buffer_size)
{
	if (buffer_page[0] == std::byte{ 0xFF })
	{
		this->is_full = false;
		return;
	}
	this->is_full = true;
	size_t index = sizeof(uint64_t);
	this->parent = *(PageN*)(buffer_page + index);
	index += sizeof(PageN);
	if (*(PageN*)(buffer_page + index) != UINT64_MAX)
		this->children.push_back(*(PageN*)(buffer_page + index));
	index += sizeof(PageN);
	while (index < buffer_size && ((KeyStruct*)(buffer_page + index))->key != UINT64_MAX)
	{
		this->keys.push_back(*(KeyStruct*)(buffer_page + index));
		index += sizeof(KeyStruct);
		if (*(PageN*)(buffer_page + index) != UINT64_MAX)
			this->children.push_back(*(PageN*)(buffer_page + index));
		index += sizeof(PageN);
	}
}

void BTreeNode::writePage(std::byte* buffer_page, size_t buffer_size)
{
	memset(buffer_page, 0xff, buffer_size);
	*(uint64_t*)(buffer_page) = 0xEFCDAB8967452301ui64;
	size_t index = sizeof(uint64_t);
	*(PageN*)(buffer_page + index) = this->parent;
	index += sizeof(PageN);
	
	while (!this->keys.empty() || !this->children.empty())
	{
		if (!this->children.empty())
		{
			*(PageN*)(buffer_page + index) = this->children.front();
			this->children.pop_front();
		}
		index += sizeof(PageN);
		if (!this->keys.empty())
		{
			*(KeyStruct*)(buffer_page + index) = this->keys.front();
			this->keys.pop_front();
		}
		index += sizeof(KeyStruct);
		//if (index > buffer_size)
			//throw std::out_of_range("NODE EXCEEDING BUFFER");
	}
}

int BTreeNode::findKeyIndex(Key key)
{
	//std::find(this->_keys.begin(), this->_keys.end(), KeyStruct(key, 0));

	std::uint64_t length = this->keys.size();
	std::uint64_t a = 0;
	std::uint64_t b = length;
	std::uint64_t index = length / 2;

	if (length == 0)
		return -1;
	
	do 
	{
		if (this->keys[index].key == key)
			return index;

		if (this->keys[index].key > key)
		{
			if (index > 0 &&
				this->keys[index - 1].key < key)
				return index;

			b = index;
		}
		else
		{
			if (index < length-1 &&
				this->keys[index + 1].key > key)
				return index + 1;

			a = index;
		}

		index = (a + b) / 2;
	} while (this->keys[index].key != key &&
		index != 0 && index != length - 1);

	if (index == 0 || index == length - 1)
	{
		if (this->keys[index].key >= key)
			return index;
		else
			return index +1;
	}
	else if (this->keys[index].key == key)
		return index;
	
	return 0;
}

int BTreeNode::insertKey(KeyStruct key_s)
{
	if (this->keys.size() == 0)
	{
		this->keys.push_back(key_s);
		return 0;
	}
	int index = this->findKeyIndex(key_s.key);
	this->keys.insert(this->keys.begin() + index, key_s);
	return index;
}

void BTreeNode::replaceKey(KeyStruct key_s, int index)
{
	if (index >= this->keys.size())
		return;
	this->keys[index] = key_s;
}

void BTreeNode::insertChild(PageN child, int index)
{
	this->children.insert(this->children.begin() + index, child);
}
