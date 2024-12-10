#pragma once

#include <stack>
#include <memory>

#include "RandomAccessFile.h"
#include "BTreeNode.h"


class BTreeFile : public RandomAccessFile
{
public:
	BTreeFile(const char* file_path, int d);
	~BTreeFile();

	void findEmptyPages();
	void findRoot();

	KeyStruct search(Key key);
	bool insert(KeyStruct key_s);
	KeyStruct remove(Key key);
	void offloadAll();
	void resetLoaded();

	void showTree();

	void showFile();

	void startKeyByKey();
	KeyStruct getNextKey();
	void stopKeyByKey();

	void cleanBack();

	bool shouldRebuild(double empty_limit);

	int d;

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

	
	std::shared_ptr<BTreeNode> root;
	std::vector<std::shared_ptr<BTreeNode>> loaded_nodes;
	std::deque<PageN> empty_pages;
	std::stack<int> key_by_key_cursors;

	bool _debug;
};