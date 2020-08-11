#ifndef COMMON_H
#define COMMON_H

#include <bits/stdc++.h>

using namespace std;

const int buffer_size = 4096;
const unsigned eof_code = 404; //sorry

struct Node{
	Node *left = nullptr;
	Node *right = nullptr;
	unsigned code = 0;
	int frequency = 0;

	bool operator>(const Node &obj)const{
		return frequency > obj.frequency;
	}
};

void ropen_check(int fd)
{
	if (fd == -1){
		cerr << "Failed to open file (perhaps it doesn't exist or incorrect permissions)\n";
		exit(1);
	}
}

void wopen_check(int fd)
{
	if (fd == -1){
		cerr << "Failed to open archive file.\n";
		exit(1);
	}
}

void read_check(int bytes, int err)
{
	if (bytes < 0){
		cerr << "Bad read: " << err << "\n";
		exit(1);
	}
}

void write_check(int bytes, int err, bool allow_zero = false)
{
	int allowed = 0;
	if (allow_zero)
		allowed = -1;
	if (bytes <= allowed){
		cerr << "Bad write: " << err << "\n";
		exit(1);
	}
}

void dealloc(Node *root)
{
	if (root == nullptr)
		return;
	dealloc(root->left);
	dealloc(root->right);
	delete root;
}

#endif //COMMON_H