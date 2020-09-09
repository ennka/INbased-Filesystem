#include "Filesystem.h"
using namespace std;

char buffer[SYSTEM_SIZE];

INode::INode() {
	this->id = -1;
	this->fsize = 0;
	this->fmode = 0;
	this->ctime = 0;
	this->count = 0;
	this->mcount = 0;
	memset(dir_address, 0, sizeof(dir_address));
	memset(indir_address, 0, sizeof(indir_address));
}

INode::~INode() {
}

void INode::clear() {
	this->id = -1;
	this->fsize = 0;
	this->fmode = 0;
	this->ctime = 0;
	this->count = 0;
	this->mcount = 0;
	memset(dir_address, 0, sizeof(dir_address));
	memset(indir_address, 0, sizeof(indir_address));
}

Address::Address() {
	addressByte[0] = addressByte[1] = addressByte[2] = 0;
}

Address::~Address() {
}

int Address::getblockID() {

	int address = (((addressByte[0] << 8) | addressByte[1]) << 8) | addressByte[2];//将3个byte转化为int
	return int(address >> BLOCK_OFFSET_SIZE);

}

int Address::getOffset() {
	int address = (((addressByte[0] << 8) | addressByte[1]) << 8) | addressByte[2];//将3个byte转化为int
	return int(address ^ ((address >> BLOCK_OFFSET_SIZE) << BLOCK_OFFSET_SIZE));
}

