#include "Database.h"

Database::Database()
{
    this->is_open = false;
    this->index_file = nullptr;
    this->main_file = nullptr;
    this->name = "";
    srand(time(NULL));
}

Database::~Database()
{
    if (this->index_file != nullptr)
        delete this->index_file;
    if (this->main_file != nullptr)
        delete this->main_file;
}

void Database::start()
{
    std::cout << "== trapezoid database ==\n";
    std::cout << "INFO: looking for collections\n";
    if (std::filesystem::exists(COLLECTIONS_DIR))
    {
        for (const auto& entry : std::filesystem::directory_iterator(COLLECTIONS_DIR))
        {
            if (entry.is_directory())
                std::cout << "Found collection: " << entry.path().filename() << "\n";
        }
    }
    else
    {
        std::cout << "INFO: creating collections directory\n";
        std::filesystem::create_directory(COLLECTIONS_DIR);
    }
    std::string input;
    while (input != "exit")
    {
        std::cout << "Choose mode of operations [file, tui] or quit [exit]\n>";
        std::cin.clear();
        std::cin >> input;
        

        if (input == "tui")
            this->tui();
        else if (input == "file")
            this->readInstructionsFile();
        else if (input == "exit")
            break;
        else
            std::cout << "unknown mode\n";
        std::cin.ignore(INT_MAX, '\n');
    }

}

void Database::openCollection(std::string name)
{
    if (this->is_open)
    {
        std::cout << "FAIL: close currently open collection\n";
        return;
    }

    std::string collection_directory = COLLECTIONS_DIR + std::string("/") + name;
    if (!std::filesystem::exists(collection_directory))
    {
        std::cout << "FAIL: collection doesn't exist\n";
        return;
    }

    bool bad_flag = false;
    if (!std::filesystem::exists(collection_directory + "/" + name + ".conf"))
    {
        bad_flag = true;
        std::cout << "FAIL: collection configuration file missing\n";
    }
    if (!std::filesystem::exists(collection_directory + "/" + name + ".main"))
    {
        bad_flag = true;
        std::cout << "FAIL: collection main file missing\n";
    }
    if (!std::filesystem::exists(collection_directory + "/" + name + ".tree"))
    {
        bad_flag = true;
        std::cout << "FAIL: collection index file missing\n";
    }

    if (bad_flag)
    {
        std::cout << "FAIL: collection not complete, couldn't open\n";
        return;
    }

    std::ifstream config_file = std::ifstream(std::string(collection_directory + "/" + name + ".conf"));
    int d, b;
    config_file >> d;
    config_file >> b;
    config_file.close();

    if (d <= 0)
    {
        std::cout << "FAIL: config file contains invalid properties\n";
        return;
    }

    std::cout << "INFO: opening collection " << name << "\n";
    this->main_file = new RecordFile(std::string(collection_directory + "/" + name + ".main").c_str(), b);
    this->main_file->findEmptyPlaces();
    this->index_file = new BTreeFile(std::string(collection_directory + "/" + name + ".tree").c_str(), d);
    this->index_file->findEmptyPages();
    this->index_file->findRoot();
    this->is_open = true;
    this->name = name;
}

bool Database::createCollection(std::string name, int d=DEFAULT_D, int b=DEFAULT_B)
{
    if (this->is_open)
    {
        std::cout << "FAIL: close currently open collection\n";
        return false;
    }
    std::string collection_directory = COLLECTIONS_DIR + std::string("/") + name;
    if (std::filesystem::exists(collection_directory))
    {
        std::cout << "FAIL: collection exists already\n";
        return false;
    }

    std::cout << "INFO: creating collection\n";
    std::filesystem::create_directory(collection_directory);
    std::cout << "INFO: creating main file\n";
    std::ofstream main_file(collection_directory + "/" + name + ".main");
    main_file.close();
    std::cout << "INFO: creating index file\n";
    std::ofstream index_file(collection_directory + "/" + name + ".tree");
    index_file.close();
    std::cout << "INFO: creating config file\n";
    std::ofstream config_file_creation(collection_directory + "/" + name + ".conf");
    config_file_creation.close();
    std::fstream config_file(collection_directory + "/" + name + ".conf");
    config_file << d;
    config_file << "\n";
    config_file << b;
    config_file.close();
    return true;
}

void Database::deleteCollection(std::string name)
{
    if (this->is_open)
    {
        std::cout << "FAIL: close currently open collection\n";
        return;
    }
    std::string collection_directory = COLLECTIONS_DIR + std::string("/") + name;
    if (!std::filesystem::exists(collection_directory))
    {
        std::cout << "FAIL: collection doesn't exist\n";
        return;
    }
    std::cout << "INFO: removing collection\n";
    std::filesystem::remove_all(collection_directory);
}

