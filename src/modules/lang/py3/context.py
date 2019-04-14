class Context:

    def __init__(self):
        self.buffer = ''
        self.imported_alias = False
        self.imported_config = False
        self.alias_map = {}
        self.config_map = {}
        self.syms = {} # symbol table
        self.last_expr_val = 0
