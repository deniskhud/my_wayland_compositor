#ifndef NODE_H
#define NODE_H
#include <stdint.h>

enum node_type {
	NODE_LEAF, NODE_SPLIT
};

enum split_type {
	SPLIT_VERTICAL,
};
/*окна слева идут в левое поддерево, правые в правое;
 *верхние в левое, нижние в правое
 *также и с сплит узлами
 */
struct node {
	struct client_xdg_toplevel* toplevel;
	uint32_t x, y, height, width;

	enum split_type split;
	enum node_type type;
	struct node* left, *right, *parent;

};

struct node* node_create_leaf(struct client_xdg_toplevel* toplevel);
struct node* node_create_split(enum split_type split, struct node* left, struct node* right);
void node_delete(struct node* node);

void arrange_windows();

void node_destroy();


#endif //NODE_H