void Address::setblockID(int id) {
	bool bit[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int temp = id;
	unsigned char tempBit[3];
	for (int i = 0; temp != 0; i++) {
		bit[i + 2] = temp % 2;
		temp = temp / 2;
	}
	for (int i = 0; i < 8; i++) {
		tempBit[0] = (tempBit[0] << 1) | bit[15 - i];
	}
	for (int i = 0; i < 8; i++) {
		tempBit[1] = (tempBit[1] << 1) | bit[7 - i];
	}
	unsigned char term = (1 << 1) | 1;
	addressByte[0] = tempBit[0];
	addressByte[1] = tempBit[1] | (addressByte[1] & term);
}

void Address::setOffset(int offset) {
	int bit[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int temp = offset;
	unsigned char tempBit[3];
	for (int i = 0; temp != 0; i++) {
		bit[i] = temp % 2;
		temp = temp / 2;
	}
	for (int i = 0; i < 8; i++) {
		tempBit[1] = (tempBit[1] << 1) | bit[15 - i];
	}
	for (int i = 0; i < 8; i++) {
		tempBit[2] = (tempBit[2] << 1) | bit[7 - i];
	}
	unsigned char term = ((((((1 << 1) | 1) << 1 | 1) << 1 | 1) << 1 | 1) << 1 | 1) << 2;//11111100
	addressByte[2] = tempBit[2];
	addressByte[1] = tempBit[1] | (addressByte[1] & term);
}


System::System() {
	strcpy(current_Path, root_dir);
	flag = true;
}

System::~System() {
	fclose(fp);
}

void System::initialize() {
	fstream file;
	file.open(HOME, ios::in);
	if (!file) {
		cout << "Initializing..." << endl;
		cout << endl;
		fp = fopen(HOME, "wb+");
		if (fp == NULL) {
			cout << "Error occurs when creating the Operating System..." << endl;
			flag = false;
			return;
		}

		fwrite(buffer, SYSTEM_SIZE, 1, fp); //创建一个16MB的空间

		fseek(fp, 0, SEEK_SET);
		superblock.systemsize = SYSTEM_SIZE;
		superblock.blocksize = BLOCK_SIZE;
		superblock.blocknum = BLOCK_NUM;
		superblock.address_length = ADDRESS_LENGTH;
		superblock.max_filename_size = MAX_FILENAME_SIZE;
		superblock.superblock_size = SUPER_BLOCK_SIZE;
		superblock.inode_size = INODE_SIZE;
		superblock.inode_bitmap_size = INODE_BITMAP_SIZE;
		superblock.block_bitmap_size = BLOCK_BITMAP_SIZE;
		superblock.inode_block_size = INODE_BLOCK_SIZE;
		fwrite(&superblock, sizeof(Superblock), 1, fp); // 写入超级块

		fseek(fp, INODE_BITMAP_START, SEEK_SET);
		for (int i = 0; i < INODE_BITMAP_SIZE; ++i) {
			unsigned char byte = 0;
			fwrite(&byte, sizeof(unsigned char), 1, fp);
		} // 初始化bitmap

		fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
		for (int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
			unsigned char byte = 0;
			fwrite(&byte, sizeof(unsigned char), 1, fp);
		} // 初始化bitmap

		modifyBlockBitmap(0); //设置块的状态
		for (int i = 0; i < INODE_BLOCK_SIZE / 1024 + INODE_BITMAP_SIZE / 1024 + BLOCK_BITMAP_SIZE / 1024; ++i) {
			modifyBlockBitmap(i + 1);
			//cout << i + 1 << endl;
		} // 设置块的状态

		root_Inode.clear();
		root_Inode.count = 0;
		root_Inode.mcount = 0;
		root_Inode.ctime = time(NULL);
		root_Inode.fmode = DENTRY_MODE;
		root_Inode.id = 0;
		modifyInodeBitmap(0);

		writeInode(0, root_Inode);
		current_Inode = root_Inode;
	}
	else {
		cout << "The Operating System is loading..." << endl;
		cout << endl;
		fp = fopen(HOME, "rb+");
		if (fp == NULL) {
			cout << "Error when loading the Operating System..." << endl;
			flag = false;
			return;
		}
		fseek(fp, 0, SEEK_SET);
		fread(&superblock, sizeof(Superblock), 1, fp);
		current_Inode = root_Inode = readInode(0);
	}
}

void System::tip() {
	cout << "[root@INODE_OS~]:" << current_Path << "# ";
}

void System::help() {
	cout << "Commands supported in this Operating System:" << endl << endl;

	cout << "createFile <fileName> <fileSize>" << endl;
	cout << "\te.g. createFile /dir/myFile 10" << endl << endl;

	cout << "deleteFile <fileName>" << endl;
	cout << "\te.g. deleteFile /dir/myFile" << endl << endl;

	cout << "createDir <dirPath>" << endl;
	cout << "\te.g. createDir /dir/sub" << endl << endl;

	cout << "deleteDir <dirPath>" << endl;
	cout << "\te.g. deleteDir /dir/sub" << endl << endl;

	cout << "changeDir <dirPath>" << endl;
	cout << "\te.g. changeDir /dir" << endl << endl;

	cout << "dir" << endl;
	cout << "\te.g. dir" << endl << endl;

	cout << "cp <fileName1> <fileName2>" << endl;
	cout << "\te.g. cp file1 file2" << endl << endl;

	cout << "sum" << endl;
	cout << "\te.g. sum" << endl << endl;

	cout << "cat <fileName>" << endl;
	cout << "\te.g. cat /dir/myFile" << endl << endl;

	cout << "help" << endl;
	cout << "\te.g. help" << endl << endl;

	cout << "exit" << endl;
	cout << "\te.g. exit" << endl << endl;
}

void System::copyright() {
	cout << endl;

	cout << "Inode-based operating system" << endl;
	cout << "Group members: " << endl;

	cout << endl;

	cout << "\tShuowei Cai\t\t201730612390" << endl;
	cout << "\tShiju Lin\t\t201730631209" << endl;
	cout << "\tZhixuan Zhong\t\t201764615138" << endl;

	cout << endl;

	cout << "Type \"help\" to get the commands in this operating system." << endl;
	cout << endl;

}

int System::findAvailableInode() {
	fseek(fp, INODE_BITMAP_START, SEEK_SET);
	int pos = -1;
	for (int i = 0; i < INODE_BITMAP_SIZE; ++i) {
		unsigned char byte;
		fread(&byte, sizeof(unsigned char), 1, fp);
		for (int j = 0; j < 8; ++j) {
			if (((byte >> j) & 1) == 0) {
				pos = i * 8 + j;
				break;
			}
		}
		if (pos != -1) break;
	}
	return pos;
}

int System::findAvailableBlock() {
	fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
	int pos = -1;
	for (int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
		unsigned char byte;
		fread(&byte, sizeof(unsigned char), 1, fp);
		for (int j = 0; j < 8; ++j) {
			if (((byte >> j) & 1) == 0) {
				pos = i * 8 + j;
				break;
			}
		}
		if (pos != -1) break;
	}
	return pos;
}

int System::numberOfAvailableBlock() {
	int unused = 0;
	fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
	for (int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
		unsigned char byte;
		fread(&byte, sizeof(unsigned char), 1, fp);
		for (int j = 0; j < 8; ++j) {
			if (((byte >> j) & 1) == 0) 
				unused++;
		}
	}
	return unused;
}

void System::sum() {
	int unused = numberOfAvailableBlock();
	int used = BLOCK_NUM - unused;

	cout << "System Size: " << superblock.systemsize << " Bytes" << endl;
	cout << "Block Size: " << superblock.blocksize << " Bytes" << endl;
	cout << "Size of iNode bitmap: " << superblock.inode_bitmap_size << " Bytes" << endl;
	cout << "Size of block bitmap: " << superblock.inode_bitmap_size << " Bytes" << endl;
	cout << "Memory for storing iNodes: " << superblock.inode_block_size << " Bytes" << endl;
	cout << "Total number of blocks: " << superblock.blocknum << endl;
	cout << "Number of Blocks that have been used: " << used << endl;
	cout << "Number of Blocks that are available: " << unused << endl;
	cout << endl;

}

void System::modifyInodeBitmap(int pos) {
	int origin = pos / 8;
	int offset = pos % 8;
	unsigned char byte;
	fseek(fp, INODE_BITMAP_START + origin, SEEK_SET);
	fread(&byte, sizeof(unsigned char), 1, fp);
	byte = (byte ^ (1 << offset));
	fseek(fp, INODE_BITMAP_START + origin, SEEK_SET);
	fwrite(&byte, sizeof(unsigned char), 1, fp);
}

void System::modifyBlockBitmap(int pos) {
	int origin = pos / 8;
	int offset = pos % 8;
	unsigned char byte;
	fseek(fp, BLOCK_BITMAP_START + origin, SEEK_SET);
	fread(&byte, sizeof(unsigned char), 1, fp);
	byte = (byte ^ (1 << offset));
	fseek(fp, BLOCK_BITMAP_START + origin, SEEK_SET);
	fwrite(&byte, sizeof(unsigned char), 1, fp);
}

void System::writeInode(int pos, INode inode) {
	fseek(fp, INODE_TABLE_START + INODE_SIZE * pos, SEEK_SET);
	fwrite(&inode, sizeof(INode), 1, fp);
}

INode System::readInode(int pos) {
	INode inode;
	fseek(fp, INODE_TABLE_START + INODE_SIZE * pos, SEEK_SET);
	fread(&inode, sizeof(INode), 1, fp);
	return inode;
}

void System::result(string command, State st) {
	if (st != SUCCESS)
		cout << "Command \'" << command << "\': ";
	if (st == DIR_NOT_EXIST) {
		cout << "The directory does not exist!" << endl;
	}
	else if (st == NO_FILENAME) {
		cout << "No file name provided!" << endl;
	}
	else if (st == FILE_EXISTS) {
		cout << "File already existed!" << endl;
	}
	else if (st == LENGTH_EXCEED) {
		cout << "The file name is too long" << endl;
	}
	else if (st == DIRECTORY_EXCEED) {
		cout << "Too many files in the directory!" << endl;
	}
	else if (st == NO_ENOUGH_SPACE) {
		cout << "The system has no enough space for the file!" << endl;
	}
	else if (st == NO_SUCH_FILE) {
		cout << "No such file!" << endl;
	}
	else if (st == NO_DIRNAME) {
		cout << "No directory name provided!" << endl;
	}
	else if (st == NO_SUCH_DIR) {
		cout << "No such directory!" << endl;
	}
	else if (st == DIR_NOT_EMPTY) {
		cout << "The directory is not empty!" << endl;
	}
	else if (st == CAN_NOT_DELETE_TEMP_DIR) {
		cout << "You cannot delete the current directory!" << endl;
	}
	else if (st == DIR_EXISTS) {
		cout << "The directory already existed!" << endl;
	}
}

void System::dir() {
	current_Inode = readInode(current_Inode.id);
	int s1=22, s2=8, s3=14, s4=25;
	cout << left << setw(s1) << "Name";
	cout << left << setw(s2) << "Mode";
	cout << left << setw(s3) << "Size(Bytes)";
	cout << left << setw(s4) << "Created Time";
	cout << endl;
	cout << left << setw(s1) << "---------------------";
	cout << left << setw(s2) << "------";
	cout << left << setw(s3) << "-------------";
	cout << left << setw(s4) << "------------------------";
	cout << endl;


	int count = current_Inode.mcount;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);
	for (int i = 0; i < current_Inode.NUM_DIRECT_ADDRESS; ++i) {
		for (int j = 0; j < FILE_PER_BLOCK; ++j) {
			if (count == 0) break;
			count--;
			File file;
			fseek(fp, BLOCK_SIZE*current_Inode.dir_address[i] + sizeof(File)*j, SEEK_SET);
			fread(&file, sizeof(File), 1, fp);
			if (file.inode_id == -1) continue;//该文件已经被删除
			INode inode = readInode(file.inode_id);
			cout << left << setw(s1) << file.filename;
			if (inode.fmode == DENTRY_MODE) cout << left << setw(s2) << "Dentry";
			else cout << left << setw(s2) << "File";
			if (inode.fmode == DENTRY_MODE) cout << left << setw(s3) << getSize(inode);
			else cout << left << setw(s3) << inode.fsize * 1024;
			char buffer[100];
			strftime(buffer, 50, "%a %b %d %T %G", localtime(&inode.ctime));
			cout << left << setw(s4) << buffer;
			cout << " ";
			cout << endl;
		}
	}
}

int System::getSize(INode cur_Inode)
{
	cur_Inode = readInode(cur_Inode.id);
	int count = cur_Inode.mcount;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);
	int res = 0;
	for (int i = 0; i < cur_Inode.NUM_DIRECT_ADDRESS; ++i) {
		for (int j = 0; j < FILE_PER_BLOCK; ++j) {
			if (count == 0) break;
			count--;
			File file;
			fseek(fp, BLOCK_SIZE*cur_Inode.dir_address[i] + sizeof(File)*j, SEEK_SET);
			fread(&file, sizeof(File), 1, fp);
			if (file.inode_id == -1) continue;//该文件已经被删除
			INode inode = readInode(file.inode_id);
			if (inode.fmode == DENTRY_MODE) res += getSize(inode);
			else res += inode.fsize * 1024;
		}
	}
	return res;
}
State System::createFile(string fileName, int fileSize) {
	//第一步：正常的抵达需要创建文件的目录，确认文件可以正常创建
	int len = (int)fileName.size();
	INode inode;
	if (fileName[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> vect;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (fileName[i] == '/') {
			if (temp != "") {
				vect.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += fileName[i];
	}

	if (temp == "") return NO_FILENAME;//此时的temp为文件名
	if ((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;
	vect.push_back(temp);

	int count = (int)vect.size();

	for (int i = 0; i < count - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, vect[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nextInode;
	}
	bool ok = true;
	INode nextInode = findNextInode(inode, vect[count - 1], ok);//vect[count-1]是文件名
	if (ok) return FILE_EXISTS;
	int SUM_OF_DIRECTORY = superblock.blocksize / sizeof(File)*INode::NUM_DIRECT_ADDRESS;
	if (inode.count >= SUM_OF_DIRECTORY) return DIRECTORY_EXCEED;

	int unused = numberOfAvailableBlock();
	if (unused < fileSize) return NO_ENOUGH_SPACE;
	if (fileSize > MAX_FILE_SIZE) return NO_ENOUGH_SPACE;


	//第二步：准备好存储文件信息的Inode，并将对应的Inodeid存入文件类中
	File file;
	file.inode_id = findAvailableInode();
	modifyInodeBitmap(file.inode_id);
	strcpy(file.filename, vect[count - 1].c_str());
	INode newInode;//创建文件的I-node
	newInode.clear();
	newInode.fsize = fileSize;
	newInode.ctime = time(NULL);
	newInode.fmode = FILE_MODE;
	newInode.id = file.inode_id;
	
	//第三步：把文件写入目标I节点，注意inode这里是目录的I节点
	writeFileToDentry(file, inode);

	//第四步：开始向块中写入内容
	int tempSize = fileSize;//单位KB 与blocksize相同
	for (int i = 0; i < INode::NUM_DIRECT_ADDRESS; ++i) {
		if (tempSize == 0) {
			break;
		}
		tempSize--;
		newInode.dir_address[i] = findAvailableBlock();
		modifyBlockBitmap(newInode.dir_address[i]);
		writeRandomStringToBlock(newInode.dir_address[i]);
	}
	if (tempSize > 0) {//直接块被使用完了
		newInode.indir_address[0] = findAvailableBlock();
		modifyBlockBitmap(newInode.indir_address[0]); 
		int count = 0;
		while (tempSize > 0) {
			tempSize--;
			int blockid = findAvailableBlock();
			modifyBlockBitmap(blockid);
			Address address;
			address.setblockID(blockid);
			address.setOffset(0);
			writeRandomStringToBlock(blockid);
			writeAddressToBlock(address, newInode.indir_address[0], count);//将文件的地址写入非直接块中
			count++;
		}
	}
	writeInode(newInode.id, newInode);
	return SUCCESS;
}

void System::writeRandomStringToBlock(int blockid) {
	srand(time(0));
	fseek(fp, blockid*superblock.blocksize, SEEK_SET);
	for (int i = 0; i < superblock.blocksize; ++i) {
		char randomChar = (rand() % 26) + 'a';
		// cout << randomChar << endl;
		fwrite(&randomChar, sizeof(char), 1, fp);
	}
}

void System::writeAddressToBlock(Address address, int blockid, int offset) {
	fseek(fp, blockid*superblock.blocksize + offset * sizeof(Address), SEEK_SET);
	fwrite(&address, sizeof(Address), 1, fp);
}

void System::writeFileToDentry(File file, INode inode) {
	int count = inode.count;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);

	if (inode.mcount == count) {
		if (count % FILE_PER_BLOCK == 0) {
			inode.dir_address[count / FILE_PER_BLOCK] = findAvailableBlock();
			modifyBlockBitmap(inode.dir_address[count / FILE_PER_BLOCK]);
			fseek(fp, BLOCK_SIZE*inode.dir_address[count / FILE_PER_BLOCK], SEEK_SET);
			// cout << BLOCK_SIZE*inode.dir_address[cnt/FILE_PER_BLOCK] << " " << (int)ftell(fp) << endl;
		}
		else {
			fseek(fp, BLOCK_SIZE*inode.dir_address[count / FILE_PER_BLOCK] + sizeof(File)*(count%FILE_PER_BLOCK), SEEK_SET);
		}
		// cout << (int)ftell(fp) << endl;
		fwrite(&file, sizeof(File), 1, fp);

		inode.count++;
		inode.mcount++;
		writeInode(inode.id, inode);
	}
	else {
		// cout << inode.count << " " << inode.mcount << "\n";
		bool ok = false;
		int temp = inode.mcount;
		for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
			if (temp == 0) break;
			for (int j = 0; j < FILE_PER_BLOCK; ++j) {
				if (temp == 0) break;
				temp--;
				File tempfile;
				fseek(fp, BLOCK_SIZE*inode.dir_address[i] + sizeof(File)*j, SEEK_SET);
				fread(&tempfile, sizeof(File), 1, fp);
				if (tempfile.inode_id == -1) {
					// cout << i << " " << j << "\n";
					ok = true;
					fseek(fp, BLOCK_SIZE*inode.dir_address[i] + sizeof(File)*j, SEEK_SET);
					fwrite(&file, sizeof(File), 1, fp);
				}
				if (ok) break;
			}
			if (ok) break;
		}

		inode.count++;
		writeInode(inode.id, inode);
	}

	if (inode.id == current_Inode.id) current_Inode = readInode(current_Inode.id);
	if (inode.id == root_Inode.id) root_Inode = readInode(root_Inode.id);

}

INode System::findNextInode(INode inode, string fileName, bool &canFind) {
	int count = inode.mcount;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (count == 0) break;
		fseek(fp, BLOCK_SIZE*inode.dir_address[i], SEEK_SET);
		for (int j = 0; j < FILE_PER_BLOCK; ++j) {
			if (count == 0) break;
			count--;
			File file;
			fread(&file, sizeof(File), 1, fp);
			if (file.inode_id == -1) continue;
			if (strcmp(file.filename, fileName.c_str()) == 0) {//如果名字一样，则返回
				return readInode(file.inode_id);
			}
		}
	}
	canFind = false;
	return inode;
}

void System::deleteFileFromDentry(INode inode, string fileName) {
	int count = inode.mcount;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);
	bool ok = false;
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (count == 0) break;
		fseek(fp, BLOCK_SIZE*inode.dir_address[i], SEEK_SET);
		for (int j = 0; j < FILE_PER_BLOCK; ++j) {
			if (count == 0) break;
			count--;
			File file;
			fread(&file, sizeof(File), 1, fp);
			if (file.inode_id == -1) continue;
			if (strcmp(file.filename, fileName.c_str()) == 0) {
				fseek(fp, -(int)sizeof(File), SEEK_CUR);
				file.inode_id = -1;
				fwrite(&file, sizeof(File), 1, fp);
				ok = true;
			}
			if (ok) break;
		}
		if (ok) break;
	}
	inode.count--;
	writeInode(inode.id, inode);
}

State System::deleteFile(string fileName) {
	int len = (int)fileName.size();
	INode inode;
	if (fileName[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> vect;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (fileName[i] == '/') {
			if (temp != "") {
				vect.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += fileName[i];
	}
	if (temp == "") return NO_SUCH_FILE;
	vect.push_back(temp);

	int cnt = (int)vect.size();

	for (int i = 0; i < cnt - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, vect[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return NO_SUCH_FILE;
		inode = nextInode;
	}

	bool ok = true;
	INode nextInode = findNextInode(inode, vect[cnt - 1], ok);
	if (nextInode.fmode == DENTRY_MODE) ok = false;
	if (!ok) return NO_SUCH_FILE;
	//已经到达目标目录，可以创建文件了
	deleteFileFromDentry(inode, vect[cnt - 1]);//在该目录所属的I节点下删除对应名字的文件

	inode = nextInode;
	modifyInodeBitmap(inode.id);//释放掉所删除的文件所对应的I节点
	int filesize = inode.fsize;
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {//释放直接块
		if (filesize == 0) {
			break;
		}
		filesize--;
		modifyBlockBitmap(inode.dir_address[i]);
	}
	if (filesize > 0) {//非直接块
		modifyBlockBitmap(inode.indir_address[0]);
		int offset = 0;
		while (filesize > 0) {
			filesize--;
			Address address;
			fseek(fp, inode.indir_address[0] * superblock.blocksize + offset * sizeof(Address), SEEK_SET);
			fread(&address, sizeof(Address), 1, fp);
			modifyBlockBitmap(address.getblockID());
			offset++;
		}
	}
	return SUCCESS;
}

State System::createDir(string dirName) {
	int len = (int)dirName.size();
	INode inode;
	if (dirName[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (dirName[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += dirName[i];
	}
	if (temp == "") return NO_DIRNAME;
	if ((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;
	// cout << temp << endl;
	v.push_back(temp);

	int count = (int)v.size();

	for (int i = 0; i < count - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, v[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nextInode;
	}
	bool ok = true;
	INode nextInode = findNextInode(inode, v[count - 1], ok);
	if (ok) return DIR_EXISTS;

	int SUM_OF_DIRECTORY = superblock.blocksize / sizeof(File)*INode::NUM_DIRECT_ADDRESS;
	if (inode.count >= SUM_OF_DIRECTORY) return DIRECTORY_EXCEED;
	// 抵达目标目录，确认有足够空间可以创建目录。

	File file;
	file.inode_id = findAvailableInode();
	modifyInodeBitmap(file.inode_id);
	strcpy(file.filename, v[count - 1].c_str());
	INode newInode;
	newInode.clear();
	newInode.fsize = 0;
	newInode.ctime = time(NULL);
	newInode.fmode = DENTRY_MODE;
	newInode.id = file.inode_id;

	writeFileToDentry(file, inode);

	writeInode(newInode.id, newInode);
	return SUCCESS;
}

State System::deleteDir(string dirName) {
	int len = (int)dirName.size();
	INode inode;
	if (dirName[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (dirName[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += dirName[i];
	}
	if (temp == "") return NO_SUCH_DIR;
	v.push_back(temp);

	int cnt = (int)v.size();

	for (int i = 0; i < cnt - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, v[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return NO_SUCH_DIR;
		inode = nextInode;
	}

	bool ok = true;
	INode nextInode = findNextInode(inode, v[cnt - 1], ok);
	if (nextInode.fmode == FILE_MODE) ok = false;
	if (!ok) return NO_SUCH_DIR;

	if (dirName[0] == '/') {//进行确认，不能删除当前位置的文件夹
		vector<string> cur;
		string tempdir = "";
		int sz = strlen(current_Path);
		for (int i = 1; i < sz; ++i) {
			if (current_Path[i] == '/') {
				if (tempdir != "") {
					cur.push_back(tempdir);
					tempdir = "";
				}
				continue;
			}
			tempdir += current_Path[i];
		}
		if (tempdir != "") cur.push_back(tempdir);

		int curlen = (int)cur.size();
		// cout << sz << " " << curlen << endl;
		if (cnt <= curlen) {
			bool ok = true;
			for (int i = 0; i < cnt; ++i) {
				if (v[i] != cur[i]) {
					ok = false;
					break;
				}
			}
			if (ok) return CAN_NOT_DELETE_TEMP_DIR;
		}
	}

	if (nextInode.count > 0) return DIR_NOT_EMPTY;

	deleteFileFromDentry(inode, v[cnt - 1]);

	inode = nextInode;
	modifyInodeBitmap(inode.id);
	int count = inode.mcount;
	int FILE_PER_BLOCK = superblock.blocksize / sizeof(File);
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (count <= 0) {
			break;
		}
		count -= FILE_PER_BLOCK;
		modifyBlockBitmap(inode.dir_address[i]);//释放这个目录文件对应的块
	}

	return SUCCESS;
}

State System::changeDir(string path) {
	int len = (int)path.size();

	if (path == "/") {
		current_Inode = root_Inode;
		strcpy(current_Path, root_dir);
		return SUCCESS;
	}

	INode inode;
	if (path[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> vect;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (path[i] == '/') {
			if (temp != "") {
				vect.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += path[i];
	}
	if (temp == "") return NO_DIRNAME;
	if ((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;

	vect.push_back(temp);

	int count = (int)vect.size();

	for (int i = 0; i < count - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, vect[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nextInode;
	}
	bool ok = true;
	INode nextInode = findNextInode(inode, vect[count - 1], ok);
	if (nextInode.fmode != DENTRY_MODE) ok = false;
	if (!ok) return DIR_NOT_EXIST;

	current_Inode = nextInode;//移动至目标I节点位置

	if (path[0] == '/') {//调整显示
		strcpy(current_Path, root_dir);
		for (int i = 0; i < count; ++i) {
			strcat(current_Path, "/");
			strcat(current_Path, vect[i].c_str());
		}
	}
	else {
		for (int i = 0; i < count; ++i) {
			strcat(current_Path, "/");
			strcat(current_Path, vect[i].c_str());
		}
	}
	return SUCCESS;
}

State System::cat(string fileName) {
	int len = (int)fileName.size();
	INode inode;

	if (fileName[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> v;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (fileName[i] == '/') {
			if (temp != "") {
				v.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += fileName[i];
	}
	if (temp == "") return NO_FILENAME;

	v.push_back(temp);

	int cnt = (int)v.size();

	for (int i = 0; i < cnt - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, v[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nextInode;
	}
	bool ok = true;
	INode nxtInode = findNextInode(inode, v[cnt - 1], ok);
	if (nxtInode.fmode == DENTRY_MODE) ok = false;
	if (!ok) return NO_SUCH_FILE;

	inode = nxtInode;
	//老规矩，先抵达目标节点

	int fileSize = inode.fsize;

	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (fileSize == 0) {
			break;
		}
		fileSize--;
		int blockid = inode.dir_address[i];
		fseek(fp, blockid*superblock.blocksize, SEEK_SET);
		for (int j = 0; j < BLOCK_SIZE; ++j) {
			char buffer;
			fread(&buffer, sizeof(char), 1, fp);
			cout << buffer;
		}
	}
	if (fileSize > 0) {
		int offset = 0;
		while (fileSize > 0) {
			fileSize--;
			Address address;
			fseek(fp, inode.indir_address[0] * superblock.blocksize + offset * sizeof(Address), SEEK_SET);
			fread(&address, sizeof(Address), 1, fp);
			int blockid = address.getblockID();
			fseek(fp, blockid*superblock.blocksize, SEEK_SET);
			for (int j = 0; j < BLOCK_SIZE; ++j) {
				char buffer;
				fread(&buffer, sizeof(char), 1, fp);
				cout << buffer;
			}
			offset++;
		}
	}
	cout << endl;

	return SUCCESS;
}

State System::cp(string file1, string file2) {
	//第一步：读取第一个文件的内容
	int len = (int)file1.size();
	INode inode;

	if (file1[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vector<string> vect;
	string temp = "";
	for (int i = 0; i < len; ++i) {
		if (file1[i] == '/') {
			if (temp != "") {
				vect.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += file1[i];
	}
	if (temp == "") return NO_FILENAME;

	vect.push_back(temp);

	int cnt = (int)vect.size();

	for (int i = 0; i < cnt - 1; ++i) {
		bool ok = true;
		INode nextInode = findNextInode(inode, vect[i], ok);
		if (nextInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nextInode;
	}
	bool ok = true;
	INode nextInode = findNextInode(inode, vect[cnt - 1], ok);
	if (nextInode.fmode == DENTRY_MODE) ok = false;
	if (!ok) return NO_SUCH_FILE;

	inode = nextInode;

	vector<char> content;
	int fileSize = inode.fsize;
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (fileSize == 0) {
			break;
		}
		fileSize--;
		int blockid = inode.dir_address[i];
		fseek(fp, blockid*superblock.blocksize, SEEK_SET);
		for (int j = 0; j < BLOCK_SIZE; ++j) {
			char buffer;
			fread(&buffer, sizeof(char), 1, fp);
			content.push_back(buffer);
		}
	}
	if (fileSize > 0) {
		int offset = 0;
		while (fileSize > 0) {
			fileSize--;
			Address address;
			fseek(fp, inode.indir_address[0] * superblock.blocksize + offset * sizeof(Address), SEEK_SET);
			fread(&address, sizeof(Address), 1, fp);
			int blockid = address.getblockID();
			fseek(fp, blockid*superblock.blocksize, SEEK_SET);
			for (int j = 0; j < BLOCK_SIZE; ++j) {
				char buffer;
				fread(&buffer, sizeof(char), 1, fp);
				content.push_back(buffer);
			}
			offset++;
		}
	}


	State state = createFile(file2, inode.fsize);//第二步：先创建一个文件的框架在这里
	if (state != SUCCESS) return state;
	len = (int)file2.size();

	//第三步：抵达这个文件的目录
	if (file2[0] == '/') {
		inode = root_Inode;
	}
	else {
		inode = current_Inode;
	}
	vect.clear();
	temp = "";
	for (int i = 0; i < len; ++i) {
		if (file2[i] == '/') {
			if (temp != "") {
				vect.push_back(temp);
				temp = "";
			}
			continue;
		}
		temp += file2[i];
	}
	if (temp == "") return NO_FILENAME;

	vect.push_back(temp);

	cnt = (int)vect.size();

	for (int i = 0; i < cnt - 1; ++i) {
		bool ok = true;
		INode nxtInode = findNextInode(inode, vect[i], ok);
		if (nxtInode.fmode != DENTRY_MODE) ok = false;
		if (!ok) return DIR_NOT_EXIST;
		inode = nxtInode;
	}
	ok = true;
	nextInode = findNextInode(inode, vect[cnt - 1], ok);
	if (nextInode.fmode == DENTRY_MODE) ok = false;
	if (!ok) return NO_SUCH_FILE;

	//第四步：覆盖这个新创建的文件的内容
	inode = nextInode;
	
	fileSize = inode.fsize;
	int index = 0;
	for (int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
		if (fileSize == 0) {
			break;
		}
		fileSize--;
		int blockid = inode.dir_address[i];
		fseek(fp, blockid*superblock.blocksize, SEEK_SET);
		for (int j = 0; j < BLOCK_SIZE; ++j) {
			char buffer = content[index++];
			fwrite(&buffer, sizeof(char), 1, fp);
		}
	}
	if (fileSize > 0) {
		int offset = 0;
		while (fileSize > 0) {
			fileSize--;
			Address address;
			fseek(fp, inode.indir_address[0] * superblock.blocksize + offset * sizeof(Address), SEEK_SET);
			fread(&address, sizeof(Address), 1, fp);
			int blockid = address.getblockID();
			fseek(fp, blockid*superblock.blocksize, SEEK_SET);
			for (int j = 0; j < BLOCK_SIZE; ++j) {
				char buffer = content[index++];
				fwrite(&buffer, sizeof(char), 1, fp);
			}
			offset++;
		}
	}

	return SUCCESS;
}



