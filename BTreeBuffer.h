//#pragma once
//
//#include "Buffer.h"
//
//struct KeyPointer
//{
//	std::uint64_t key;
//	std::uint64_t record_number;
//
//	
//};
//
//struct ChildPointer
//{
//	std::uint64_t child_page;
//};
//
//enum BTreeNodeType : uint64_t
//{
//	ROOT,
//	INTERNAL,
//	LEAF,
//	EMPTY = UINT64_MAX
//};
//
///*
//	B-Tree node structure
//
//	std::uint64_t	TYPE [EMPTY, ROOT, INTERNAL, LEAF]
//	std::uint64_t	PARENT_PAGE
//	std::uint64_t	LENGTH
//
//	std::uint64_t
//	[2 * D]			KEY_TABLE
//
//	std::uint64_t
//	[2 * D + 1]		CHILDREN_TABLE
//*/
//
//
//class BTreeBuffer : Buffer {
//
//public:
//	BTreeBuffer(int d);
//	~BTreeBuffer();
//
//
//	void replaceKey(std::uint64_t index, KeyPointer key);
//	KeyPointer getKeyAtIndex(std::uint64_t index);
//
//	std::uint64_t insertKey(KeyPointer key);
//	void insertChild(ChildPointer child, std::uint64_t index);
//
//	std::uint64_t findIndexOfKey(KeyPointer key);
//
//	KeyPointer popLowestKey();
//	ChildPointer popLowestChild();
//	KeyPointer popHighestKey();
//	ChildPointer popHighestChild();
//
//protected:
//	int _d;
//	BTreeNodeType*	_node_type;
//	std::uint64_t*	_parent_page;
//	std::uint64_t*	_node_length;
//	KeyPointer*		_key_table;
//	ChildPointer*	_children_table;
//};