void Database::close()
{
    std::string collection_directory = COLLECTIONS_DIR + std::string("/") + name;
    if (!this->is_open)
    {
        std::cout << "FAIL: no collection open\n";
        return;
    }
    std::cout << "INFO: closing collection " << this->name << "\n";
    index_file->offloadAll();

    // performing cleaning
    this->index_file->cleanBack();
    this->main_file->cleanBack();
    this->main_file->findEmptyPlaces();
    this->index_file->findEmptyPages();
    bool should_reorganise = false;
    if (this->main_file->shouldRebuild(EMPTY_LIMIT))
    {
        should_reorganise = true;
        std::cout << "INFO: main file empty places over the limit\n";
    }
    if (this->index_file->shouldRebuild(EMPTY_LIMIT))
    {
        should_reorganise = true;
        std::cout << "INFO: main file empty places over the limit\n";
    }
    if (should_reorganise)
    {
        std::cout << "INFO: rebuilding the tree\n";
        int d = this->index_file->d;
        delete this->index_file;
        RecordFile* old_main = this->main_file;
        this->main_file = new RecordFile(std::string(collection_directory + "/" + name + ".maincopy").c_str(), this->main_file->_record_page_size);
        this->index_file = new BTreeFile(std::string(collection_directory + "/" + name + ".treecopy").c_str(), d);
        this->index_file->findRoot();

        for (int i = 0; i < old_main->getRecordCount(); i++)
        {
            Record record = old_main->getRecordFromFile(i);
            if (record.getKey() != UINT64_MAX)
            {
                this->insert(record);
            }
        }

        delete old_main;
        index_file->offloadAll();
    }



    delete this->index_file;
    delete this->main_file;
    this->index_file = nullptr;
    this->main_file = nullptr;
    this->is_open = false;
    if (should_reorganise)
    {
        std::filesystem::remove(collection_directory + "/" + name + ".main");
        std::filesystem::remove(collection_directory + "/" + name + ".tree");
        std::filesystem::copy_file(collection_directory + "/" + name + ".treecopy", collection_directory + "/" + name + ".tree");
        std::filesystem::copy_file(collection_directory + "/" + name + ".maincopy", collection_directory + "/" + name + ".main");
    }
    this->name = "";
}

void Database::readInstructionsFile()
{
    
    std::cout << "Enter file name: ";
    std::string fileName;
    std::cin >> fileName;
    if (!std::filesystem::exists(fileName))
    {
        std::cout << "ERROR: File not found\n";
        return;
    }

    std::ifstream insrtuction_file(fileName);
    std::string input;
    while (!insrtuction_file.eof())
    {
        insrtuction_file.clear();
        insrtuction_file >> input;
        // create <name> [ default | custom <d> <b>]
        if (input == "create")
        {
            std::string name;
            insrtuction_file >> name;
            int d, b;
            insrtuction_file >> input;
            if (input == "default")
            {
                this->createCollection(name);
            }
            else if (input == "custom")
            {
                insrtuction_file >> d;
                insrtuction_file >> b;
                this->createCollection(name, d, b);
            }
            else
            {
                std::cout << "wrong arguments\n";
            }
        }
        // delete <name>
        else if (input == "remove")
        {
            std::string name;
            insrtuction_file >> name;
            this->deleteCollection(name);
        }
        // open <name>
        else if (input == "open")
        {
            std::string name;
            insrtuction_file >> name;
            this->openCollection(name);
        }
        // close
        else if (input == "close")
        {
            this->close();
        }
        // insert [key <key> | nokey] <a> <b> <h>
        else if (input == "insert")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            insrtuction_file >> input;
            if (input == "key")
            {
                Key key;
                insrtuction_file >> key;
                double a, b, h;
                insrtuction_file >> a >> b >> h;
                this->insert(Record(key, Trapezoid(a, b, h)));
            }
            else if (input == "nokey")
            {
                double a, b, h;
                insrtuction_file >> a >> b >> h;
                Key key = this->insertWithoutKey(Trapezoid(a, b, h), true);
                std::cout << "INFO: Record inserted with key " << key << "\n";
            }
            else
            {
                std::cout << "wrong arguments\n";
            }
        }
        // randinsert <amount>
        else if (input == "randinsert")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            this->resetOperationsCount();
            int amount;
            insrtuction_file >> amount;
            for (int i = 0; i < amount; i++)
            {
                this->insertWithoutKey(Trapezoid(rand() / 100., rand() / 100., rand() / 100.), false);
            }
            this->printOperationsCount();
        }
        // update <key> <a> <b> <h>
        else if (input == "update")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            insrtuction_file >> key;
            double a, b, h;
            insrtuction_file >> a >> b >> h;
            this->modify(key, Trapezoid(a, b, h));
        }
        // remove <key>
        else if (input == "delete")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            insrtuction_file >> key;
            this->remove(key);

        }
        // get <key>
        else if (input == "get")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            insrtuction_file >> key;
            Record record = this->get(key);
            if (record.getKey() != key)
                std::cout << "Record of key " << key << " not found\n";
            else
                std::cout << "Fetched record: " << record << "\n";
        }
        // getall
        else if (input == "getall")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            this->printAllInOrder();
        }
        // showtree
        else if (input == "showtree")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            this->resetOperationsCount();
            this->index_file->showTree();
            this->printOperationsCount();
        }
        // showfile
        else if (input == "showfile")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                insrtuction_file.ignore(INT_MAX, '\n');
                continue;
            }
            this->printFiles();
        }
        else
        {
            std::cout << "unknown command\n";
        }

        insrtuction_file.ignore(INT_MAX, '\n');
    }

    insrtuction_file.close();
    this->close();
}

