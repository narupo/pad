#include "config.h"

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

static bool
iskey(int c) {
	return isalpha(c) || c == '_';
}

static void
parse(struct node **root, FILE *fin) {
	int m = 0;
	struct cap_string *key = cap_strnew();
	struct cap_string *val = cap_strnew();

	for (int c; (c = fgetc(fin)) != EOF; ) {
		switch (m) {
		case 0: // First
			if (c == '#') {
				m = 100;
			} else if (iskey(c)) {
				m = 10;
				cap_strpushb(key, c);
			}
			break;
		case 10: // Read name
			if (isblank(c) || c == '=') {
				m = 20;
			} else {
				cap_strpushb(key, c);
			}
			break;
		case 20: // Ignore blanks and equals
			if (isblank(c) || c == '=') {
				;
			} else if (c == '"') {
				// printf("key[%s]\n", cap_strgetc(key));
				m = 30;
			} else {
				// printf("key[%s]\n", cap_strgetc(key));
				cap_strpushb(val, c);
				m = 40;
			}
			break;
		case 30: // Read string of value
			if (c == '"') {
				// printf("val[%s]\n", cap_strgetc(val));
				struct node *nd = nodenew();
				nodemovekey(nd, key);
				nodemoveval(nd, val);

				if (!(*root)) {
					*root = nd;
				} else {
					nodepush(*root, nd);
				}

				key = cap_strnew();
				val = cap_strnew();
				m = 100;
			} else {
				cap_strpushb(val, c);
			}
			break;
		case 40: // Read number of value
			break;
		case 100: // Waiting newline
			if (c == '\n') {
				m = 0;
			}
			break;
		}
	}

	cap_strdel(key);
	cap_strdel(val);
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





struct cap_config {
	struct node *root;
};

void
cap_confdel(struct cap_config *self) {
	if (self) {
		nodedeltree(self->root);
		free(self);
	}
}

struct cap_config *
cap_confnew(void) {
	struct cap_config *self = calloc(1, sizeof(struct cap_config));
	if (!self) {
		return NULL;
	}
	return self;
}

struct cap_config *
cap_confnewfile(const char *fname) {
	struct cap_config *self = cap_confnew();
	if (!cap_confloadfile(self, fname)) {
		cap_confdel(self);
		return NULL;
	}
	return self;
}

struct cap_config *
cap_confnewload(void) {
	const char *confpath = getenv("CAP_CONFPATH");
	if (!confpath) {
		return NULL;
	}
	return cap_confnewfile(confpath);
}

bool
cap_confloadfile(struct cap_config *self, const char *fname) {
	FILE *fin = fopen(fname, "rb");
	if (!fin) {
		return false;
	}
	if (self->root) {
		nodedeltree(self->root);
		self->root = NULL;
	}
	parse(&self->root, fin);
	fclose(fin);
	return true;
}

const char *
cap_confgetc(const struct cap_config *self, const char *key) {
	struct node *fnd = nodefindkey(self->root, key);
	if (!fnd) {
		return NULL;
	}

	return cap_strgetc(fnd->val);
}

char *
cap_confgetcp(const struct cap_config *self, const char *key) {
	struct node *fnd = nodefindkey(self->root, key);
	if (!fnd) {
		return NULL;
	}

	return strdup(cap_strgetc(fnd->val));
}

void
cap_confdump(const struct cap_config *self, FILE *fout) {
	nodedumptree(self->root, fout, 'r', 0);
}

#if 0
int main(int argc, char *argv[]) {
	struct cap_config *conf = cap_confnew();
	cap_confloadfile(conf, argv[1]);
	cap_confdump(conf, stderr);
	const char *fnd = cap_confgetc(conf, "editor");
	fprintf(stderr, "Found[%s]\n", fnd);
	cap_confdel(conf);
	return 0;
}
#endif