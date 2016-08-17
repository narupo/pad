/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "map.h"

struct node {
	long hash;
	struct cap_string *key;
	struct cap_string *val;
	struct node *left;
	struct node *right;
};

static void
nodepush(struct node *self, struct node *nd) {
	if (nd->hash < self->hash) {
		// To left
		if (!self->left) {
			self->left = nd;
		} else {
			nodepush(self->left, nd);
		}
	} else {
		// To right
		if (!self->right) {
			self->right = nd;
		} else {
			nodepush(self->right, nd);
		}
	}
}

static void
nodedel(struct node *self) {
	if (self) {
		cap_strdel(self->val);
		cap_strdel(self->key);
		free(self);
	}
}

static void
nodedeltree(struct node *self) {
	if (self->left) {
		nodedeltree(self->left);
	}
	if (self->right) {
		nodedeltree(self->right);		
	}
	nodedel(self);
}

static struct node *
nodenew(void) {
	struct node *self = calloc(1, sizeof (struct node));
	return self;
}

static void
nodemovekey(struct node *self, struct cap_string *key) {
	self->hash = cap_hashl(cap_strgetc(key));
	if (self->key) {
		cap_strdel(self->key);
	}
	self->key = key;
}

static void
nodemoveval(struct node *self, struct cap_string *val) {
	if (self->val) {
		cap_strdel(self->val);
	}
	self->val = val;
}

static void
nodedump(struct node *self, FILE *fout, int sym) {
	fprintf(fout, "[%c] node[%p] hash[%ld] key[%s] val[%s]\n", sym, self, self->hash, cap_strgetc(self->key), cap_strgetc(self->val));
}

static void
nodedumptree(struct node *self, FILE *fout, int sym, int dep) {
	for (int i = 0; i < dep; ++i) {
		fputc('-', fout);
	}
	nodedump(self, fout, sym);
	if (self->left) {
		nodedumptree(self->left, fout, 'L', dep+1);
	}
	if (self->right) {
		nodedumptree(self->right, fout, 'R', dep+1);
	}
}

static struct node *
nodefindhash(struct node *self, long hash) {
	if (!self) {
		return NULL;
	}

	if (self->hash == hash) {
		return self;
	} else if (hash < self->hash) {
		return nodefindhash(self->left, hash);
	} else {
		return nodefindhash(self->right, hash);
	}
}

static struct node *
nodefindkey(struct node *self, const char *key) {
	return nodefindhash(self, cap_hashl(key));
}

/**********
* cap map *
**********/

struct cap_map {
	struct node *root;
};

void
cap_mapdel(struct cap_map *self) {
	if (self) {
		nodedeltree(self->root);
		free(self);
	}
}

struct cap_map *
cap_mapnew(void) {
	struct cap_map *self = calloc(1, sizeof(struct cap_map));
	if (!self) {
		return NULL;
	}
	return self;
}

struct cap_map *
cap_mapset(struct cap_map *self, const char *key, const char *val) {
	struct node *fnd = nodefindkey(self->root, key);
	if (!fnd) {
		struct node *nd = nodenew();
		struct cap_string *strkey = cap_strnew();
		struct cap_string *strval = cap_strnew();
		cap_strset(strkey, key);
		cap_strset(strval, val);
		nodemovekey(nd, strkey);
		nodemoveval(nd, strval);
		if (!self->root) {
			self->root = nd;
		} else {
			nodepush(self->root, nd);
		}
	} else {
		cap_strset(fnd->val, val);
	}

	return self;
}

const char *
cap_mapgetc(struct cap_map *self, const char *key) {
	struct node *fnd = nodefindkey(self->root, key);
	if (!fnd) {
		return NULL;
	}

	return cap_strgetc(fnd->val);
}

char *
cap_mapgetcp(struct cap_map *self, const char *key) {
	return strdup(cap_mapgetc(self, key));
}

#if 0
int
main(int argc, char *argv[]) {
	struct cap_map *map = cap_mapnew();
	cap_mapset(map, "home", "/tmp");
	cap_mapset(map, "cd", "/var/tmp");
	const char *fnd = cap_mapgetc(map, "home");
	printf("home[%s]\n", fnd);
	cap_mapdel(map);
	return 0;
}
#endif