void Database::tui()
{
    std::cout << "text user interface (type help for help)\n";
    std::string input;
    do
    {
        std::cout << this->name << "> ";
        std::cin.clear();
        std::cin >> input;

        // create <name> [ default | custom <d> <b>]
        if (input == "create")    
        {
            std::string name;
            std::cin >> name;
            int d, b;
            std::cin >> input;
            if (input == "default")
            {
                this->createCollection(name);
            }
            else if (input == "custom")
            {
                std::cin >> d;
                std::cin >> b;
                this->createCollection(name, d, b);
            }
            else
            {
                std::cout << "wrong arguments\n";
            }
        }
        // remove <name>
        else if (input == "remove")     
        {
            std::string name;
            std::cin >> name;
            this->deleteCollection(name);
        }
        // open <name>
        else if (input == "open")       
        {
            std::string name;
            std::cin >> name;
            this->openCollection(name);
        }
        // close
        else if (input == "close")
        {
            this->close();
        }
        // insert [key <key> | nokey] <a> <b> <h>
        else if (input == "insert")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            std::cin >> input;
            if (input == "key")
            {
                Key key;
                std::cin >> key;
                double a, b, h;
                std::cin >> a >> b >> h;
                this->insert(Record(key, Trapezoid(a, b, h)));
            }
            else if (input == "nokey")
            {
                double a, b, h;
                std::cin >> a >> b >> h;
                Key key = this->insertWithoutKey(Trapezoid(a, b, h), true);
                std::cout << "INFO: Record inserted with key " << key << "\n";
            }
            else
            {
                std::cout << "wrong arguments\n";
            }
        }
        // randinsert <amount>
        else if (input == "randinsert")
        {

            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            this->resetOperationsCount();
            int amount;
            std::cin >> amount;
            for (int i = 0; i < amount; i++)
            {
                this->insertWithoutKey( Trapezoid(rand() / 100., rand() / 100., rand() / 100.), false);
            }
            this->printOperationsCount();
        }
        // update <key> <a> <b> <h>
        else if (input == "update")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            std::cin >> key;
            double a, b, h;
            std::cin >> a >> b >> h;
            this->modify(key, Trapezoid(a, b, h));
        }
        // delete <key>
        else if (input == "delete")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            std::cin >> key;
            this->remove(key);
            
        }
        // get <key>
        else if (input == "get")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            Key key;
            std::cin >> key;
            Record record = this->get(key);
            if (record.getKey() != key)
                std::cout << "Record of key " << key << " not found\n";
            else
                std::cout << "Fetched record: " << record << "\n";
        }
        // getall
        else if (input == "getall")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            this->printAllInOrder();
        }
        // showtree
        else if (input == "showtree")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            this->resetOperationsCount();
            this->index_file->showTree();
            this->printOperationsCount();
        }
        // showfile
        else if (input == "showfile")
        {
            if (!this->is_open)
            {
                std::cout << "FAIL: no collection open\n";
                std::cin.ignore(INT_MAX, '\n');
                continue;
            }
            this->printFiles();
        }
        // help
        else if (input == "help")
        {
            std::cout << "create <name> [ default | custom <d> <b>] - create a collection\n";
            std::cout << "remove <name> - delete a collection\n";
            std::cout << "open <name> - open a collection\n";
            std::cout << "close - close the opened collection\n";
            std::cout << "insert [key <key> | nokey] <a> <b> <h> - insert record\n";
            std::cout << "update <key> <a> <b> <h> - update record\n";
            std::cout << "delete <key> - delete a record\n";
            std::cout << "getall - show all reccords\n";
            std::cout << "showtree - show tree structure\n";
            std::cout << "showfile - show file structures\n";
            std::cout << "help - show this\n";
            std::cout << "exit - leave the text interface\n";
        }
        // exit
        else if (input == "exit")
        {
            std::cout << "Bye!\n";
            if (this->is_open)
                this->close();
            break;
        }
        else
        {
            std::cout << "unknown command\n";
        }

        std::cin.ignore(INT_MAX, '\n');
    } while (input != "exit");
}

