#include "BTreeFile.h"

BTreeFile::BTreeFile(const char* file_path, int d) : RandomAccessFile(file_path, sizeof(uint64_t) + sizeof(PageN) * (2 * d + 2) + sizeof(KeyStruct) * 2 * d)
{
	this->d = d; 
	this->limit_drive_reads = false;
	this->loaded_nodes = std::vector<std::shared_ptr<BTreeNode>>();
	this->empty_pages = std::vector<PageN>();
}

BTreeFile::~BTreeFile()
{
	offloadLoadedNodes();
	offloadRoot();
}

void BTreeFile::findEmptyPages()
{
	for (PageN i = 1; i <= this->_allocated_page_count; i++)
	{
		this->loadBlockToBuffer(this->_allocated_page_count - i);
		if (this->getBuffer()[0] == std::byte(0xFF))
		{
			this->empty_pages.push_back(this->_allocated_page_count - i);
		}
	}
}

void BTreeFile::findRoot()
{
	if (this->_allocated_page_count == 0)
	{
		std::shared_ptr<BTreeNode> node = std::make_shared<BTreeNode>();
		this->loadBlockToBuffer(0);
		node->readPage(this->getBuffer(), this->_page_size);
		node->page_number = 0;
		node->is_full = true;
		this->root = node;
		return;
	}

	if (this->empty_pages.size() == this->_allocated_page_count)
	{
		PageN root_page = allocateNewNode();
		this->root = getNode(root_page);
		this->root->page_number = root_page;
		this->loaded_nodes.pop_back();
		return;
	}


	for (PageN i = 0; i < this->_allocated_page_count; i++)
	{
		std::shared_ptr<BTreeNode> node = std::make_shared<BTreeNode>();
		this->loadBlockToBuffer(i);
		node->readPage(this->getBuffer(), this->_page_size);
		if (node->is_full && node->parent == UINT64_MAX)
		{
			node->page_number = i;
			this->root = node;
			return;
		}

	}

}

std::uint64_t BTreeFile::allocateNewNode()
{
	/*std::shared_ptr<BTreeNode> new_node = std::make_shared<BTreeNode>();*/
	this->loaded_nodes.push_back(std::make_shared<BTreeNode>());
	
	if (!this->empty_pages.empty())
	{
		this->loaded_nodes.back()->page_number = this->empty_pages.back();
		this->empty_pages.pop_back();
		return this->loaded_nodes.back()->page_number;
	}

	this->loaded_nodes.back()->page_number = this->_allocated_page_count;
	this->loadBlockToBuffer(this->_allocated_page_count);
	return this->loaded_nodes.back()->page_number;
}

int BTreeFile::loadNode(PageN page_number)
{
	for (int i = 0; i < this->loaded_nodes.size(); i++)
	{
		if (this->loaded_nodes[i]->page_number == page_number)
			return i;
	}
	
	this->loadBlockToBuffer(page_number);
	std::shared_ptr<BTreeNode> loaded_node = std::make_shared<BTreeNode>();
	loaded_node->readPage(this->getBuffer(), this->_page_size);
	loaded_node->page_number = page_number;
	this->loaded_nodes.push_back(loaded_node);
	return this->loaded_nodes.size() - 1;
}

void BTreeFile::offloadNode(PageN page_number)
{
	if (page_number == this->root->page_number)
	{
		this->root->writePage(this->getBuffer(), this->_page_size);
		this->offloadBlockToFile(page_number);
		return;
	}

	int i = 0;
	while (i < this->loaded_nodes.size() && this->loaded_nodes[i]->page_number != page_number)
		i++;

	if (i == this->loaded_nodes.size())
		return;

	this->loaded_nodes[i]->writePage(this->getBuffer(), this->_page_size);
	this->offloadBlockToFile(this->loaded_nodes[i]->page_number);
	this->loaded_nodes.erase(this->loaded_nodes.begin() + i);
}

void BTreeFile::offloadLoadedNodes()
{
	for (auto node : this->loaded_nodes)
	{
		node->writePage(this->getBuffer(), this->_page_size);
		this->offloadBlockToFile(node->page_number);
	}
	this->loaded_nodes.clear();
	
}

void BTreeFile::offloadRoot()
{
	this->root->writePage(this->getBuffer(), this->_page_size);
	this->offloadBlockToFile(this->root->page_number);
}

