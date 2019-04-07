from stream import Stream
from tokens import Token
from nodes import *
from context import Context

'''
    BNF 0.0.0

    program ::= ( code-block | text-block ) program 
    code-block ::= '{@' formula '@}'
    text-block ::= .*
    formula ::= ( import | caller ), formula
    import ::= 'import' identifier
    caller ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    identifier ::= ( [a-z] | [0-9] | _ )+ 

    2019-04-01 03:23:29 曇のち雪 1~10°
    =================================
    BNF 0.1.0

    + は新規追加したところ
    ^ は更新

    program ::= block, program 
    + block ::= text-block | code-block | ref-block 
    text-block ::= .*
    code-block ::= '{@' {formula}* '@}'
    + ref-block ::= '{{' identifier '}}'
    ^ formula ::= ( expr | import-stmt | caller-stmt ), formula
    + expr ::= assign-expr
    + assign-expr ::= assignable-operand assign-operator operand
    + assign-operator ::= '='
    + assignable-operand ::= identifier
    + operand ::= identifier | string
    import-stmt ::= 'import' identifier
    caller-stmt ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    identifier ::= ( [a-z] | [0-9] | _ )+ 

    2019-04-02 05:37:52 曇のち雪
    ===========================
    BNF 0.2.0

    + は新規追加したところ
    ^ は更新

    ^ block ::= ( text-block | code-block | ref-block ), block
    text-block ::= .*
    code-block ::= '{@' {formula}* '@}'
    ref-block ::= '{{' identifier '}}'
    ^ formula ::= ( expr | if-stmt | import-stmt | caller-stmt ), ( formula | '@}' block '{@' )
    + if-stmt ::= 'if' expr ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    + elif-stmt ::= 'elif' expr ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    + else-stmt ::= 'else' ':' '@}'? ( block | formula ) '@}'? 'end'
    ^ expr ::= assign-expr | digit
    assign-expr ::= assignable-operand assign-operator operand
    assign-operator ::= '='
    assignable-operand ::= identifier
    operand ::= identifier | string
    import-stmt ::= 'import' identifier
    caller-stmt ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    + digit ::= [0-9]+
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

    class ReferenceError(RuntimeError):
        pass

    def parse(self, tokens, debug=False):
        self.debug_parse = debug
        self.strm = Stream(tokens)
        self.root = self.block(dep=0)

    def traverse(self, debug=False):
        self.debug_traverse = debug
        self.context = Context()
        self.last_expr_val = 0
        self._traverse(self.root, dep=0)
        return self.context

    def show_traverse(self, node, dep):
        if self.debug_traverse:
            print(('_'*dep) + str(dep) + ' ' + str(node).split('.')[1].split(' ')[0])

    def _traverse(self, node, dep):
        if node is None:
            return
        self.show_traverse(node, dep)

        if isinstance(node, BlockNode):
            self._traverse(node.text_block, dep=dep+1)
            self._traverse(node.code_block, dep=dep+1)
            self._traverse(node.ref_block, dep=dep+1)
            self._traverse(node.block, dep=dep+1)

        elif isinstance(node, TextBlockNode):
            self.context.buffer += node.text

        elif isinstance(node, CodeBlockNode):
            self._traverse(node.formula, dep=dep+1)

        elif isinstance(node, RefBlockNode):
            self.traverse_ref_block(node)

        elif isinstance(node, FormulaNode):
            if node.expr:
                self.last_expr_val = 0
                self._traverse(node.expr, dep=dep+1)
            elif node.import_:
                self._traverse(node.import_, dep=dep+1)
            elif node.caller:
                self._traverse(node.caller, dep=dep+1)
            elif node.if_:
                self._traverse(node.if_, dep=dep+1)

            if node.formula:
                self._traverse(node.formula, dep=dep+1)
            elif node.block:
                self._traverse(node.block, dep=dep+1)

        elif isinstance(node, IfNode):
            self.last_expr_val = 0
            self._traverse(node.expr, dep=dep+1)
            if self.last_expr_val:
                if node.formula:
                    self._traverse(node.formula, dep=dep+1)
                elif node.block:
                    self._traverse(node.block, dep=dep+1)
            else:
                if node.elif_:
                    self._traverse(node.elif_, dep=dep+1)
                elif node.else_:
                    self._traverse(node.else_, dep=dep+1)

        elif isinstance(node, ElseNode):
            if node.block:
                self._traverse(node.block, dep=dep+1)
            elif node.formula:
                self._traverse(node.formula, dep=dep+1)

        elif isinstance(node, ExprNode):
            if node.assign_expr:
                self._traverse(node.assign_expr, dep=dep+1)
            elif node.digit:
                self.last_expr_val = node.digit.value

        elif isinstance(node, AssignExprNode):
            self.traverse_assign_expr(node)

        elif isinstance(node, ImportNode):
            self.traverse_import(node)

        elif isinstance(node, CallerNode):
            self.traverse_caller(node)

        else:
            raise AST.ModuleError('impossible. not supported node', type(node))

    def traverse_assign_expr(self, node):
        symkey = node.assignable_operand.identifier
        if node.assign_operator.operator == '=':
            if node.operand.identifier != None:
                self.context.syms[symkey] = self.context.syms[node.operand.identifier]
                self.last_expr_val = self.context.syms[symkey]
            elif node.operand.string != None:
                self.context.syms[symkey] = node.operand.string
                self.last_expr_val = self.context.syms[symkey]
             
    def traverse_ref_block(self, node):
        if node.identifier in self.context.syms.keys():
            self.context.buffer += self.context.syms[node.identifier]

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

    def show_parse(self, name, dep):
        if self.debug_parse:
            t = self.strm.cur()
            print(dep, name + ': ' + str(t))

    def block(self, dep=0):
        self.show_parse('block', dep=dep)
        if self.strm.eof():
            return None
        
        t = self.strm.cur()
        if t.kind in ('rbraceat', 'end', 'if', 'elif', 'else'):
            return None

        node = BlockNode()

        node.code_block = self.code_block(dep=dep+1)
        if node.code_block is None:
            node.ref_block = self.ref_block(dep=dep+1)
            if node.ref_block is None:
                node.text_block = self.text_block(dep=dep+1)

        node.block = self.block(dep=dep+1)
        return node

    def ref_block(self, dep=0):
        self.show_parse('ref_block', dep=dep)
        if self.strm.eof():
            return None

        node = RefBlockNode()

        t = self.strm.get()
        if t.kind != 'ldbrace':
            self.strm.prev()
            return None

        t = self.strm.get()
        if t.kind == 'identifier':
            node.identifier = t.value
        elif t.kind == 'rdbrace':
            return node

        t = self.strm.get()
        if t == Stream.EOF or t.kind != 'rdbrace':
            raise AST.SyntaxError('not found "rdbrace"')

        return node

    def code_block(self, dep=0):
        self.show_parse('code_block', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != 'lbraceat':
            self.strm.prev()
            return None

        node = CodeBlockNode()
        node.formula = self.formula(dep=dep+1)

        t = self.strm.get()
        if t == Stream.EOF:
            pass
        elif t.kind in ('end'):
            return node
        elif t.kind in ('ldbrace', 'else', 'elif'):
            self.strm.prev()

        return node

    def text_block(self, dep=0):
        self.show_parse('text_block', dep=dep)
        if self.strm.eof():
            return None
        
        t = self.strm.get()
        if t.kind != 'text-block':
            self.strm.prev()
            return None

        node = TextBlockNode()
        node.text = t.value

        return node

    def formula(self, dep=0):
        self.show_parse('formula', dep=dep)
        if self.strm.eof():
            return None
        
        t = self.strm.cur()
        if t.kind in ('rbraceat', 'ldbrace', 'end', 'elif', 'else'):
            return None

        node = FormulaNode()
        t = self.strm.cur()
        if t.kind == 'import':
            node.import_ = self.import_(dep=dep+1)
        elif t.kind == 'if':
            node.if_ = self.if_(dep=dep+1)
        else:
            node.caller = self.caller(dep=dep+1)
            if node.caller is None:
                node.expr = self.expr(dep=dep+1)
                if not self.strm.eof() and node.expr is None:
                    return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node

        if t.kind == 'rbraceat' and self.strm.cur() != Stream.EOF:
            node.block = self.block(dep=dep+1)
            t = self.strm.get()
            if t == Stream.EOF:
                pass
            elif t.kind == 'lbraceat':
                pass
            else:
                self.strm.prev()
        else:
            self.strm.prev()
            node.formula = self.formula(dep=dep+1)

        return node

    def if_(self, dep=0, first_symbol='if'):
        self.show_parse('if', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != first_symbol:
            return None

        node = IfNode()
        node.expr = self.expr(dep=dep+1)
        if node.expr is None:
            raise AST.SyntaxError('invalid if statement. not found expression')

        t = self.strm.get()
        if t.kind != 'colon':
            raise AST.SyntaxError('invalid if statement. not found colon')

        t = self.strm.get()
        if t.kind == 'rbraceat': # @}
            node.block = self.block(dep=dep+1)
        else:
            self.strm.prev()
            node.formula = self.formula(dep=dep+1)

        t = self.strm.get()
        if t == Stream.EOF:
            return node

        if t.kind != 'lbraceat':
            self.strm.prev()

        t = self.strm.get()
        if t.kind == 'end':
            pass
        elif t.kind == 'elif':
            self.strm.prev()
            node.elif_ = self.if_(dep=dep+1, first_symbol='elif')
        elif t.kind == 'else':
            self.strm.prev()
            node.else_ = self.else_(dep=dep+1)
        else:
            self.strm.prev()

        return node

    def else_(self, dep=0):
        self.show_parse('else', dep=dep)
        if self.strm.eof():
            return None

        node = ElseNode()

        t = self.strm.get()
        if t.kind != 'else':
            raise AST.SyntaxError('invalid else statement. not found "else"')

        t = self.strm.get()
        if t.kind != 'colon':
            raise AST.SyntaxError('invalid else statement. not found colon')

        t = self.strm.get()
        if t.kind == 'rbraceat':
            node.block = self.block(dep=dep+1)
        else:
            self.strm.prev()
            node.formula = self.formula(dep=dep+1)

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        if t.kind != 'lbraceat':
            self.strm.prev()

        t = self.strm.get()
        if t.kind == 'end':
            pass
        else:
            self.strm.prev()

        return node

    def expr(self, dep=0):
        self.show_parse('expr', dep=dep)
        if self.strm.eof():
            return None
        
        node = ExprNode()

        t = self.strm.get()
        if t.kind == 'digit':
            self.strm.prev()
            node.digit = self.digit(dep=dep+1)
        else:
            self.strm.prev()
            node.assign_expr = self.assign_expr(dep=dep+1)

        return node

    def digit(self, dep=0):
        self.show_parse('digit', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != 'digit':
            raise AST.ModuleError('not found digit')

        node = DigitNode()
        node.value = t.value

        return node

    def assign_expr(self, dep=0):
        self.show_parse('assign_expr', dep=dep)
        if self.strm.eof():
            return None

        node = AssignExprNode()
        node.assignable_operand = self.assignable_operand(dep=dep+1)
        node.assign_operator = self.assign_operator(dep=dep+1)
        node.operand = self.operand(dep=dep+1)

        return node

    def operand(self, dep=0):
        self.show_parse('operand', dep=dep)
        if self.strm.eof():
            return None

        node = OperandNode()
        t = self.strm.get()
        if t.kind == 'identifier':
            node.identifier = t.value
        elif t.kind == 'string':
            node.string = t.value
        else:
            self.strm.prev()
            return None

        return node
        
    def assign_operator(self, dep=0):
        self.show_parse('assign_operator', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != 'operator':
            self.strm.prev()
            return None

        node = AssignOperatorNode()
        node.operator = t.value
        return node
        
    def assignable_operand(self, dep=0):
        self.show_parse('assignable_operand', dep=dep)
        if self.strm.eof():
            return None

        node = AssignableOperandNode()
        t = self.strm.get()
        if t.kind != 'identifier':
            self.strm.prev()
            return None

        node.identifier = t.value
        return node
                
    def import_(self, dep=0):
        self.show_parse('import', dep=dep)
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

    def caller(self, dep=0):
        """
        ここがしんどい
        もっと抽象化するべき
        """
        self.show_parse('caller', dep=dep)
        if self.strm.eof():
            return None
        
        # find lparen
        i = self.strm.index
        found = False
        while not self.strm.eof():
            t = self.strm.get()
            if t.kind == 'identifier':
                pass
            elif t.kind == 'operator' and t.value == '.':
                pass
            elif t.kind == 'lparen':
                found = True
                break
            else:
                break
        self.strm.index = i
        if not found:
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

