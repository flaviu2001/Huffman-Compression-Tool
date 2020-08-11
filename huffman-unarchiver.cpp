#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

void add(Node *&node, string code, unsigned int init_value, int pos = 0)
{
	if (node == nullptr)
		node = new Node;
	if (pos == (int)code.length()){
		node->code = init_value;
		return;
	}
	if (code[pos] == '0')
		add(node->left, code, init_value, pos+1);
	else add(node->right, code, init_value, pos+1);
}

int main(int argc, char **argv)
{
	if (argc != 3){
		cerr << "Needs two arguments.\n";
		exit(1);
	}
	int read_descriptor = open(argv[1], O_RDONLY); ropen_check(read_descriptor);
	int codes_size;
	string eof_encoding;
	read_check(read(read_descriptor, &codes_size, sizeof(int)), 1);
	Node *root = nullptr;	//Don't forget to dealloc it
	for (int i = 0; i < codes_size; ++i){
		unsigned int code = 0;
		unsigned char bits;
		if (i+1 != codes_size)
			read_check(read(read_descriptor, &code, sizeof(unsigned char)), 2);
		else code = eof_code;
		read_check(read(read_descriptor, &bits, sizeof(unsigned char)), 3);
		string s;
		int need_bytes = (bits+7)/8;
		unsigned char array[need_bytes];
		read_check(read(read_descriptor, array, need_bytes*sizeof(unsigned char)), 4);
		int pos_array = 0, pos = 0;
		for (int i = 0; i < bits; ++i){
			if (array[pos_array] & (1<<pos))
				s += "1";
			else s += "0";
			++pos;
			if (pos == 8*sizeof(unsigned char)){
				pos = 0;
				++pos_array;
			}
		}
		reverse(s.begin(), s.end());
		add(root, s, code);
	}
	int write_descriptor = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); wopen_check(write_descriptor);
	unsigned char c[buffer_size], bitbuffer[buffer_size];
	for (int i = 0; i < buffer_size; ++i)
		c[i] = 0;
	int pos_c = 0;
	Node *where = root;
	bool finished = false;
	while(!finished){
		int bytes_read = read(read_descriptor, bitbuffer, buffer_size*sizeof(unsigned char)); read_check(bytes_read, 6);
		for (int i = 0; i < bytes_read && !finished; ++i){
			for (int j = 0; j < (int)(8*sizeof(unsigned char)) && !finished; ++j){
				if (bitbuffer[i] & (1<<j))
					where = where->right;
				else where = where->left;
				if (where->left == nullptr && where->right == nullptr){
					if (where->code == eof_code) {
						finished = true;
					}else {
						c[pos_c++] = where->code;
						if (pos_c == buffer_size){
							pos_c = 0;
							write_check(write(write_descriptor, c, buffer_size*sizeof(unsigned char)), 1);
						}
						where = root;
					}
				}
			}

		}
		if (bytes_read < buffer_size)
			break;
	}
	write_check(write(write_descriptor, c, pos_c*sizeof(unsigned char)), 2, true);
	dealloc(root);
	close(read_descriptor);
	close(write_descriptor);
	return 0;
}