std::shared_ptr<BTreeNode> BTreeFile::getNode(PageN page_number)
{
	if (page_number == UINT64_MAX)
	{
		std::cout << "GET NODE FAIL: something asked about null node\n";
		throw std::out_of_range("");
	}

	std::weak_ptr weak = this->root;

	if (!weak.expired() && page_number == this->root->page_number)
		return this->root;

	int index = loadNode(page_number);
	return this->loaded_nodes[index];
}

void BTreeFile::splitNode(PageN page_number)
{
	std::shared_ptr<BTreeNode> loaded_node = this->getNode(page_number);
	if (loaded_node->keys.size() != this->d * 2 + 1)
	{
		std::cout << "SPLIT ERROR: ??? this node isn't overflowing\n";
		return;
	}
	std::shared_ptr<BTreeNode> parent = this->getNode(loaded_node->parent);
	std::cout << "SPLIT: allocating node for new sibbling\n";
	PageN new_sibling_page = this->allocateNewNode();
	std::shared_ptr<BTreeNode> sibling = this->getNode(new_sibling_page);

	std::stack<KeyStruct> keys_to_new;
	std::stack<PageN> children_to_new;
	for (int i = 0; i < this->d + 1; i++)
	{
		keys_to_new.push(loaded_node->keys.back());
		loaded_node->keys.pop_back();
		if (loaded_node->children.size() > 0)
		{
			children_to_new.push(loaded_node->children.back());
			loaded_node->children.pop_back();
		}
	}
	KeyStruct median = keys_to_new.top();
	keys_to_new.pop();

	int index = parent->insertKey(median);
	parent->insertChild(new_sibling_page, index + 1);
	sibling->parent = parent->page_number;

	while (!keys_to_new.empty())
	{
		sibling->insertKey(keys_to_new.top());
		keys_to_new.pop();
	}
	while (!children_to_new.empty())
	{
		std::shared_ptr<BTreeNode> child = this->getNode(children_to_new.top());
		child->parent = sibling->page_number;
		sibling->children.push_back(child->page_number);
		children_to_new.pop();
	}
	
}

void BTreeFile::splitRoot()
{
	std::cout << "SPLIT: allocating new root node\n";
	PageN new_root_page = allocateNewNode();
	PageN old_root_page = this->root->page_number;
	std::shared_ptr<BTreeNode> new_root = this->getNode(new_root_page);

	this->root->parent = new_root_page;
	new_root->children.push_back(this->root->page_number);
	
	std::shared_ptr<BTreeNode> old_root = this->root;
	this->loaded_nodes.insert(this->loaded_nodes.begin(), old_root);
	
	this->root = new_root;
	this->loaded_nodes.pop_back();

	this->splitNode(old_root_page);
}

bool BTreeFile::compensation(PageN page_number)
{
	std::shared_ptr<BTreeNode> overflown_node = this->getNode(page_number);
	std::shared_ptr<BTreeNode> parent = this->getNode(overflown_node->parent);

	// find yourself in parents children
	// we can use a key from the overflown node to find its index in parent's children
	int index = parent->findKeyIndex(overflown_node->keys[0].key);

	// if we have a left sibling check if its overflown
	if (index > 0)
	{
		std::cout << "COMPENSATION: trying to compensate into the left sibbling\n";
		std::shared_ptr<BTreeNode> left = this->getNode(parent->children[index-1]);
		if (left->keys.size() < (2 * this->d))
		{
			// we can compensate to the left
			left->keys.push_back(parent->keys[index-1]);
			parent->replaceKey(overflown_node->keys.front(), index-1);
			overflown_node->keys.pop_front();

			if (overflown_node->children.size() > 0)
			{
				std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.front());
				moved_child->parent = left->page_number;
				left->children.push_back(moved_child->page_number);
				overflown_node->children.pop_front();
			}
			std::cout << "COMPENSATION: success\n";
			return true;
		}
		std::cout << "COMPENSATION: left sibbling full\n";
	}

	// if we have a right sibling check if its overflown
	if (index < parent->children.size()-1)
	{
		std::cout << "COMPENSATION: trying to compensate into the right sibbling\n";
		std::shared_ptr<BTreeNode> right = this->getNode(parent->children[index + 1]);
		if (right->keys.size() < (2 * this->d))
		{
			// we can compensate to the right
			right->keys.push_front(parent->keys[index]);
			parent->replaceKey(overflown_node->keys.back(), index);
			overflown_node->keys.pop_back();

			if (overflown_node->children.size() > 0)
			{
				std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.back());
				moved_child->parent = right->page_number;
				right->children.push_front(moved_child->page_number);
				overflown_node->children.pop_back();
			}
			std::cout << "COMPENSATION: success\n";
			return true;
		}
		std::cout << "COMPENSATION: right sibbling full\n";
	}
	std::cout << "COMPENSATION FAIL: sibblings full, couldn't compensate\n";
	return false;
}

