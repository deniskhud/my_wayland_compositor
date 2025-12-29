#include "include/tree/node.h"

#include "include/clients/client.h"

struct node* node_create_leaf(struct client_xdg_toplevel* toplevel) {
	struct node* node = calloc(1, sizeof(struct node));
	node->type = NODE_LEAF;
	node->toplevel = toplevel;
	node->x = toplevel->box->x;
	node->y = toplevel->box->y;
	node->width = toplevel->box->width;
	node->height = toplevel->box->height;

	return node;
}

struct node* node_create_split(enum split_type split,struct node* left, struct node* right) {
	if (left == NULL || right == NULL) {
		return NULL;
	}
	struct node* node = calloc(1, sizeof(struct node));
	node->type = NODE_SPLIT;
	node->split = split;

	node->left = left;
	node->right = right;

	left->parent = node;
	right->parent = node;
	return node;
}

void node_delete(struct node* node) {
	free(node);
}


void replace_parent(struct node* old, struct node* new) {
	new->parent = old->parent;

	if (!old->parent) {
		return;
	}
	if (old->parent->left == old) {
		old->parent->left = new;
	}
	if (old->parent->right == old) {
		old->parent->right = new;
	}
}

