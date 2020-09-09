
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define _CRT_SECURE_NO_WARNINGS
#define MAX_FILENAME_SIZE (20)
#define BLOCK_ID_SIZE (14)
#define BLOCK_OFFSET_SIZE (10)

#define SYSTEM_SIZE (16*1024*1024)
#define BLOCK_SIZE (1024)
#define BLOCK_NUM (16*1024)
#define ADDRESS_LENGTH (24)
#define BLOCK_BIT (14)
#define OFFSET (10)
#define MAX_PATH (1000)
#define INODE_SIZE (128)
#define SUPER_BLOCK_SIZE (1024)
#define INODE_BITMAP_SIZE (2*1024)
#define BLOCK_BITMAP_SIZE (2*1024)
#define INODE_BLOCK_SIZE (2*1024*1024)
#define INODE_BITMAP_START SUPER_BLOCK_SIZE
#define BLOCK_BITMAP_START (INODE_BITMAP_START+INODE_BITMAP_SIZE)
#define INODE_TABLE_START (BLOCK_BITMAP_START+BLOCK_BITMAP_SIZE)
#define MAX_FILE_SIZE (351)
#define FILE_MODE 0
#define DENTRY_MODE 1
#define HOME "unix.os"
#define root_dir "~"

#include <fstream>
#include <ctime>
#include <iomanip>
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
//#include "Superblock.h"
//#include "File.h"
//#include "INode.h"
//#include "Address.h"

enum State {
	DIR_NOT_EXIST = 0,
	SUCCESS = 1,
	NO_FILENAME = 2,
	FILE_EXISTS = 3,
	LENGTH_EXCEED = 4,
	DIRECTORY_EXCEED = 5,
	NO_ENOUGH_SPACE = 6,
	NO_SUCH_FILE = 7,
	NO_DIRNAME = 8,
	NO_SUCH_DIR = 9,
	DIR_NOT_EMPTY = 10,
	CAN_NOT_DELETE_TEMP_DIR = 11,
	DIR_EXISTS = 12
};

struct Superblock {
	int systemsize;
	int blocksize;
	int blocknum;
	int address_length;
	int max_filename_size;
	int superblock_size;
	int inode_size;
	int inode_bitmap_size;
	int block_bitmap_size;
	int inode_block_size;
};

struct File {
	int inode_id;
	char filename[MAX_FILENAME_SIZE];
};

class INode {
public:
	static const int NUM_DIRECT_ADDRESS = 10;
	static const int NUM_INDIRECT_ADDRESS = 1;

public:
	int id;
	int fsize;
	int fmode; // 1ΪĿ¼��0Ϊ�ļ�
	int count; // ��ǰ�����File������Ŀ
	int mcount; // ��Inode�������ʷ���File������Ŀ
	time_t ctime;
	int dir_address[NUM_DIRECT_ADDRESS];
	int indir_address[NUM_INDIRECT_ADDRESS];

public:
	INode();
	~INode();
	void clear();

};

class Address {
public:
	unsigned char addressByte[3];

public:
	Address();
	~Address();
	int getblockID();
	void setblockID(int id);
	int getOffset();
	void setOffset(int offset);
};

class System {

public:
	char current_Path[MAX_PATH];
	std::FILE *fp;
	bool flag;
	Superblock superblock;
	INode root_Inode;
	INode current_Inode;

public:
	System();
	~System();
	void initialize();//��ʼ��buffer�ã���Ҫ�����ǽ��ļ������ݶ���superblock�С����ǵ�һ�ε��ã�����ʼ��OS�Ļ����ṹ
	void copyright();//������ǵ�ѧ��
	State createFile(std::string fileName, int fileSize);
	State deleteFile(std::string fileName);
	State createDir(std::string dirName);
	State deleteDir(std::string dirName);
	State changeDir(std::string path);
	void dir();
	State cp(std::string file1, std::string file2);
	void sum();
	State cat(std::string fileName);
	void tip();//��������ǰ����һ����ʾ����ʾ����ϵͳ���֣�
	void help();//��ʾ��OS�����
	int numberOfAvailableBlock();//��ʾ�ж��ٿ���ʹ�õĿ�
	int findAvailableInode();//����һ���ɱ�ʹ�õ�I-node
	int findAvailableBlock();//����һ�ſɱ�ʹ�õĿ�
	void modifyInodeBitmap(int pos);//��Inode��״̬�ı䣺����<->����
	void modifyBlockBitmap(int pos);//�����״̬�ı䣺����<->����
	void writeInode(int pos, INode inode);//��Inodeд��ָ���ĵ�ַ��д��ĵ�ַһ��ΪInode.id
	void writeFileToDentry(File file, INode inode);//��File����Inode�¡����������´������ļ�����Ŀ¼��Ӧfileʵ����inode�¡�
	INode readInode(int pos);//��ȡ�ض�id��inode
	INode findNextInode(INode inode, std::string fileName, bool &canFind);//�ҵ�Ŀ¼�µĶ�Ӧ�ļ�����Inode
	void writeRandomStringToBlock(int blockid);//���ļ���д������ַ����������ν���ļ��������̣�
	void writeAddressToBlock(Address address, int blockid, int offset);//��ֱ�ӿ��е��ã�����ĵ�ַд���ֱ�ӿ���
	void deleteFileFromDentry(INode inode, std::string fileName);
	void result(std::string command, State st);//��������ִ�н���Լ�������ʾ��
	int getSize(INode inode);
	INode goalInode(std::string path, std::string &fileName,State &s);
	void deleteEmptyDir(INode inode, std::string dirName);
	void deleteFileUsingInode(INode inode);
	void deleteDirUsingInode(INode inode, std::string dirName);
};

#endif