void BTreeFile::printNodeReccurency(PageN page_number, int level)
{
	for (int i = 0; i < level; i++) std::cout << "\t";
	std::shared_ptr<BTreeNode> node_to_print = this->getNode(page_number);
	std::cout << "[.";
	for (KeyStruct key : node_to_print->keys)
	{
		std::cout << key.key << ".";
	}
	std::cout << "]\n";
	for (PageN child : node_to_print->children)
	{
		this->printNodeReccurency(child, level + 1);
	}
	offloadLoadedNodes();
}

KeyStruct BTreeFile::search(Key key)
{
	bool debug = false;
	if (debug) std::cout << "search: looking for " << key << "\n";
	std::shared_ptr<BTreeNode> node_to_search = this->root;
	
	while (true)
	{
		if (debug)
		{
			std::cout << "search: node keys -> ";
			for (auto key : node_to_search->keys)
				std::cout << key.key << " ";
			std::cout << "\n";
		}
		int index = node_to_search->findKeyIndex(key);
		// the node is empty
		if (index == -1)
		{
			if (debug) std::cout << "search: node empty\n";
			return { UINT64_MAX, UINT64_MAX };
		}

		if (index < node_to_search->keys.size() &&
			node_to_search->keys[index].key == key)
		{
			if (debug) std::cout << "search: found it at index " << index << "\n";
			return node_to_search->keys[index];
		}

		if (node_to_search->children.size() == 0)
		{
			if (debug) std::cout << "search: reached a leaf without the key\n";
			return { UINT64_MAX, UINT64_MAX };
		}

		if (debug)
		{
			std::cout << "search: going to child with keys: ";
			if (index == 0)
				std::cout << "<" << node_to_search->keys[0].key;
			else if (index == node_to_search->keys.size())
				std::cout << ">" << node_to_search->keys.back().key;
			else
				std::cout << node_to_search->keys[index - 1].key << " - " << node_to_search->keys[index].key;
			std::cout << "\n";
		}
		node_to_search = this->getNode(node_to_search->children[index]);
	}
}

bool BTreeFile::insert(KeyStruct key_s)
{
	bool was_added = false;

	KeyStruct search_result = this->search(key_s.key);
	if (search_result.key == key_s.key)
	{
		std::cout << "INSERT FAIL: key " << key_s.key << " already in the tree\n";
		offloadLoadedNodes();
		return false;
	}

	std::shared_ptr<BTreeNode> node_to_add_to = this->root;
	if (this->loaded_nodes.size() != 0)
		node_to_add_to = this->loaded_nodes.back();

	// dodawanie
	was_added = true;
	node_to_add_to->insertKey(key_s);
	std::cout << "INSERT: inserting key "<< key_s.key <<" to node "<< node_to_add_to->page_number <<"\n";
	while (node_to_add_to->keys.size() == 2 * this->d + 1)
	{
		if (node_to_add_to->page_number != this->root->page_number)
		{
			std::cout << "INSERT: node overflowing, trying compensation with siblings\n";
			if (this->compensation(node_to_add_to->page_number))
				break;
		}
		
		std::cout << "INSERT: node overflowing, splitting into two\n";
		if (this->root->page_number == node_to_add_to->page_number)
			this->splitRoot();
		else
			this->splitNode(node_to_add_to->page_number);
		
		
		std::cout << "INSERT: checking if parent overflowing\n";
		node_to_add_to = this->getNode(node_to_add_to->parent);
	}
	
	this->showTree();
	offloadLoadedNodes();
	return was_added;
}

void BTreeFile::showTree()
{
	this->printNodeReccurency(this->root->page_number, 0);
}