Key Database::insertWithoutKey(Trapezoid trapezoid, bool count_operations = true)
{
    if(count_operations) this->resetOperationsCount();
    
    Key key;
    do
    {
        this->index_file->resetLoaded();
        key = (rand() % (UINT64_MAX - 2) +1);
    } while (this->index_file->search(key).key != -1);
    unsigned int record_index = this->main_file->putRecordInFile({key, trapezoid});
    std::cout << (this->index_file->insert({ key, record_index }) ? "INFO: insert success\n" : "INFO: insert fail\n");
    if (count_operations) this->printOperationsCount();
    //this->printOperationsCount();
    return key;
}

bool Database::insert(Record record_with_key)
{
    this->resetOperationsCount();
    unsigned int record_index = this->main_file->putRecordInFile(record_with_key);
    if (this->index_file->insert({ record_with_key.getKey(), record_index }))
    {
        std::cout << "INFO: Record " << record_with_key << " inserted\n";
        this->printOperationsCount();
        return true;
    }
    this->main_file->deleteRecordFromFile(record_index);
    this->printOperationsCount();
    return false;
}

Record Database::get(Key key)
{
    this->resetOperationsCount();
    Record record = { UINT64_MAX, {0,0,0} };
    KeyStruct search_result = this->index_file->search(key);
    if (search_result.key == key)
    {
        record = this->main_file->getRecordFromFile(search_result.record_index);
    }
    this->index_file->resetLoaded();
    this->printOperationsCount();
    return record;
}

void Database::remove(Key key)
{
    this->resetOperationsCount();
    KeyStruct remove_result = this->index_file->remove(key);
    if (remove_result.key == key)
        this->main_file->deleteRecordFromFile(remove_result.record_index);
    this->printOperationsCount();
}

void Database::modify(Key key, Trapezoid trapezoid)
{
    this->resetOperationsCount();
    Record record = { UINT64_MAX, {0,0,0} };
    KeyStruct search_result = this->index_file->search(key);
    if (search_result.key == key)
    {
        //record = this->main_file->getRecordFromFile(search_result.record_index);
        std::cout << "INFO: Value for record of key " << key << " applied\n";
        this->main_file->replaceRecordInFile(search_result.record_index, Record(key, trapezoid));
    }
    this->index_file->offloadAll(); 
    this->printOperationsCount();
    
}

void Database::printFiles()
{
    this->resetOperationsCount();
    std::cout << "MAIN FILE\n";
    this->main_file->printFile(); 
    std::cout << "\n\nINDEX FILE\n";
    this->index_file->showFile();
    this->printOperationsCount();
}

void Database::printAllInOrder()
{
    this->resetOperationsCount();
    this->index_file->startKeyByKey();
    KeyStruct search_result(0, 0);
    unsigned int record_count = 0;
    do
    {
        search_result = this->index_file->getNextKey();
        if (search_result.record_index == UINT64_MAX)
        {
            std::cout << "== Total " << record_count << " records ==\n";
        }
        else
        {
            std::cout << this->main_file->getRecordFromFile(search_result.record_index) << "\n";
        }
        record_count++;

    } while (search_result.key != UINT64_MAX);

    this->index_file->stopKeyByKey();
    this->printOperationsCount();
    //std::cout << "INFO: DRIVE READS: " << this->getDriveReads() << " DRIVE WRITES: " << this->getDriveWrites() << "\n";
}

void Database::resetOperationsCount()
{
    this->index_file->getDriveReads();
    this->main_file->getDriveReads();
    this->index_file->getDriveWrites();
    this->main_file->getDriveWrites();
}

void Database::printOperationsCount()
{
    std::cout << "INFO: MAINFILE READS: " << this->main_file->getDriveReads() << " WRITES: " << this->main_file->getDriveWrites() << "\n";
    std::cout << "INFO: INDEXFILE READS: " << this->index_file->getDriveReads() << " WRITES: " << this->index_file->getDriveWrites() << "\n";
}

int Database::getDriveReads()
{
    return this->main_file->getDriveReads() + this->index_file->getDriveReads();
}

int Database::getDriveWrites()
{
    return this->main_file->getDriveWrites() + this->index_file->getDriveWrites();
}
