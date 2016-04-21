#include "strtree.h"

/********************
* Tree and TreeNode *
********************/

typedef struct TreeNode TreeNode;
typedef struct Tree Tree;

struct TreeNode {
	int key;
	TreeNode* parent;
	TreeNode* left;
	TreeNode* right;
};

struct Tree {
	TreeNode* root;
};

/**************************
* TreeNode delete and new *
**************************/

void
treenode_delete(TreeNode* self) {
	if (self) {
		if (self->left) {
			treenode_delete(self->left);
		}
		if (self->right) {
			treenode_delete(self->right);
		}
		free(self);
	}
}

TreeNode*
treenode_new(void) {
	TreeNode* self = (TreeNode*) calloc(1, sizeof(TreeNode));
	if (!self) {
		warn("Failed to construct TreeNode");
		return NULL;
	}

	return self;
}

TreeNode*
treenode_new_from_key(int key) {
	TreeNode* self = (TreeNode*) calloc(1, sizeof(TreeNode));
	if (!self) {
		warn("Failed to construct TreeNode");
		return NULL;
	}

	self->key = key;

	return self;
}

/*******************
* TreeNode writter *
*******************/

void
treenode_display(TreeNode* self, FILE* stream, int depth, int pad, const char* head) {
	if (!self) {
		return;
	}

	for (int i = 0; i < depth; ++i) {
		fputc(pad, stream);
	}

	fprintf(stream, "%s[%d]", head, self->key);
	fprintf(stream, " P[%d]", (self->parent ? self->parent->key : -1));
	fprintf(stream, " L[%d]", (self->left ? self->left->key : -1));
	fprintf(stream, " R[%d]", (self->right ? self->right->key : -1));
	fprintf(stream, "\n");

	if (self->left) {
		treenode_display(self->left, stream, depth + 1, pad, "L");
	}

	if (self->right) {
		treenode_display(self->right, stream, depth + 1, pad, "R");
	}
}

/******************
* TreeNode getter *
******************/

TreeNode*
treenode_minimum(TreeNode* node) {
	if (!node) {
		return NULL;
	}

	for (; node->left != NULL; ) {
		node = node->left;
	}

	return node;
}

TreeNode*
treenode_find(TreeNode* self, int key) {
	if (!self) {
		// Not found
		return NULL;
	}

	if (key == self->key) {
		// Found
		return self;
	}

	if (key < self->key) {
		return treenode_find(self->left, key);
	} else {
		return treenode_find(self->right, key);
	}
}

/******************
* TreeNode setter *
******************/

TreeNode*
treenode_insert(TreeNode* node, int key) {
	if (!node) {
		return NULL;
	}

	if (key < node->key) {
		// Insert to left
		if (!node->left) {
			node->left = treenode_new_from_key(key);
			node->left->parent = node;
			return node->left;
		} else {
			return treenode_insert(node->left, key);
		}
	} else {
		// Insert to right
		if (!node->right) {
			node->right = treenode_new_from_key(key);
			node->right->parent = node;
			return node->right;
		} else {
			return treenode_insert(node->right, key);
		}
	}

	die("Impossible insert key [%d]", key);
	return node; // Impossible
}

/**********************
* Tree delete and new *
**********************/

void
tree_delete(Tree* self) {
	if (self) {
		treenode_delete(self->root);
		free(self);
	}
}

Tree*
tree_new(void) {
	Tree* self = (Tree*) calloc(1, sizeof(Tree));
	if (!self) {
		warn("Failed to construct Tree");
		return NULL;
	}

	return self;
}

/***************
* Tree writter *
***************/

void
tree_display(Tree const* self, FILE* stream) {
	treenode_display(self->root, stream, 0, '.', "r");
}

/**************
* Tree setter *
**************/

Tree*
tree_insert(Tree* self, int key) {
	if (!self->root) {
		TreeNode* node = treenode_new_from_key(key);
		if (!node) {
			warn("Failed to construct TreeNode");
			return NULL;
		}

		self->root = node;
		
		return self;
	}

	if (!treenode_insert(self->root, key)) {
		warn("Failed to insert TreeNode");
		return NULL;
	}

	return self;
}

static void
tree_detach(Tree* tree, TreeNode* u, TreeNode* v) {
	if (u->parent == NULL) {
		tree->root = v;
	} else if (u == u->parent->left) {
		u->parent->left = v;
	} else {
		u->parent->right = v;
	}

	if (v != NULL) {
		v->parent = u->parent;
	}
}

TreeNode*
tree_remove(Tree* tree, int key) {
	if (!tree->root) {
		// Tree is empty
		return NULL;
	}

	TreeNode* z = treenode_find(tree->root, key);
	if (!z) {
		warn("Not found key \"%s\"", key);
		return NULL;
	}

	if (z->left == NULL) {
		tree_detach(tree, z, z->right);
	} else if (z->right == NULL) {
		tree_detach(tree, z, z->left);
	} else {
		TreeNode* y;
		y = treenode_minimum(tree, z->right);
		if (!y) {
			warn("Failed to find minimum");
			return NULL;
		}

		if (y->parent != z) {
			tree_detach(tree, y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		tree_detach(tree, z, y);
		y->left = z->left;
		y->left->parent = y;
	}

	z->parent = z->left = z->right = NULL;
	treenode_delete(z);

	return z;
}

/*************************
* Tree and TreeNode test *
*************************/

#if defined(TEST_TREE)
int
test_insert(int argc, char* argv[]) {
	Tree* tree = tree_new();

	tree_insert(tree, 100); // Root

	tree_insert(tree, 10); // Left
	tree_insert(tree, 200); // Right
	
	tree_insert(tree, 300);
	tree_insert(tree, 400);
	tree_insert(tree, 50);
	tree_insert(tree, 30);
	tree_insert(tree, 350);
	tree_insert(tree, 450);

	tree_display(tree, stderr);

	tree_delete(tree);
	return 0;
}

int
test_remove(int argc, char* argv[]) {
	Tree* tree = tree_new();

	tree_insert(tree, 100);

	tree_insert(tree, 50);
	tree_insert(tree, 150);

	tree_insert(tree, 25);

	tree_insert(tree, 70);
	tree_insert(tree, 60);
	tree_insert(tree, 80);

	tree_display(tree, stderr);
	puts("----");

	int rmkey = 50;
	printf("remove key[%d]\n", rmkey);
	tree_remove(tree, rmkey);

	tree_display(tree, stderr);

	tree_delete(tree);
	return 0;
}

int
main(int argc, char* argv[]) {
	// return test_insert(argc, argv);
	return test_remove(argc, argv);
}
#endif

