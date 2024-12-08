#pragma once

#include <stack>
#include <memory>

#include "RandomAccessFile.h"
#include "BTreeNode.h"


class BTreeFile : RandomAccessFile
{
public:
	BTreeFile(const char* file_path, int d);
	~BTreeFile();

	void findEmptyPages();
	void findRoot();

	KeyStruct search(Key key);
	bool insert(KeyStruct key_s);
	bool remove(Key key);
	void offloadAll();

	void showTree();

	void showFile();

protected:
	std::uint64_t allocateNewNode();

	int loadNode(PageN page_number);
	void offloadNode(PageN page_number);

	void offloadLoadedNodes();
	void offloadRoot();
	

	std::shared_ptr<BTreeNode> getNode(PageN page_number);
	std::shared_ptr<BTreeNode> grabTopLoaded();
	std::shared_ptr<BTreeNode> grabSecondLoaded();
	void offloadTopLoaded();
	void loadOnTop(PageN page_number);

	void splitNode();
	void splitRoot();

	void mergeNode();

	bool insertCompensation();

	bool removeCompensation();

	void printNodeReccurency(PageN page_number, int level);

	int d;
	std::shared_ptr<BTreeNode> root;
	std::vector<std::shared_ptr<BTreeNode>> loaded_nodes;
	std::vector<PageN> empty_pages;
};