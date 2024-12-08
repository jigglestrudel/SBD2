#include "BTreeFile.h"

BTreeFile::BTreeFile(const char* file_path, int d) : RandomAccessFile(file_path, sizeof(NodeType) + sizeof(PageN) * (2 * d + 1) + sizeof(KeyStruct) * 2 * d)
{
	this->d = d; 
	this->limit_drive_reads = false;
	this->loaded_nodes = std::vector<std::shared_ptr<BTreeNode>>();
	this->empty_pages = std::vector<PageN>();
}

BTreeFile::~BTreeFile()
{
}

void BTreeFile::findEmptyPages()
{
	if (this->_allocated_page_count == 0)
		return;
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
		node->type = ROOT;
		this->root = node;
		this->offloadRoot();
		return;
	}

	if (this->empty_pages.size() == this->_allocated_page_count)
	{
		PageN root_page = allocateNewNode();
		this->root = getNode(root_page);
		this->root->type = ROOT;
		this->root->page_number = root_page;
		this->loaded_nodes.pop_back();
		return;
	}


	for (PageN i = 0; i < this->_allocated_page_count; i++)
	{
		std::shared_ptr<BTreeNode> node = std::make_shared<BTreeNode>();
		this->loadBlockToBuffer(i);
		node->readPage(this->getBuffer(), this->_page_size);
		if (node->type == ROOT)
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
	//this->offloadRoot();
	
}

void BTreeFile::offloadRoot()
{
	this->root->writePage(this->getBuffer(), this->_page_size);
	this->offloadBlockToFile(this->root->page_number);
}

