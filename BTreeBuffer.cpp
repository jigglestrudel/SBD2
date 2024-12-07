//#include "BTreeBuffer.h"
//
//BTreeBuffer::BTreeBuffer(int d)
//	: Buffer(0)
//{
//	this->_d = d;
//	// see BTreeBuffer.h
//	size_t size = 0;
//	size += sizeof(BTreeNodeType);
//	size += sizeof(uint64_t);
//	size += sizeof(uint64_t);
//	size += sizeof(KeyPointer) * (2 * d);
//	size += sizeof(ChildPointer) * (2 * d + 1);
//	
//	this->_node_type = (BTreeNodeType*)(this->_buffer);
//	this->_parent_page = (std::uint64_t*)(this->_buffer) + 1;
//	this->_node_length = (std::uint64_t*)(this->_buffer) + 2;
//	this->_key_table = (KeyPointer*)(this->_buffer + (sizeof(BTreeNodeType) + 2 * sizeof(uint64_t)));
//	this->_children_table = (ChildPointer*)(this->_key_table + (2 * d));
//
//}
//
//void BTreeBuffer::replaceKey(std::uint64_t index, KeyPointer key)
//{
//	_key_table[index] = key;
//}
//
//KeyPointer BTreeBuffer::getKeyAtIndex(std::uint64_t index)
//{
//	return _key_table[index];
//}
//
//std::uint64_t BTreeBuffer::insertKey(KeyPointer key)
//{
//	if (*(this->_node_length) >= this->_d * 2)
//	{
//		throw std::overflow_error("node full");
//	}
//
//	std::uint64_t index = *this->_node_length;
//	while (index != 0 && this->_key_table[index-1].key > key.key)
//	{
//		this->_key_table[index] = this->_key_table[index - 1];
//		index--;
//	}
//	this->_key_table[index] = key;
//	*(this->_node_length)++;
//	return index;
//
//}
//
//void BTreeBuffer::insertChild(ChildPointer child, std::uint64_t desired_index)
//{
//	if (this->_children_table[this->_d * 2].child_page != UINT64_MAX)
//	{
//		throw std::overflow_error("node full");
//	}
//
//	std::uint64_t index = *this->_node_length + 1;
//	while (index != desired_index )
//	{
//		this->_children_table[index] = this->_children_table[index - 1];
//		index--;
//	}
//	this->_children_table[index] = child;
//}
//
//std::uint64_t BTreeBuffer::findIndexOfKey(KeyPointer key)
//{
//	// bisexion
//	std::uint64_t length = *(this->_node_length);
//	std::uint64_t a = 0;
//	std::uint64_t b = length;
//	std::uint64_t index = length / 2;
//	
//	// search until found or on the edge 
//	while (this->_key_table[index].key != key.key &&
//		index != 0 && index != length - 1)
//	{
//		if (this->_key_table[index].key > key.key)
//		{
//			if (this->_key_table[index - 1].key < key.key)
//				return index;
//
//			b = index;
//		}
//		else
//		{
//			if (this->_key_table[index + 1].key > key.key)
//				return index + 1;
//
//			a = index;
//		}
//
//		index = (a + b) / 2;
//	}
//
//	if (this->_key_table[index].key == key.key ||
//		index == 0)
//		return index;
//	else
//		return index + 1;
//}
//
//KeyPointer BTreeBuffer::popLowestKey()
//{
//	std::uint64_t index = 0;
//	KeyPointer lowest_key = this->_key_table[0];
//	*this->_node_length--;
//	while (index != *this->_node_length)
//	{
//		this->_key_table[index] = this->_key_table[index + 1];
//		index++;
//	}
//
//	return lowest_key;
//}
//
//ChildPointer BTreeBuffer::popLowestChild()
//{
//	std::uint64_t index = 0;
//	ChildPointer lowest_child = this->_children_table[0];
//	while (index != *this->_node_length + 1)
//	{
//		this->_children_table[index] = this->_children_table[index + 1];
//		index++;
//	}
//
//	return lowest_child;
//}
//
//KeyPointer BTreeBuffer::popHighestKey()
//{
//	KeyPointer highest_key = this->_key_table[*this->_node_length - 1];
//	this->_key_table[*this->_node_length - 1] = { UINT64_MAX, UINT64_MAX };
//	*this->_node_length--;
//	return highest_key;
//}
//
//ChildPointer BTreeBuffer::popHighestChild()
//{
//	ChildPointer highest_child = this->_children_table[*this->_node_length];
//	this->_children_table[*this->_node_length] = { UINT64_MAX };
//	return highest_child;
//}
//
//
