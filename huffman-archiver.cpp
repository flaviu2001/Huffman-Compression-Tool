#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"

struct Code{
	unsigned code;
	string encoding;
};

void dfs(Node *root, Code &c, vector<Code> &codes)
{
	if (root->left == nullptr && root->right == nullptr){
		c.code = root->code;
		codes.push_back(c);
		return;
	}
	c.encoding += "0";
	dfs(root->left, c, codes);
	c.encoding.pop_back();
	c.encoding += "1";
	dfs(root->right, c, codes);
	c.encoding.pop_back();
}

int main(int argc, char **argv)
{
	if (argc != 3){
		cerr << "Needs two arguments.\n";
		exit(1);
	}
	int file_descriptor = open(argv[1], O_RDONLY); ropen_check(file_descriptor);
	unordered_map<unsigned, int> frequency_map;
	while (1){
		unsigned char buffer[buffer_size];
		int bytes_read = read(file_descriptor, buffer, buffer_size*sizeof(unsigned char)); read_check(bytes_read, 1);
		for (int i = 0; i < bytes_read; ++i)
			++frequency_map[buffer[i]];
		if (bytes_read < buffer_size)
			break;
	}
	close(file_descriptor);
	/* Read and built the frequency of characters. */
	auto comp = [](Node *n1, Node *n2){return *n1 > *n2;};
	priority_queue<Node*, vector<Node*>, decltype(comp) > pq(comp);
	if (frequency_map.size() == 0){
		frequency_map['a'] = 1;
		frequency_map['b'] = 1;
	}else if (frequency_map.size() == 1){
		if (frequency_map.find('a') == frequency_map.end())
			frequency_map['a'] = 1;
		else frequency_map['b'] = 1;
	}
	++frequency_map[eof_code];
	for (auto x : frequency_map){
		Node *n = new Node;
		*n = {nullptr, nullptr, x.first, x.second};
		pq.push(n);
	}
	while(pq.size() > 1){
		Node *n1 = pq.top();
		pq.pop();
		Node *n2 = pq.top();
		pq.pop();
		Node *new_node = new Node;
		*new_node = {n1, n2, 0, n1->frequency+n2->frequency};
		pq.push(new_node);
	}
	Node *root = pq.top();
	vector<Code> codes;
	Code code;
	dfs(root, code, codes);
	dealloc(root);
	for (auto &x : codes)
		if (x.code == eof_code)
			swap(x, codes.back());
	unordered_map<unsigned char, string> mapping;
	/* Built the huffman tree and codes for each character */
	string eof_encoding;
	for (auto x : codes)
		if (x.code != eof_code)
			mapping[x.code] = x.encoding;
		else eof_encoding = x.encoding;
	int write_descriptor = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); wopen_check(write_descriptor);
	int codes_size = codes.size();
	write_check(write(write_descriptor, &codes_size, sizeof(int)), 1);
	for (auto code : codes){
		reverse(code.encoding.begin(), code.encoding.end());
		if (code.code != eof_code)
			write_check(write(write_descriptor, &code.code, sizeof(unsigned char)), 2);
		int need_bytes = (code.encoding.length()+7)/8;
		assert(code.encoding.length() < 256);
		unsigned char bits = code.encoding.length();	//Hope I won't use more than 255 bits
		write_check(write(write_descriptor, &bits, sizeof(unsigned char)), 3);
		int pos_word = 0, pos_array = 0;
		unsigned char array[need_bytes];
		for (int i = 0; i < need_bytes; ++i)
			array[i] = 0;
		for (auto x : code.encoding){
			if (x == '1')
				array[pos_array] |= (1<<pos_word);
			++pos_word;
			if (pos_word == 8*sizeof(unsigned char)){
				pos_word = 0;
				++pos_array;
			}
		}
		write_check(write(write_descriptor, array, need_bytes*sizeof(unsigned char)), 4);
	}
	/* Wrote the compression details, mapping for each char */
	int parse_third_time = open(argv[1], O_RDONLY); ropen_check(parse_third_time);
	unsigned char buffer_bytes[buffer_size];
	int pos = 0, pos_array = 0;
	for (int i = 0; i < buffer_size; ++i)
		buffer_bytes[i] = 0;
	while (1){
		unsigned char buffer[buffer_size];
		int bytes_read = read(parse_third_time, buffer, buffer_size*sizeof(unsigned char)); read_check(bytes_read, 3);
		for (int i = 0; i < bytes_read; ++i){
			for (auto it = mapping[buffer[i]].begin(); it != mapping[buffer[i]].end(); ++it){
				if (*it == '1')
					buffer_bytes[pos_array] |= (1<<pos);
				++pos;
				if (pos == 8*sizeof(unsigned char)){
					pos = 0;
					++pos_array;
					if (pos_array == buffer_size){
						write_check(write(write_descriptor, buffer_bytes, buffer_size*sizeof(unsigned char)), 6);
						for (int i = 0; i < buffer_size; ++i)
							buffer_bytes[i] = 0;
						pos_array = 0;
					}
				}
			}
		}
		if (bytes_read < buffer_size)
			break;
	}
	for (auto it = eof_encoding.begin(); it != eof_encoding.end(); ++it){
		if (*it == '1')
			buffer_bytes[pos_array] |= (1<<pos);
		++pos;
		if (pos == 8*sizeof(unsigned char)){
			pos = 0;
			++pos_array;
			if (pos_array == buffer_size){
				write_check(write(write_descriptor, buffer_bytes, buffer_size*sizeof(unsigned char)), 6);
				for (int i = 0; i < buffer_size; ++i)
					buffer_bytes[i] = 0;
				pos_array = 0;
			}
		}
	}
	// Writing last part which is unfilled buffer. Kind of like flushing.
	write_check(write(write_descriptor, buffer_bytes, pos_array*sizeof(unsigned char)), 7, true);
	if (pos != 0)
		write_check(write(write_descriptor, &buffer_bytes[pos_array], sizeof(unsigned char)), 8);
	close(parse_third_time);
	close(write_descriptor);
	/* Wrote the code of each char */
	return 0;
}