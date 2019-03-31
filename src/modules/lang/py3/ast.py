from stream import Stream
from tokens import Token
from nodes import *
from context import Context

'''
    BNF

    program ::= ( code-block | text-block ) program 
    code-block ::= '{@' formula '@}'
    text-block ::= .*
    formula ::= ( import | caller ), formula
    import ::= 'import' identifier
    caller ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    identifier ::= ( [a-z] | [0-9] | _ )+ 
'''

class AST:
    class ModuleError(RuntimeError):
        pass

    class SyntaxError(SyntaxError):
        pass

    class ImportError(ImportError):
        pass

    class NameError(NameError):
        pass

    def parse(self, tokens, debug=False):
        self.debug_parse = debug
        self.strm = Stream(tokens)
        self.root = self.program()

    def traverse(self):
        self.context = Context()
        self._traverse(self.root)
        return self.context

    def _traverse(self, node):
        if node is None:
            return

        if isinstance(node, BinNode):
            self._traverse(node.lhs)
            self._traverse(node.rhs)
        elif isinstance(node, CodeBlockNode):
            self._traverse(node.formula)
        elif isinstance(node, TextBlockNode):
            self.context.buffer += node.text
        elif isinstance(node, FormulaNode):
            if node.import_:
                self._traverse(node.import_)
            elif node.caller:
                self._traverse(node.caller)
            self._traverse(node.formula)
        elif isinstance(node, ImportNode):
            self.traverse_import(node)
        elif isinstance(node, CallerNode):
            self.traverse_caller(node)
        else:
            raise AST.ModuleError('impossible. not supported node', type(node))

    def traverse_import(self, node):
        if node.identifier == 'alias':
            self.context.imported_alias = True 
        elif node.identifier == 'config':
            self.context.imported_config = True
        else:
            raise AST.ImportError('can not import package "%s"' % node.identifier)

    def traverse_caller(self, node):
        first = node.identifiers[0]
        if first == 'alias':
            self.call_alias_package(node)
        elif first == 'config':
            self.call_config_package(node)
        else:
            raise AST.NameError('"%s" is not defined' % first)

    def call_alias_package(self, node):
        if not self.context.imported_alias:
            raise AST.ImportError('alias is not imported')

        method = node.identifiers[1]
        if method == 'set':
            if len(node.args) != 2:
                AST.SyntaxError('alias.set need two arguments')
            name = node.args[0]
            cmd = node.args[1]
            self.context.alias_map[name] = cmd

    def call_config_package(self, node):
        if not self.context.imported_config:
            raise AST.ImportError('config is not imported')

        method = node.identifiers[1]
        if method == 'set':
            if len(node.args) != 2:
                AST.SyntaxError('config.set need two arguments')
            name = node.args[0]
            cmd = node.args[1]
            self.context.config_map[name] = cmd

    def show_parse(self, name):
        if self.debug_parse:
            t = self.strm.cur()
            print(name + ': ' + str(t))

    def program(self):
        self.show_parse('program')
        if self.strm.eof():
            return None

        node = BinNode()
        node.lhs = self.code_block()
        if node.lhs is None:
            node.lhs = self.text_block()
            if node.lhs is None:
                raise AST.SyntaxError('not supported token: %s' % str(self.strm.cur()))

        node.rhs = self.program()
        return node

    def code_block(self):
        self.show_parse('code_block')
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != 'lbraceat':
            self.strm.prev()
            return None

        node = CodeBlockNode()
        node.formula = self.formula()

        t = self.strm.get()
        if t == Stream.EOF or t.kind != 'rbraceat':
            raise AST.SyntaxError('not found "@}" in code block')

        return node

    def text_block(self):
        self.show_parse('text_block')
        if self.strm.eof():
            return None
        
        t = self.strm.get()
        if t.kind != 'text-block':
            self.strm.prev()
            return None

        node = TextBlockNode()
        node.text = t.value

        return node

    def formula(self):
        self.show_parse('formula')
        if self.strm.eof():
            return None

        if self.strm.cur().kind == 'rbraceat':
            return None
        
        node = FormulaNode()
        node.import_ = self.import_()
        if node.import_ is None:
            node.caller = self.caller()
            if not self.strm.eof() and node.caller is None:
                return None

        node.formula = self.formula()
        return node

    def import_(self):
        self.show_parse('import')
        if self.strm.eof():
            return None
        
        node = ImportNode()
        t = self.strm.get()
        if t.kind != 'import':
            self.strm.prev()
            return None

        t = self.strm.get()
        if t.kind != 'identifier':
            raise AST.SyntaxError('invalid import statement')

        node.identifier = t.value
        return node

    def caller(self):
        """
        ここがしんどい
        もっと抽象化するべき
        """
        self.show_parse('caller')
        if self.strm.eof():
            return None
        
        node = CallerNode()
        save_index = self.strm.index

        while not self.strm.eof():
            t = self.strm.get()
            if t.kind == 'identifier':
                node.identifiers.append(t.value)
            elif t.kind == 'lparen':
                break

        if not len(node.identifiers):
            self.strm.index = save_index
            return None

        while not self.strm.eof():
            t = self.strm.get()
            if t.kind == 'string':
                node.args.append(t.value)
            elif t.kind == 'rparen':
                break

        return node

