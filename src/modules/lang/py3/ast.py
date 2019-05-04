from stream import Stream
from tokens import Token
from nodes import *
from context import Context
import sys

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
    if 文の追加とそれに関連する修正など

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

    2019-04-02 05:37:52 曇のち雪
    ===========================
    BNF 0.2.1
    式の実装

    + は新規追加したところ
    ^ は更新

    block ::= ( text-block | code-block | ref-block ), block
    text-block ::= .*
    code-block ::= '{@' {formula}* '@}'
    ref-block ::= '{{' identifier '}}'
    ^ formula ::= ( expr | assign-expr | if-stmt | import-stmt | caller-stmt ), ( formula | '@}' block '{@' )
    ^ if-stmt ::= 'if' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    elif-stmt ::= 'elif' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    else-stmt ::= 'else' ':' '@}'? ( block | formula ) '@}'? 'end'
    + comparison ::= expr comp_op comparison | expr
    + cmp-op ::= '==' | '!=' | '<' | '>' | '<=' | '>='
    ^ expr ::= term '+' expr | term '-' + expr | term
    + term ::= factor '*' term | factor '/' term | factor
    + factor ::= digit | identifier | string | '(' expr ')'
    assign-expr ::= assignable-operand assign-operator operand
    assign-operator ::= '='
    assignable-operand ::= identifier
    ^ operand ::= expr | string
    import-stmt ::= 'import' identifier
    caller-stmt ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    digit ::= [0-9]+
    identifier ::= ( [a-z] | [0-9] | _ )+ 

    2019-04-14 11:20:12 曇のち雨
    ============================
    BNF 0.2.2
    lopts の実装

    + は新規追加したところ
    ^ は更新

    block: ( text-block | code-block | ref-block ), block
    text-block: .*
    code-block: '{@' {formula}* '@}'
    ^ ref-block: '{{' ( identifier | callable ) '}}'
    + callable: caller-list '(' args ')'
    + caller-list: identifier '.' caller-list | identifier
    + args: arg ',' args | arg
    + arg: digit | string | identifier
    formula: ( expr | assign-expr | if-stmt | import-stmt | caller-stmt ), ( formula | '@}' block '{@' )
    if-stmt: 'if' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    elif-stmt: 'elif' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    else-stmt: 'else' ':' '@}'? ( block | formula ) '@}'? 'end'
    comparison: expr comp_op comparison | expr
    cmp-op: '==' | '!=' | '<' | '>' | '<=' | '>='
    expr: term '+' expr | term '-' expr | term
    term: factor '*' term | factor '/' term | factor
    ^ factor: digit | identifier | string | callable | '(' expr ')'
    ^ assign-expr: assign-operand-lhs assign-operator assign-expr | assign-oeprand-rhs
    assign-operator: '='
    ^ assign-operand-lhs: identifier
    ^ assign-operand-rhs: expr | string | identifier | callable
    import-stmt: 'import' identifier
    caller-stmt: identifier ( '.' identifier )+ '(' args ')'
    args: string | ',' args
    string: '"' .* '"'
    digit: [0-9]+
    identifier: ( [a-z] | [0-9] | _ )+ 

    2019-04-14 11:20:12 曇のち雨
    ============================
    BNF 0.3.0
    for の実装
    id-expr の実装
    リファクタリング

    + は新規追加したところ
    ^ は更新

    block: ( text-block | code-block | ref-block ), block
    text-block: .*
    code-block: '{@' {formula}* '@}'
    ref-block: '{{' ( identifier | callable ) '}}'
    callable: caller-list '(' args ')'
    caller-list: identifier '.' caller-list | identifier
    args: arg ',' args | arg
    arg: digit | string | identifier
    ^ formula: ( expr | assign-expr | if-stmt | for-stmt | import-stmt | callable ), ( formula | '@}' block '{@' )
    if-stmt: 'if' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    elif-stmt: 'elif' comparison ':' ( formula | '@}' block '{@' ) ( 'end' | elif-stmt | else-stmt )
    else-stmt: 'else' ':' '@}'? ( block | formula ) '@}'? 'end'
    + for-stmt: 'for' expr ';' comparison ';' expr ':' ( formula | '@}' block '{@' ) 'end'
    comparison: expr cmp-op comparison | expr
    cmp-op: '==' | '!=' | '<' | '>' | '<=' | '>='
    expr: term '+' expr | term '-' expr | term
    term: factor '*' term | factor '/' term | factor
    ^ factor: digit | identifier | string | callable | id-expr | assign-expr | '(' expr ')'
    + id-expr: identifier ('++' | '--') | ('++' | '--') identifier
    ^ assign-expr: identifier assign-operator assign-expr | expr
    ^ assign-operator: '='
    import-stmt: 'import' identifier
    args: string | ',' args
    string: '"' .* '"'
    digit: [0-9]+
    identifier: ( [a-z] | [0-9] | _ )+ 

    i++ + 10
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

    class TypeError(TypeError):
        pass

    def parse(self, tokens, debug=False):
        self.debug_parse = debug
        self.strm = Stream(tokens)
        self.root = self.block(dep=0)

    def traverse(self, debug=False, opts=None):
        self.debug_traverse = debug
        self.opts = opts
        self.context = Context()
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
            self.traverse_ref_block(node, dep=dep+1)

        elif isinstance(node, FormulaNode):
            if node.comparison:
                self.context.last_expr_val = self._traverse(node.comparison, dep=dep+1)
            elif node.assign_expr:
                self.context.last_expr_val = self._traverse(node.assign_expr, dep=dep+1)
            elif node.import_:
                self._traverse(node.import_, dep=dep+1)
            elif node.for_:
                self._traverse(node.for_, dep=dep+1)
            elif node.if_:
                self._traverse(node.if_, dep=dep+1)
            elif node.callable:
                self._traverse(node.callable, dep=dep+1)

            if node.formula:
                self._traverse(node.formula, dep=dep+1)
            elif node.block:
                self._traverse(node.block, dep=dep+1)

        elif isinstance(node, ComparisonNode):
            if node.expr and node.comparison:
                lval = self._traverse(node.expr, dep=dep+1)
                rval = self._traverse(node.comparison, dep=dep+1)
                if node.op == '>':
                    return lval > rval
                elif node.op == '<':
                    return lval < rval
                elif node.op == '>=':
                    return lval >= rval
                elif node.op == '<=':
                    return lval <= rval
                elif node.op == '==':
                    return lval == rval
                elif node.op == '!=':
                    return lval != rval
                else:
                    raise AST.ModuleError('unsupported comparison operator "%s"' % node.op)

            elif node.expr:
                return self._traverse(node.expr, dep=dep+1)
            else:
                raise AST.ModuleError('unsupported node %s in comparison' % str(node))

        elif isinstance(node, ForNode):
            self.traverse_for(node, dep=dep+1) 

        elif isinstance(node, IfNode):
            result = self._traverse(node.comparison, dep=dep+1)
            if result:
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
            if node.term and node.expr:
                lval = self._traverse(node.term, dep=dep+1)
                rval = self._traverse(node.expr, dep=dep+1)
                if node.op == '+':
                    return lval + rval
                elif node.op == '-':
                    return lval - rval
                else:
                    raise AST.ModuleError('unsupported operation "%s" in traverse expr' % node.op)
            elif node.term:
                return self._traverse(node.term, dep=dep+1)
            else:
                raise AST.ModuleError('programming error. impossible case in traverse expr')

        elif isinstance(node, TermNode):
            if node.factor and node.term:
                lval = self._traverse(node.factor, dep=dep+1)
                rval = self._traverse(node.term, dep=dep+1)
                if node.op == '*':
                    return lval * rval
                elif node.op == '/':
                    return lval / rval
                else:
                    raise AST.ModuleError('unsupported operation "%s" in traverse term' % node.op)
            elif node.factor:
                return self._traverse(node.factor, dep=dep+1)
            else:
                raise AST.ModuleError('programming error. impossible case in traverse term')

        elif isinstance(node, FactorNode):
            if node.expr:
                return self._traverse(node.expr, dep=dep+1)
            elif node.identifier != None:
                if node.identifier not in self.context.syms.keys():
                    raise AST.ReferenceError('%s is not defined' % node.identifier)
                return self.context.syms[node.identifier]
            elif node.digit:
                return self._traverse(node.digit, dep=dep+1)
            elif node.string != None:
                return node.string
            elif node.callable:
                return self._traverse(node.callable, dep=dep+1)
            elif node.assign_expr:
                return self._traverse(node.assign_expr, dep=dep+1)
            elif node.id_expr:
                return self._traverse(node.id_expr, dep=dep+1)
            else:
                raise AST.ModuleError('impossible. invalid case in factor node')

        elif isinstance(node, IdExprNode):
            return self.traverse_id_expr(node, dep+1)

        elif isinstance(node, DigitNode):
            return node.value

        elif isinstance(node, AssignExprNode):
            return self.traverse_assign_expr(node, dep+1)

        elif isinstance(node, ImportNode):
            self.traverse_import(node, dep+1)

        elif isinstance(node, CallerNode):
            self.traverse_caller(node, dep+1)

        elif isinstance(node, CallableNode):
            return self.traverse_callable(node, dep+1)

        else:
            raise AST.ModuleError('impossible. not supported node', type(node))

    def traverse_callable(self, node, dep):
        package = node.caller_list.identifier
        if package == 'opts':
            if node.caller_list.caller_list.identifier == 'get':
                if self.opts and node.args.arg.string in self.opts.keys():
                    return self.opts[node.args.arg.string]
                else:
                    return ''
        elif package == 'alias':
            if node.caller_list.caller_list.identifier == 'set':
                self.context.alias_map[node.args.arg.string] = node.args.args.arg.string
                return None

    def traverse_for(self, node, dep):
        self._traverse(node.init_expr, dep=dep+1)
        while True:
            if node.comparison:
                result = self._traverse(node.comparison, dep=dep+1)
                if not result:
                    break

            if node.block:
                self._traverse(node.block, dep=dep+1)
            elif node.formula:
                self._traverse(node.formula, dep=dep+1)

            self._traverse(node.update_expr, dep=dep+1)

    def traverse_id_expr(self, node, dep):
        if node.identifier not in self.context.syms.keys():
            raise AST.SyntaxError('"%s" is not defined' % node.identifier)

        if node.front_or_back == 'front':
            if node.operator == '++':
                self.context.syms[node.identifier] += 1
                return self.context.syms[node.identifier]
            elif node.operator == '--':
                self.context.syms[node.identifier] -= 1
                return self.context.syms[node.identifier]

        elif node.front_or_back == 'back':
            if node.operator == '++':
                ret = self.context.syms[node.identifier]
                self.context.syms[node.identifier] += 1
                return ret
            elif node.operator == '--':
                ret = self.context.syms[node.identifier]
                self.context.syms[node.identifier] -= 1
                return ret

        else:
            raise AST.ModuleError('impossible. invalid front or back value "%s"' % node.front_or_back)

    def traverse_assign_expr(self, node, dep):
        if node.expr:
            return self._traverse(node.expr, dep=dep+1)

        if node.assign_operator == '=':
            self.context.syms[node.identifier] = self._traverse(node.assign_expr, dep=dep+1)
            return self.context.syms[node.identifier]

        raise AST.ModuleError('invalid operator %s' % node.assign_operator)
             
    def traverse_ref_block(self, node, dep):
        if node.identifier != None:
            if node.identifier in self.context.syms.keys():
                self.context.buffer += str(self.context.syms[node.identifier])
        elif node.callable:
            result = self._traverse(node.callable, dep=dep+1)
            self.context.buffer += str(result)

    def traverse_import(self, node, dep):
        if node.identifier == 'alias':
            self.context.imported_alias = True 
        else:
            raise AST.ImportError('can not import package "%s"' % node.identifier)

    def show_parse(self, name, dep):
        if self.debug_parse:
            t = self.strm.cur()
            print(dep, name + ': ' + str(t))

    def block(self, dep):
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

    def ref_block(self, dep):
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
            t2 = self.strm.cur()
            if t2 == Stream.EOF:
                raise AST.SyntaxError('reference block not closed')
            elif t2.value in ('.', '('):
                self.strm.prev()
                node.callable = self.callable(dep=dep+1)
            else:
                node.identifier = t.value
        elif t.kind == 'rdbrace':
            return node

        t = self.strm.get()
        if t == Stream.EOF or t.kind != 'rdbrace':
            raise AST.SyntaxError('not found "rdbrace"')

        return node

    def callable(self, dep):
        self.show_parse('callable', dep=dep)
        if self.strm.eof():
            return None

        node = CallableNode()

        i = self.strm.index
        node.caller_list = self.caller_list(dep=dep+1)
        if node.caller_list is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t.value != '(':
            self.strm.index = i
            return None
        
        node.args = self.args(dep=dep+1)

        t = self.strm.get()
        if t.value != ')':
            raise AST.SyntaxError('not found ")" in callable')
        
        return node

    def caller_list(self, dep):
        self.show_parse('caller_list', dep=dep)
        if self.strm.eof():
            return None

        node = CallerListNode()
        t = self.strm.get()
        if t == Stream.EOF:
            raise AST.SyntaxError('reached EOF in caller list')
        elif t.kind == 'identifier':
            node.identifier = t.value
        else:
            return None

        t = self.strm.get()
        if t.value == '.':
            node.caller_list = self.caller_list(dep=dep+1)
        else:
            self.strm.prev()

        return node

    def args(self, dep):
        self.show_parse('args', dep=dep)
        if self.strm.eof():
            return None

        node = ArgsNode()
        node.arg = self.arg(dep=dep+1)
        t = self.strm.get()
        if t.value == ',':
            node.args = self.args(dep=dep+1)
        else:
            self.strm.prev()

        return node

    def arg(self, dep):
        self.show_parse('arg', dep=dep)
        if self.strm.eof():
            return None

        node = ArgNode()
        t = self.strm.get()
        if t == Stream.EOF:
            raise AST.SyntaxError('reached EOF in argument')
        elif t.kind == 'digit':
            node.digit = DigitNode()
            node.digit.value = t.value
        elif t.kind == 'string':
            node.string = t.value
        elif t.kind == 'identifier':
            node.identifier = t.value
        else:
            raise AST.SyntaxError('not supported argument type "%s"' % t)

        return node

    def code_block(self, dep):
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
            self.strm.prev()
            return node
        elif t.kind in ('ldbrace', 'else', 'elif'):
            self.strm.prev()

        return node

    def text_block(self, dep):
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

    def formula(self, dep):
        self.show_parse('formula', dep=dep)
        if self.strm.eof():
            return None
        
        t = self.strm.cur()
        if t.kind in ('rbraceat', 'ldbrace', 'end', 'elif', 'else'):
            return None
        if t.kind in ('colon'):
            raise AST.SyntaxError('found "%s". invalid formula.' % t)

        node = FormulaNode()
        t = self.strm.cur()
        if t.kind == 'import':
            node.import_ = self.import_(dep=dep+1)
        elif t.kind == 'if':
            node.if_ = self.if_(dep=dep+1)
        elif t.kind == 'for':
            node.for_ = self.for_(dep=dep+1)
        else:
            i = self.strm.index
            node.callable = self.callable(dep=dep+1)
            if node.callable is None:
                self.strm.index = i
                t2 = self.strm.cur(1)
                if t2.value == '=':
                    node.assign_expr = self.assign_expr(dep=dep+1)
                    if not self.strm.eof() and node.assign_expr is None:
                        return None
                else:
                    node.comparison = self.comparison(dep=dep+1)
                    if not self.strm.eof() and node.comparison is None:
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

    def for_(self, dep):
        self.show_parse('for_', dep=dep)
        if self.strm.eof():
            return None
    
        tok = self.strm.get()
        if tok.kind != 'for':
            raise AST.ModuleError('impossible. not found "for" token')

        node = ForNode()
        node.init_expr = self.expr(dep=dep+1)
        tok = self.strm.get()
        if tok.kind != 'semicolon':
            raise AST.SyntaxError('not found ";" at initialize expression in for statement')

        node.comparison = self.comparison(dep=dep+1)
        tok = self.strm.get()
        if tok.kind != 'semicolon':
            raise AST.SyntaxError('not found ";" at comparison in for statement')

        node.update_expr = self.expr(dep=dep+1)
        tok = self.strm.get()
        if tok.kind != 'colon':
            raise AST.SyntaxError('not found ":" in for statement')

        tok = self.strm.get()
        if tok.kind == 'end':
            pass
        elif tok.kind == 'rbraceat':
            node.block = self.block(dep=dep+1)
            tok = self.strm.get()
            if tok.kind != 'end':
                raise AST.SyntaxError('not found "end" in for statement (1). token is %s' % tok)
        else:
            self.strm.prev()
            node.formula = self.formula(dep=dep+1)
            tok = self.strm.get()
            if tok.kind != 'end':
                raise AST.SyntaxError('not found "end" in for statement (2). token is %s' % tok)

        return node

    def comparison(self, dep):
        self.show_parse('comparison', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.cur()
        if tok.kind in ('colon', 'semicolon'):
            return None
        
        node = ComparisonNode()
        node.expr = self.expr(dep=dep+1)
        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.kind != 'comp_op':
            self.strm.prev()
            return node
        node.op = t.value

        node.comparison = self.comparison(dep=dep+1)
        return node

    def if_(self, dep, first_symbol='if'):
        self.show_parse('if', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != first_symbol:
            return None

        node = IfNode()
        node.comparison = self.comparison(dep=dep+1)
        if node.comparison is None:
            raise AST.SyntaxError('invalid if statement. not found comparison')

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
            raise AST.SyntaxError('reached EOF in if statement')

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
            raise AST.SyntaxError('not ended in if statement. token is %s' % t)

        return node

    def else_(self, dep):
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

    def expr(self, dep):
        self.show_parse('expr', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.cur()
        if tok.kind in ('colon', 'semicolon'):
            return None
        
        node = ExprNode()
        node.term = self.term(dep=dep+1)

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.value not in ('+', '-'):
            self.strm.prev()
            return node
        node.op = t.value

        node.expr = self.expr(dep=dep+1)
        return node

    def term(self, dep):
        self.show_parse('term', dep=dep)
        if self.strm.eof():
            return None

        node = TermNode()
        node.factor = self.factor(dep=dep+1)

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.value not in ('*', '/'):
            self.strm.prev()
            return node
        node.op = t.value

        node.term = self.term(dep=dep+1)
        return node

    def factor(self, dep):
        self.show_parse('factor', dep=dep)
        if self.strm.eof():
            return None

        node = FactorNode()
        t = self.strm.get()
        if t == Stream.EOF:
            raise AST.SyntaxError('reached EOF in factor')
        elif t.kind == 'lparen':
            node.expr = self.expr(dep=dep+1)
            t = self.strm.get()
            if t == Stream.EOF:
                raise AST.SyntaxError('not found rparen in factor. reached EOF')
            if t.kind != 'rparen':
                raise AST.SyntaxError('not found rparen in factor. token is %s' % t)
        elif t.kind == 'digit':
            node.digit = DigitNode()
            node.digit.value = t.value
        elif t.value in ('++', '--'):
            self.strm.prev()
            node.id_expr = self.id_expr(dep=dep+1)
        elif t.kind == 'identifier':
            if self.is_assign_op(self.strm.cur()):
                self.strm.prev()
                node.assign_expr = self.assign_expr(dep=dep+1)
            elif self.strm.cur().value in ('++', '--'):
                self.strm.prev()
                node.id_expr = self.id_expr(dep=dep+1)
            else:
                self.strm.prev()
                if self.is_callable(dep=dep+1):
                    node.callable = self.callable(dep=dep+1)
                else:
                    t = self.strm.get()
                    node.identifier = t.value
        elif t.kind == 'string':
            node.string = t.value
        else:
            self.strm.prev()
            return None

        return node 

    def id_expr(self, dep):
        self.show_parse('id_expr', dep=dep)
        if self.strm.eof():
            return None

        node = IdExprNode()
        tok = self.strm.get()
        if tok.kind == 'identifier':
            node.front_or_back = 'back'
            node.identifier = tok.value
            tok = self.strm.get()
            if tok.value not in ('++', '--'):
                raise AST.SyntaxError('not found operator in inc-dec expression')
            node.operator = tok.value
        else:
            if tok.value not in ('++', '--'):
                raise AST.SyntaxError('not found operator in inc-dec expression 2')            
            node.front_or_back = 'front'
            node.operator = tok.value
            tok = self.strm.get()
            if tok.kind != 'identifier':
                raise AST.SyntaxError('not found identifier in inc-dec expression')
            node.identifier = tok.value

        return node
        
    def is_callable(self, dep):
        self.show_parse('is_callable', dep=dep)
        if self.strm.eof():
            return None

        ret = True
        i = self.strm.index

        while not self.strm.eof():
            t = self.strm.get()
            if t == Stream.EOF:
                ret = False
                break
            elif t.kind == 'identifier':
                pass
            elif t.value == '.':
                pass
            elif t.value == '(':
                break
            else:
                ret = False
                break

        self.strm.index = i
        return ret


    def digit(self, dep):
        self.show_parse('digit', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != 'digit':
            raise AST.ModuleError('not found digit')

        node = DigitNode()
        node.value = t.value

        return node

    # ^ assign-expr: identifier assign-operator assign-expr | expr
    def assign_expr(self, dep):
        self.show_parse('assign_expr', dep=dep)
        if self.strm.eof():
            return None

        node = AssignExprNode()
        if self.strm.cur().kind == 'identifier' and self.is_assign_op(self.strm.cur(1)):
            tok = self.strm.get()
            node.identifier = tok.value
            tok = self.strm.get()
            node.assign_operator = tok.value
            node.assign_expr = self.assign_expr(dep=dep+1)
        else:
            node.expr = self.expr(dep=dep+1)

        return node

    def is_assign_op(self, tok):
        if tok == Stream.EOF:
            return False
        return tok.value in ('=')

    def import_(self, dep):
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
