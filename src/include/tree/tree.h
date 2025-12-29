#ifndef TREE_H
#define TREE_H
#include <stdint.h>

struct tree {
	struct node* root;
	uint32_t size;

};

void tree_init(struct tree* tree);



#endif //TREE_H