void BTreeFile::offloadAll()
{
	this->offloadLoadedNodes();
	this->offloadRoot();
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

std::shared_ptr<BTreeNode> BTreeFile::grabTopLoaded()
{
	std::shared_ptr<BTreeNode> node;
	if (this->loaded_nodes.empty())
		node = this->root;
	else
		node = this->loaded_nodes.back();
	return node;
}

std::shared_ptr<BTreeNode> BTreeFile::grabSecondLoaded()
{
	std::shared_ptr<BTreeNode> node;
	if (this->loaded_nodes.empty())
		throw std::out_of_range("no nodes loaded");
	if (this->loaded_nodes.size() == 1)
		node = this->root;
	else
		node = this->loaded_nodes[this->loaded_nodes.size() - 2];
	return node;
}

void BTreeFile::offloadTopLoaded()
{
	if (this->loaded_nodes.empty())
		return;
	std::shared_ptr<BTreeNode> node = this->loaded_nodes.back();
	this->loaded_nodes.pop_back();
	node->writePage(this->getBuffer(), this->_page_size);
	this->offloadBlockToFile(node->page_number);
}

void BTreeFile::loadOnTop(PageN page_number)
{
	this->loadBlockToBuffer(page_number);
	std::shared_ptr<BTreeNode> loaded_node = std::make_shared<BTreeNode>();
	loaded_node->readPage(this->getBuffer(), this->_page_size);
	loaded_node->page_number = page_number;
	this->loaded_nodes.push_back(loaded_node);
}

void BTreeFile::splitNode()
{
	//std::shared_ptr<BTreeNode> loaded_node = this->getNode(page_number);
	std::shared_ptr<BTreeNode> loaded_node = this->grabTopLoaded();
	/*if (loaded_node->keys.size() != this->d * 2 + 1)
	{
		std::cout << "SPLIT ERROR: ??? this node isn't overflowing\n";
		return;
	}*/
	//std::shared_ptr<BTreeNode> parent = this->getNode(loaded_node->parent);
	std::shared_ptr<BTreeNode> parent = this->grabSecondLoaded();
	std::cout << "SPLIT: allocating node for new sibbling\n";
	PageN new_sibling_page = this->allocateNewNode();
	//std::shared_ptr<BTreeNode> sibling = this->getNode(new_sibling_page);
	std::shared_ptr<BTreeNode> sibling = this->grabTopLoaded();

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
	//sibling->parent = parent->page_number;

	while (!keys_to_new.empty())
	{
		sibling->insertKey(keys_to_new.top());
		keys_to_new.pop();
	}
	while (!children_to_new.empty())
	{
		//std::shared_ptr<BTreeNode> child = this->getNode(children_to_new.top());
		//child->parent = sibling->page_number;
		sibling->children.push_back(children_to_new.top());
		children_to_new.pop();
	}
	// offloading the sibbling
	this->offloadTopLoaded();
}

void BTreeFile::splitRoot()
{
	std::cout << "SPLIT: allocating new root node\n";
	PageN new_root_page = allocateNewNode();
	PageN old_root_page = this->root->page_number;
	std::shared_ptr<BTreeNode> new_root = this->grabTopLoaded();
	new_root->type = ROOT;
	this->root->type = NODE;

	//this->root->parent = new_root_page;
	new_root->children.push_back(this->root->page_number);
	
	std::shared_ptr<BTreeNode> old_root = this->root;
	this->root = new_root;
	this->loaded_nodes.pop_back();
	this->loaded_nodes.push_back(old_root);
	
	this->splitNode();
}

void BTreeFile::mergeNode()
{
	std::shared_ptr<BTreeNode> sibling = this->grabTopLoaded();
	std::shared_ptr<BTreeNode> underflown_node = this->grabSecondLoaded();
	bool is_right_sibling = underflown_node->keys.front().key < sibling->keys.front().key;
	
	std::cout << "MERGE: merging node " << underflown_node->page_number << " with node " << sibling->page_number << "\n";
	// copying the contents of sibling
	std::deque<KeyStruct> keys_from_sib = sibling->keys;
	std::deque<PageN> children_from_sib = sibling->children;

	// we can offload the sibling
	sibling->type = EMPTY;
	//this->empty_pages.push_back(sibling->page_number);
	this->offloadTopLoaded();

	std::shared_ptr<BTreeNode> parent = this->grabSecondLoaded();
	int index_in_parent = parent->findKeyIndex(underflown_node->keys.front().key);

	if (is_right_sibling)
	{
		KeyStruct median = parent->keys[index_in_parent];
		underflown_node->keys.push_back(median);
		parent->keys.erase(parent->keys.begin() + index_in_parent);
		parent->children.erase(parent->children.begin() + index_in_parent + 1);
		
		for (KeyStruct key : keys_from_sib)
			underflown_node->keys.push_back(key);

		for (PageN child : children_from_sib)
			underflown_node->children.push_back(child);
	}
	else
	{
		KeyStruct median = parent->keys[index_in_parent-1];
		underflown_node->keys.push_front(median);
		parent->keys.erase(parent->keys.begin() + index_in_parent - 1);
		parent->children.erase(parent->children.begin() + index_in_parent - 1);

		std::reverse(keys_from_sib.begin(), keys_from_sib.end());
		std::reverse(children_from_sib.begin(), children_from_sib.end());
		for (KeyStruct key : keys_from_sib)
			underflown_node->keys.push_front(key);

		for (PageN child : children_from_sib)
			underflown_node->children.push_front(child);
	}

	// check for root merging
	if (parent->type == ROOT && parent->keys.size() == 0 && parent->children.size() == 1)
	{
		underflown_node->type = ROOT;
		parent->type = EMPTY;
		this->loaded_nodes.pop_back();
		this->root = underflown_node;
		this->loaded_nodes.push_back(parent);
		this->offloadTopLoaded();
	}
	
}

bool BTreeFile::insertCompensation()
{
	//std::shared_ptr<BTreeNode> overflown_node = this->getNode(page_number);
	//std::shared_ptr<BTreeNode> parent = this->getNode(overflown_node->parent);
	std::shared_ptr<BTreeNode> overflown_node = this->grabTopLoaded();
	std::shared_ptr<BTreeNode> parent = this->grabSecondLoaded();

	// find yourself in parents children
	// we can use a key from the overflown node to find its index in parent's children
	int index = parent->findKeyIndex(overflown_node->keys[0].key);

	// if we have a left sibling check if its overflown
	if (index > 0)
	{
		std::cout << "COMPENSATION: trying to compensate into the left sibbling\n";
		//std::shared_ptr<BTreeNode> left = this->getNode(parent->children[index-1]);
		this->loadOnTop(parent->children[index - 1]);
		std::shared_ptr<BTreeNode> left = this->grabTopLoaded();
		if (left->keys.size() < (2 * this->d))
		{
			// we can compensate to the left
			left->keys.push_back(parent->keys[index - 1]);
			parent->replaceKey(overflown_node->keys.front(), index - 1);
			overflown_node->keys.pop_front();

			if (overflown_node->children.size() > 0)
			{
				//std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.front());
				//moved_child->parent = left->page_number;
				left->children.push_back(overflown_node->children.front());
				overflown_node->children.pop_front();
			}
			std::cout << "COMPENSATION: success\n";
			this->offloadTopLoaded();
			return true;
		}
		this->offloadTopLoaded();
		std::cout << "COMPENSATION: left sibbling full\n";
	}

	// if we have a right sibling check if its overflown
	if (index < parent->children.size() - 1)
	{
		std::cout << "COMPENSATION: trying to compensate into the right sibbling\n";
		//std::shared_ptr<BTreeNode> right = this->getNode(parent->children[index + 1]);
		this->loadOnTop(parent->children[index + 1]);
		std::shared_ptr<BTreeNode> right = this->grabTopLoaded();
		if (right->keys.size() < (2 * this->d))
		{
			// we can compensate to the right
			right->keys.push_front(parent->keys[index]);
			parent->replaceKey(overflown_node->keys.back(), index);
			overflown_node->keys.pop_back();

			if (overflown_node->children.size() > 0)
			{
				//std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.back());
				//moved_child->parent = right->page_number;
				right->children.push_front(overflown_node->children.back());
				overflown_node->children.pop_back();
			}
			this->offloadTopLoaded();
			std::cout << "COMPENSATION: success\n";
			return true;
		}
		this->offloadTopLoaded();
		std::cout << "COMPENSATION: right sibbling full\n";
	}
	std::cout << "COMPENSATION FAIL: sibblings full, couldn't compensate\n";
	return false;
}


bool BTreeFile::removeCompensation()
{
	std::shared_ptr<BTreeNode> underflown_node = this->grabTopLoaded();
	std::shared_ptr<BTreeNode> parent = this->grabSecondLoaded();

	int index = parent->findKeyIndex(underflown_node->keys[0].key);

	if (index > 0)
	{
		std::cout << "COMPENSATION: trying to compensate from the left sibbling\n";
		//std::shared_ptr<BTreeNode> left = this->getNode(parent->children[index-1]);
		this->loadOnTop(parent->children[index - 1]);
		std::shared_ptr<BTreeNode> left = this->grabTopLoaded();
		if (left->keys.size() > this->d)
		{
			// we can compensate to the left
			underflown_node->keys.push_front(parent->keys[index - 1]);
			parent->replaceKey(left->keys.back(), index - 1);
			left->keys.pop_back();

			if (left->children.size() > 0)
			{
				//std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.front());
				//moved_child->parent = left->page_number;
				underflown_node->children.push_front(left->children.back());
				left->children.pop_back();
			}
			std::cout << "COMPENSATION: success\n";
			this->offloadTopLoaded();
			return true;
		}
		//this->offloadTopLoaded();
		std::cout << "COMPENSATION: left sibbling on minimal keys\n";
	}

	if (index < parent->children.size() - 1)
	{
		// this means the left sibling is loaded
		// we can unload it now
		if (this->grabTopLoaded() != underflown_node)
		{
			this->offloadTopLoaded();
		}
		std::cout << "COMPENSATION: trying to compensate from the right sibbling\n";
		//std::shared_ptr<BTreeNode> left = this->getNode(parent->children[index-1]);
		this->loadOnTop(parent->children[index + 1]);
		std::shared_ptr<BTreeNode> right = this->grabTopLoaded();
		if (right->keys.size() > this->d)
		{
			// we can compensate to the left
			underflown_node->keys.push_back(parent->keys[index]);
			parent->replaceKey(right->keys.front(), index);
			right->keys.pop_front();

			if (underflown_node->children.size() > 0)
			{
				//std::shared_ptr<BTreeNode> moved_child = this->getNode(overflown_node->children.front());
				//moved_child->parent = left->page_number;
				underflown_node->children.push_back(right->children.front());
				right->children.pop_front();
			}
			std::cout << "COMPENSATION: success\n";
			this->offloadTopLoaded();
			return true;
		}
		//this->offloadTopLoaded();
		std::cout << "COMPENSATION: right sibbling on minimal keys\n";
	}

	return false;
}

void BTreeFile::printNodeReccurency(PageN page_number, int level)
{
	for (int i = 0; i < level; i++) std::cout << "\t";
	//std::shared_ptr<BTreeNode> node_to_print = this->getNode(page_number);
	//this->loadOnTop(page_number);
	std::shared_ptr<BTreeNode> node_to_print = this->grabTopLoaded();
	std::cout << "[.";
	for (KeyStruct key : node_to_print->keys)
	{
		std::cout << key.key << ".";
	}
	std::cout << "](" << node_to_print->page_number <<")\n";
	for (PageN child : node_to_print->children)
	{
		this->loadOnTop(child);
		this->printNodeReccurency(child, level + 1);
	}
	this->offloadTopLoaded();
}

KeyStruct BTreeFile::search(Key key)
{
	bool debug = false;
	if (debug) std::cout << "search: looking for " << key << "\n";
	//std::shared_ptr<BTreeNode> node_to_search = this->root;
	std::shared_ptr<BTreeNode> node_to_search = this->grabTopLoaded();
	
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
		this->loadOnTop(node_to_search->children[index]);
		node_to_search = this->grabTopLoaded();
		//node_to_search = this->getNode(node_to_search->children[index]);
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

	/*std::shared_ptr<BTreeNode> node_to_add_to = this->root;
	if (this->loaded_nodes.size() != 0)
		node_to_add_to = this->loaded_nodes.back();*/
	std::shared_ptr<BTreeNode> node_to_add_to = this->grabTopLoaded();

	// dodawanie
	node_to_add_to->insertKey(key_s);
	std::cout << "INSERT: inserting key "<< key_s.key <<" to node "<< node_to_add_to->page_number <<"\n";
	while (node_to_add_to->keys.size() == 2 * this->d + 1)
	{
		if (node_to_add_to->page_number != this->root->page_number)
		{
			std::cout << "INSERT: node overflowing, trying compensation with siblings\n";
			if (this->insertCompensation())
				break;
		}
		
		std::cout << "INSERT: node overflowing, splitting into two\n";
		if (this->root->page_number == node_to_add_to->page_number)
			this->splitRoot();
		else
			this->splitNode();
		
		
		std::cout << "INSERT: checking if parent overflowing\n";
		this->offloadTopLoaded();
		node_to_add_to = this->grabTopLoaded();
		//std::cout << "INSERT: parent(" << node_to_add_to->page_number <<") has "<< node_to_add_to->keys.size() <<" keys\n";
		//node_to_add_to = this->getNode(node_to_add_to->parent);
	}
	
	this->offloadLoadedNodes();
	//this->showTree();
	return true;
}

bool BTreeFile::remove(Key key)
{
	// search for the key (loading the required nodes)
	KeyStruct search_result = this->search(key);
	
	// if not found, return false
	if (search_result.key == -1)
	{
		std::cout << "REMOVE FAIL: key " << key << " not in the tree\n";
		this->offloadLoadedNodes();
		return false;
	}
	
	// if found on a leaf, remove
	std::shared_ptr<BTreeNode> node_to_remove_from = this->grabTopLoaded();
	int key_index_in_node = node_to_remove_from->findKeyIndex(key);

	std::cout << "REMOVE: removing key " << key << " from node " << node_to_remove_from->page_number << "\n";

	if (node_to_remove_from->children.empty())
	{
		std::cout << "REMOVE: removing key from leaf\n";
		node_to_remove_from->keys.erase(node_to_remove_from->keys.begin() + key_index_in_node);		
	}
	else
	{
		// if not proceed to the complex removal
		// find the predecesor node
		bool in_left_subtree = true;
		this->search(key - 1);
		std::shared_ptr<BTreeNode> node_with_replace_key = this->grabTopLoaded();
		// check if not minimal key number
		if (node_with_replace_key->keys.size() == this->d)
		{
			std::cout << "REMOVE: predecesor node on minimal keys, trying the successor node\n";
			// if yes, unload from memory and go to the successor
			while (!(this->grabTopLoaded() == node_to_remove_from))
				this->offloadTopLoaded();
			this->search(key + 1);
			node_with_replace_key = this->grabTopLoaded();
			in_left_subtree = false;
		}

		if (in_left_subtree)
		{
			std::cout << "REMOVE: replacing key with predecesor\n";
			KeyStruct replacement_key = node_with_replace_key->keys.back();
			node_with_replace_key->keys.pop_back();
			node_to_remove_from->replaceKey(replacement_key, key_index_in_node);
		}
		else
		{
			std::cout << "REMOVE: replacing key with successor\n";
			KeyStruct replacement_key = node_with_replace_key->keys.front();
			node_with_replace_key->keys.pop_front();
			node_to_remove_from->replaceKey(replacement_key, key_index_in_node);
		}
	}

	std::shared_ptr<BTreeNode> node_to_check = this->grabTopLoaded();
	while (node_to_check->keys.size() < this->d && node_to_check->type != ROOT)
	{
		std::cout << "REMOVE: node underflowing, trying to compensate with siblings\n";
		if (this->removeCompensation())
			break;
		std::cout << "REMOVE: node underflowing, merging with a sibling\n";
		this->mergeNode();

		if (node_to_check->type == ROOT)
		{
			std::cout << "REMOVE: node became new root\n";
			break;
		}

		std::cout << "REMOVE: checking if parent underflowing\n";
		this->offloadTopLoaded();
		node_to_check = this->grabTopLoaded();
	}
	this->offloadLoadedNodes();

	return true;
}

void BTreeFile::showTree()
{
	this->printNodeReccurency(this->root->page_number, 0);
	this->offloadLoadedNodes();
}

void BTreeFile::showFile()
{
	for (PageN i = 0; i < this->_allocated_page_count; i++)
	{
		int j;
		BTreeNode node = BTreeNode();
		this->loadBlockToBuffer(i);
		node.readPage(this->getBuffer(), this->_page_size);
		std::cout << "Page " << i << ": "; 
		switch (node.type)
		{
		case NodeType::NODE:
			std::cout << "NODE";
			break;
		case NodeType::ROOT:
			std::cout << "ROOT";
			break;
		case NodeType::EMPTY:
			std::cout << "EMPTY";
			break;
		default:
			break;
		}
		std::cout << " [";
		for (j = 0; j < node.keys.size(); j++)
		{
			if (!node.children.empty()) std::cout << node.children[j];
			else std::cout << ".";
			std::cout << " (" << node.keys[j].key << ", " << node.keys[j].record_index << ") ";
		}
		if (!node.children.empty()) std::cout << node.children[j];
		else std::cout << ".";
		std::cout << "]\n";
	}
}



