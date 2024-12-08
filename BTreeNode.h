#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <deque>

typedef std::uint64_t Key;
typedef std::uint64_t RecordIndex;
typedef std::uint64_t PageN;

struct KeyStruct
{
	Key key;
	RecordIndex record_index;

	KeyStruct(Key k, RecordIndex r) : key(k), record_index(r) {};

	bool operator== (KeyStruct& other);
};

enum NodeType : uint64_t
{
	EMPTY = 0xffffffffffffffffu,
	ROOT = 0x746F6F72746F6F72u,
	NODE = 0x65646F6E65646F6Eu
};

class BTreeNode
{
public:
	BTreeNode();
	/*
	BTreeNode(const BTreeNode& btn);
	BTreeNode& operator=(const BTreeNode& other);
	*/
	~BTreeNode();

	void readPage(std::byte* buffer_page, size_t buffer_size);
	void writePage(std::byte* buffer_page, size_t buffer_size);

	int findKeyIndex(Key key);
	int insertKey(KeyStruct key_s);
	void replaceKey(KeyStruct key_s, int index);
	void insertChild(PageN child, int index);

	NodeType type;
	PageN page_number;
	PageN parent;
	std::deque<KeyStruct> keys;
	std::deque<PageN> children;
};