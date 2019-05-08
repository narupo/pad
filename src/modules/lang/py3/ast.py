from stream import Stream
from tokens import Token
from nodes import *
from context import Context
import sys


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

    class NotFoundSymbol(RuntimeError):
        pass

    class ZeroDivision(RuntimeError):
        pass

    def parse(self, tokens, debug=False):
        self.debug_parse = debug
        self.strm = Stream(tokens)
        self.root = self.block(dep=0)

    def traverse(self, debug=False, opts=None):
        self.debug_traverse = debug
        self.opts = opts
        self.context = Context()
        self.scope_list = [self.root] # scope chain, element is Node
        self.return_result = None # result value of return statement
        self.returned = False # flag of case of return statement
        self._trav(self.root, dep=0)
        return self.context

    @property
    def current_scope(self):
        return self.scope_list[-1] # most back of scope

    def dp(self, *args, **kwargs):
        if self.debug_parse:
            print(*args, **kwargs)

    def dt(self, *args, **kwargs):
        if self.debug_traverse:
            print(*args, **kwargs)

    def show_trav(self, node, dep):
        if self.debug_traverse:
            print(('|'*dep) + str(dep) + ' ' + str(node).split('.')[1].split(' ')[0])

    def _trav(self, node, dep):
        if node is None:
            return
        self.show_trav(node, dep)

        if isinstance(node, BlockNode):
            return self.trav_block(node, dep=dep+1)

        elif isinstance(node, TextBlockNode):
            return self.trav_text_block(node, dep=dep+1)

        elif isinstance(node, CodeBlockNode):
            return self._trav(node.formula, dep=dep+1)

        elif isinstance(node, RefBlockNode):
            return self.trav_ref_block(node, dep=dep+1)

        elif isinstance(node, FormulaNode):
            return self.trav_formula(node, dep=dep+1)

        elif isinstance(node, AssignStmtLineNode):
            return self.trav_assign_stmt_line(node, dep=dep+1)

        elif isinstance(node, AssignStmtNode):
            return self.trav_assign_stmt(node, dep=dep+1)

        elif isinstance(node, ForStmtNode):
            return self.trav_for_stmt(node, dep=dep+1)

        elif isinstance(node, DefFuncNode):
            return self.trav_def_func(node, dep=dep+1)

        elif isinstance(node, ReturnStmtNode):
            return self.trav_return(node, dep=dep+1)

        elif isinstance(node, IfStmtNode):
            return self.trav_if_stmt(node, dep=dep+1)

        elif isinstance(node, ElseStmtNode):
            return self.trav_else_stmt(node, dep=dep+1)

        elif isinstance(node, ExprLineNode):
            return self.trav_expr_line(node, dep=dep+1)

        elif isinstance(node, ExprListNode):
            return self.trav_expr_list(node, dep=dep+1)

        elif isinstance(node, ExprNode):
            return self.trav_expr(node, dep=dep+1)

        elif isinstance(node, GorasuNode):
            return self.trav_gorasu(node, dep=dep+1)

        elif isinstance(node, KamiyuNode):
            return self.trav_kamiyu(node, dep=dep+1)

        elif isinstance(node, ModNode):
            return self.trav_mod(node, dep=dep+1)

        elif isinstance(node, TermNode):
            return self.trav_term(node, dep=dep+1)

        elif isinstance(node, FactorNode):
            return self.trav_factor(node, dep=dep+1)

        elif isinstance(node, NotExprNode):
            return self.trav_not_expr(node, dep=dep+1)

        elif isinstance(node, IdExprNode):
            return self.trav_id_expr(node, dep+1)

        elif isinstance(node, DigitNode):
            return self.trav_digit(node, dep=dep+1)

        elif isinstance(node, AssignExprNode):
            return self.trav_assign_expr(node, dep+1)

        elif isinstance(node, ImportStmtNode):
            return self.trav_import_stmt(node, dep+1)

        elif isinstance(node, CallerNode):
            return self.trav_caller(node, dep+1)

        elif isinstance(node, CallableNode):
            return self.trav_callable(node, dep+1)

        else:
            raise AST.ModuleError('impossible. not supported node', type(node))

    def trav_digit(self, node, dep):
        return node.value

    def trav_not_expr(self, node, dep):
        return not self._trav(node.expr, dep=dep+1)

    def trav_text_block(self, node, dep):
        self.context.buffer += node.text
        return node.text

    def trav_expr(self, node, dep):
        if node.gorasu and node.expr:
            lval = self._trav(node.gorasu, dep=dep+1)
            rval = self._trav(node.expr, dep=dep+1)
            if node.op == '&&':
                return lval and rval
            elif node.op == '||':
                return lval or rval
            else:
                raise AST.ModuleError('unsupported operation "%s" in traverse expr' % node.op)
        elif node.gorasu:
            return self._trav(node.gorasu, dep=dep+1)
        else:
            raise AST.ModuleError('programming error. impossible case in traverse expr')

    def trav_gorasu(self, node, dep):
        if node.kamiyu and node.gorasu:
            lval = self._trav(node.kamiyu, dep=dep+1)
            rval = self._trav(node.gorasu, dep=dep+1)
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
                raise AST.ModuleError('unsupported gorasu operator "%s"' % node.op)
        elif node.kamiyu:
            return self._trav(node.kamiyu, dep=dep+1)
        else:
            raise AST.ModuleError('unsupported node %s in gorasu' % str(node))

    def trav_kamiyu(self, node, dep):
        if node.mod and node.kamiyu:
            lval = self._trav(node.mod, dep=dep+1)
            rval = self._trav(node.kamiyu, dep=dep+1)
            if node.op == '+':
                return lval + rval
            elif node.op == '-':
                return lval - rval
            else:
                raise AST.ModuleError('unsupported operation "%s" in traverse kamiyu' % node.op)
        elif node.mod:
            return self._trav(node.mod, dep=dep+1)
        else:
            raise AST.ModuleError('programming error. impossible case in traverse kamiyu')

    def trav_mod(self, node, dep):
        if node.term and node.mod:
            lval = self._trav(node.term, dep=dep+1)
            rval = self._trav(node.mod, dep=dep+1)
            if node.op == '%':
                if rval == 0:
                    raise AST.ZeroDivision('zero division error')
                return lval % rval
            else:
                raise AST.ModuleError('unsupported operation "%s" in traverse mod' % node.op)
        elif node.term:
            return self._trav(node.term, dep=dep+1)
        else:
            raise AST.ModuleError('programming error. impossible case in traverse mod')

    def trav_term(self, node, dep):
        if node.factor and node.term:
            lval = self._trav(node.factor, dep=dep+1)
            rval = self._trav(node.term, dep=dep+1)
            if node.op == '*':
                return lval * rval
            elif node.op == '/':
                if rval == 0:
                    raise AST.ZeroDivision('zero division error')
                return lval / rval
            else:
                raise AST.ModuleError('unsupported operation "%s" in traverse term' % node.op)
        elif node.factor:
            return self._trav(node.factor, dep=dep+1)
        else:
            raise AST.ModuleError('programming error. impossible case in traverse term')

    def trav_factor(self, node, dep):
        if node.expr:
            return self._trav(node.expr, dep=dep+1)
        elif node.identifier != None:
            return self.find_sym(node.identifier)
        elif node.digit:
            return self._trav(node.digit, dep=dep+1)
        elif node.string != None:
            return node.string
        elif node.callable:
            return self._trav(node.callable, dep=dep+1)
        elif node.assign_expr:
            return self._trav(node.assign_expr, dep=dep+1)
        elif node.id_expr:
            return self._trav(node.id_expr, dep=dep+1)
        elif node.not_expr:
            return self._trav(node.not_expr, dep=dep+1)
        else:
            raise AST.ModuleError('impossible. invalid case in factor node')

    def trav_else_stmt(self, node, dep):
        if node.block:
            return self._trav(node.block, dep=dep+1)
        elif node.formula:
            return self._trav(node.formula, dep=dep+1)

    def trav_if_stmt(self, node, dep):
        result = self._trav(node.expr, dep=dep+1)
        if result:
            if node.formula:
                result = self._trav(node.formula, dep=dep+1)
            elif node.block:
                result = self._trav(node.block, dep=dep+1)
        else:
            if node.elif_stmt:
                result = self._trav(node.elif_stmt, dep=dep+1)
            elif node.else_stmt:
                result = self._trav(node.else_stmt, dep=dep+1)
        return result

    def trav_formula(self, node, dep):
        # formula is DO NOT RETURN
        if node.expr_line:
            result = self._trav(node.expr_line, dep=dep+1)
            if isinstance(result, tuple) and len(result) == 1:
                self.context.last_expr_val = result[0]
            else:
                self.context.last_expr_val = result
        elif node.import_stmt:
            self._trav(node.import_stmt, dep=dep+1)
        elif node.for_stmt:
            self._trav(node.for_stmt, dep=dep+1)
        elif node.if_stmt:
            self._trav(node.if_stmt, dep=dep+1)
        elif node.return_:
            # SET RETURN VALUE
            self.return_result = self._trav(node.return_, dep=dep+1)
            self.returned = True # SET FLAG
            return None # ALWAYS RETURN
        elif node.def_func:
            self._trav(node.def_func, dep=dep+1)
        elif node.assign_stmt_line:
            result = self._trav(node.assign_stmt_line, dep=dep+1)
            self.context.last_expr_val = result

        if self.returned:
            return None

        if node.formula:
            self._trav(node.formula, dep=dep+1)
        elif node.block:
            self._trav(node.block, dep=dep+1)
        return None

    def trav_block(self, node, dep):
        result = None
        if node.text_block:
            result = self._trav(node.text_block, dep=dep+1)
        elif node.code_block:
            result = self._trav(node.code_block, dep=dep+1)
        elif node.ref_block:
            result = self._trav(node.ref_block, dep=dep+1)
        if node.block:
            return self._trav(node.block, dep=dep+1)
        return result

    def trav_assign_stmt_line(self, node, dep):
        return self._trav(node.assign_stmt, dep=dep+1)

    def trav_assign_stmt(self, node, dep):
        if node.result_list and node.expr:
            identifiers = node.result_list.to_list()
            result = self._trav(node.expr, dep=dep+1)
            if isinstance(result, tuple):
                if len(identifiers) != len(result):
                    raise AST.SyntaxError('invalid call statement. not same length')
                for i in range(len(identifiers)):
                    self.scope_list[-1].syms[identifiers[i]] = result[i]
            elif len(identifiers) >= 2:
                raise AST.SyntaxError('invalid call statement. not same length (2)')
            else:
                self.scope_list[-1].syms[identifiers[0]] = result
            return result
        elif node.expr:
            return self._trav(node.expr, dep=dep+1)
        else:
            AST.ModuleError('impossible. programing error. invalid state of node in call statement')

    def trav_return(self, node, dep):
        return self._trav(node.expr_line, dep=dep+1)

    def trav_def_func(self, node, dep):
        # DO NOT CALL _trav with node.formula
        # This is called by callable
        self.scope_list[-1].syms[node.identifier] = node
        return None

    def trav_expr_line(self, node, dep):
        return self._trav(node.expr_list, dep=dep+1)

    def trav_expr_list(self, node, dep):
        results = []
        result = self._trav(node.expr, dep=dep+1)
        results.append(result)
        if node.expr_list:
            els = self.trav_expr_list(node.expr_list, dep=dep+1)
            for el in els:
                results.append(el)
        return tuple(results)

    def trav_callable(self, node, dep):
        firstname = node.name_list.identifier
        if firstname == 'opts':
            if node.name_list.name_list.identifier == 'get':
                identifier = self._trav(node.args.arg.expr, dep=dep+1)
                if not isinstance(identifier, str):
                    raise AST.SyntaxError('invalid argument for opts.get')
                if self.opts and identifier in self.opts.keys():
                    return self.opts[identifier]
                else:
                    return ''
        elif firstname == 'alias':
            if node.name_list.name_list.identifier == 'set':
                identifier = self._trav(node.args.arg.expr, dep=dep+1)
                value = self._trav(node.args.args.arg.expr, dep=dep+1)
                if not isinstance(identifier, str) or not isinstance(value, str):
                    raise AST.SyntaxError('invalid argument for alias.set')
                self.context.alias_map[identifier] = value
                return None
        else:
            def_func = self.find_sym(firstname)
            if not isinstance(def_func, DefFuncNode):
                raise AST.ReferenceError('"%s" is not callable' % firstname)

            self.export_args_to_def_func(def_func, def_func.dmy_args, node.args, dep=dep+1)

            self.scope_list.append(def_func)
            self.return_result = None
            self.returned = False
            self._trav(def_func.formula, dep=dep+1)
            self.scope_list.pop()
            self.returned = False

            if isinstance(self.return_result, tuple) and len(self.return_result) == 1:
                return self.return_result[0]
            return self.return_result

    def export_args_to_def_func(self, def_func, dmy_args, args, dep):
        """
        dmy_args, args を def_func の syms に展開する
        """
        if dmy_args and args:
            if args.arg and dmy_args.dmy_arg:
                result = self._trav(args.arg.expr, dep=dep+1)
                def_func.syms[dmy_args.dmy_arg.identifier] = result
                self.export_args_to_def_func(def_func, dmy_args.dmy_args, args.args, dep=dep+1)
        elif dmy_args is None and args is None:
            return None
        else:
            raise AST.SyntaxError('not same length of function arguments')

    def find_sym(self, key):
        for scope in self.scope_list[::-1]:
            if key in scope.syms.keys():
                return scope.syms[key]
        raise AST.NotFoundSymbol('"%s" is not found in symbol table' % key)

    def trav_for_stmt(self, node, dep):
        result = None
        self._trav(node.init_expr_list, dep=dep+1)
        while True:
            if node.comp_expr:
                result = self._trav(node.comp_expr, dep=dep+1)
                if not result:
                    break

            if node.block:
                self._trav(node.block, dep=dep+1)
            elif node.formula:
                self._trav(node.formula, dep=dep+1)

            self._trav(node.update_expr_list, dep=dep+1)
        return result

    def trav_id_expr(self, node, dep):
        if node.identifier not in self.scope_list[-1].syms.keys():
            raise AST.SyntaxError('"%s" is not defined' % node.identifier)

        if node.front_or_back == 'front':
            if node.operator == '++':
                self.scope_list[-1].syms[node.identifier] += 1
                return self.scope_list[-1].syms[node.identifier]
            elif node.operator == '--':
                self.scope_list[-1].syms[node.identifier] -= 1
                return self.scope_list[-1].syms[node.identifier]

        elif node.front_or_back == 'back':
            if node.operator == '++':
                ret = self.scope_list[-1].syms[node.identifier]
                self.scope_list[-1].syms[node.identifier] += 1
                return ret
            elif node.operator == '--':
                ret = self.scope_list[-1].syms[node.identifier]
                self.scope_list[-1].syms[node.identifier] -= 1
                return ret

        else:
            raise AST.ModuleError('impossible. invalid front or back value "%s"' % node.front_or_back)

    def trav_assign_expr(self, node, dep):
        if node.expr:
            return self._trav(node.expr, dep=dep+1)

        if node.assign_operator == '=':
            self.scope_list[-1].syms[node.identifier] = self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]
        elif node.assign_operator == '+=':
            if node.identifier not in self.scope_list[-1].syms.keys():
                raise AST.ReferenceError('"%s" is not defined' % node.identifier)
            self.scope_list[-1].syms[node.identifier] += self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]
        elif node.assign_operator == '-=':
            if node.identifier not in self.scope_list[-1].syms.keys():
                raise AST.ReferenceError('"%s" is not defined' % node.identifier)
            self.scope_list[-1].syms[node.identifier] -= self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]
        elif node.assign_operator == '*=':
            if node.identifier not in self.scope_list[-1].syms.keys():
                raise AST.ReferenceError('"%s" is not defined' % node.identifier)
            self.scope_list[-1].syms[node.identifier] *= self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]
        elif node.assign_operator == '/=':
            if node.identifier not in self.scope_list[-1].syms.keys():
                raise AST.ReferenceError('"%s" is not defined' % node.identifier)
            self.scope_list[-1].syms[node.identifier] /= self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]
        elif node.assign_operator == '%=':
            if node.identifier not in self.scope_list[-1].syms.keys():
                raise AST.ReferenceError('"%s" is not defined' % node.identifier)
            self.scope_list[-1].syms[node.identifier] %= self._trav(node.assign_expr, dep=dep+1)
            return self.scope_list[-1].syms[node.identifier]

        raise AST.ModuleError('invalid operator %s' % node.assign_operator)

    def trav_ref_block(self, node, dep):
        result = None
        if node.expr != None:
            result = self._trav(node.expr, dep=dep+1)
            if result is not None:
                self.context.buffer += str(result)
                self.context.last_expr_val = result
        return result

    def trav_import_stmt(self, node, dep):
        result = None
        if node.identifier == 'alias':
            self.context.imported_alias = True
        else:
            raise AST.ImportError('can not import package "%s"' % node.identifier)
        return result

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
        if t.kind == 'rdbrace':
            return node
        else:
            self.strm.prev()
            node.expr = self.expr(dep=dep+1)

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
        node.name_list = self.name_list(dep=dep+1)
        if node.name_list is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t.value != '(':
            self.strm.index = i
            return None

        node.args = self.args(dep=dep+1)

        t = self.strm.get()
        if t.value != ')':
            raise AST.SyntaxError('not found ")" in callable. token is %s' % t)

        return node

    def name_list(self, dep):
        self.show_parse('name_list', dep=dep)
        if self.strm.eof():
            return None

        node = NameListNode()
        t = self.strm.get()
        if t == Stream.EOF:
            raise AST.SyntaxError('reached EOF in caller list')
        elif t.kind == 'identifier':
            node.identifier = t.value
        else:
            return None

        t = self.strm.get()
        if t.value == '.':
            node.name_list = self.name_list(dep=dep+1)
        else:
            self.strm.prev()

        return node

    def args(self, dep):
        self.show_parse('args', dep=dep)
        if self.strm.eof():
            return None
        elif self.strm.cur().kind == 'rparen':
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
        else:
            self.strm.prev()
            node.expr = self.expr(dep=dep+1)

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

    def return_(self, dep):
        self.show_parse('return_', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.get()
        if tok.kind != 'jmp' and tok.value != 'return':
            raise AST.ModuleError('impossible. not found "return" in return statement')

        node = ReturnStmtNode()
        node.expr_line = self.expr_line(dep=dep+1)

        return node

    def formula(self, dep):
        self.show_parse('formula', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.cur()
        if t.kind in ('rbraceat', 'ldbrace', 'end', 'elif', 'else'):
            return None
        elif t.kind in ('colon'):
            raise AST.SyntaxError('found "%s". invalid formula.' % t)

        node = FormulaNode()
        t = self.strm.cur()
        if t.kind == 'import':
            node.import_stmt = self.import_stmt(dep=dep+1)
        elif t.kind == 'if':
            node.if_stmt = self.if_stmt(dep=dep+1)
        elif t.kind == 'for':
            node.for_stmt = self.for_stmt(dep=dep+1)
        elif t.kind == 'def':
            node.def_func = self.def_func(dep=dep+1)
        elif t.kind == 'jmp' and t.value == 'return':
            node.return_ = self.return_(dep=dep+1)
        elif t.kind == 'newline':
            t = self.strm.get()
            # このデータは必要？
            node.newline = t.value
        else:
            i = self.strm.index
            node.assign_stmt_line = self.assign_stmt_line(dep=dep+1)
            if node.assign_stmt_line is None:
                node.expr_line = self.expr_line(dep=dep+1)
                if not self.strm.eof() and node.expr_line is None:
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

    def def_func(self, dep):
        self.show_parse('def_func', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.ModuleError('impossible. reached EOF in function')
        elif tok.kind != 'def':
            raise AST.ModuleError('impossible. not found "def" in function')

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in function')
        elif tok.kind != 'identifier':
            raise AST.SyntaxError('need name in function')

        node = DefFuncNode()
        node.identifier = tok.value

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in function (2)')
        elif tok.kind != 'lparen':
            raise AST.SyntaxError('not found left paren in function')

        node.dmy_args = self.dmy_args(dep=dep+1)

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in function (3)')
        elif tok.kind != 'rparen':
            raise AST.SyntaxError('not found right paren in function')

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in function (4)')
        elif tok.kind != 'colon':
            raise AST.SyntaxError('not found colon in function')

        node.formula = self.formula(dep=dep+1)

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in function (5)')
        elif tok.kind != 'end':
            raise AST.SyntaxError('not found "end" in function')

        return node

    def dmy_args(self, dep):
        self.show_parse('dmy_args', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.get()
        if tok == Stream.EOF:
            raise AST.SyntaxError('reached EOF. invalid syntax in dummy arguments')
        elif tok.kind == 'rparen':
            self.strm.prev()
            return None
        elif tok.kind != 'identifier':
            raise AST.SyntaxError('invalid argument in dummy arguments. token is %s' % tok)

        node = DmyArgs()
        node.dmy_arg = DmyArg()
        node.dmy_arg.identifier = tok.value

        tok = self.strm.get()
        if tok.kind == 'rparen':
            self.strm.prev()
            return node
        elif tok.kind != 'comma':
            self.strm.prev()
            return None

        node.dmy_args = self.dmy_args(dep=dep+1)

        return node

    def for_stmt(self, dep):
        self.show_parse('for_stmt', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.get()
        if tok.kind != 'for':
            raise AST.ModuleError('impossible. not found "for" token')

        node = ForStmtNode()
        node.init_expr_list = self.expr_list(dep=dep+1)
        tok = self.strm.get()
        if tok.kind != 'semicolon':
            raise AST.SyntaxError('not found ";" at initialize expression in for statement')

        node.comp_expr = self.expr(dep=dep+1)
        tok = self.strm.get()
        if tok.kind != 'semicolon':
            raise AST.SyntaxError('not found ";" at comparison in for statement')

        node.update_expr_list = self.expr_list(dep=dep+1)
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
        elif not self.is_comp_op(t.kind):
            self.strm.prev()
            return node
        node.op = t.value

        node.comparison = self.comparison(dep=dep+1)
        return node

    def is_comp_op(self, tok):
        return tok.kind == 'operator' and tok.value in ('==', '!=', '<', '>', '<=', '>=')

    def assign_stmt_line(self, dep):
        self.show_parse('assign_stmt_line', dep=dep)
        if self.strm.eof():
            return None

        node = AssignStmtLineNode()
        i = self.strm.index

        node.assign_stmt = self.assign_stmt(dep=dep+1)
        if node.assign_stmt is None:
            self.strm.index = i
            return None

        tok = self.strm.get()
        if tok == Stream.EOF:
            pass
        elif tok.kind in ('newline', 'comma'):
            # read it
            pass
        elif tok.kind in ('rbraceat', 'end', 'else', 'elif'):
            self.strm.prev()
        else:
            raise AST.SyntaxError('not found newline at assign statement line')

        return node

    def assign_stmt(self, dep):
        self.show_parse('assign_stmt', dep=dep)
        if self.strm.eof():
            return None

        node = AssignStmtNode()

        found_assign_op = False
        i = self.strm.index

        while not self.strm.eof():
            t = self.strm.get()
            if t.kind == 'newline':
                break
            elif t.kind == 'operator' and t.value == '=':
                found_assign_op = True
                break
        self.strm.index = i

        if not found_assign_op:
            return None

        i = self.strm.index
        node.result_list = self.result_list(dep=dep+1)
        if node.result_list is None:
            self.strm.index = i
            return None

        tok = self.strm.get()
        if not (tok.kind == 'operator' and tok.value == '='):
            raise AST.SyntaxError('not found "=" in assign statement')

        node.expr = self.expr(dep=dep+1)
        if node.expr is None:
            self.strm.index = i
            return None

        return node

    def result_list(self, dep):
        self.show_parse('result_list', dep=dep)
        if self.strm.eof():
            return None

        tok = self.strm.get()
        if tok.kind != 'identifier':
            self.strm.prev()
            return None

        node = ResultListNode()
        node.identifier = tok.value

        tok = self.strm.get()
        if tok.kind != 'comma':
            self.strm.prev()
            return node

        node.result_list = self.result_list(dep=dep+1)

        return node

    def if_stmt(self, dep, first_symbol='if'):
        self.show_parse('if', dep=dep)
        if self.strm.eof():
            return None

        t = self.strm.get()
        if t.kind != first_symbol:
            return None

        node = IfStmtNode()
        node.expr = self.expr(dep=dep+1)
        if node.expr is None:
            raise AST.SyntaxError('invalid if statement. not found expr')

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
            node.elif_stmt = self.if_stmt(dep=dep+1, first_symbol='elif')
        elif t.kind == 'else':
            self.strm.prev()
            node.else_stmt = self.else_stmt(dep=dep+1)
        else:
            raise AST.SyntaxError('not ended in if statement. token is %s' % t)

        return node

    def else_stmt(self, dep):
        self.show_parse('else', dep=dep)
        if self.strm.eof():
            return None

        node = ElseStmtNode()

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

    def expr_line(self, dep):
        self.show_parse('expr_line', dep=dep)
        if self.strm.eof():
            return None

        node = ExprLineNode()

        i = self.strm.index
        node.expr_list = self.expr_list(dep=dep+1)
        if node.expr_list is None:
            self.strm.index = i
            return None

        tok = self.strm.get()
        if tok == Stream.EOF:
            return node
        elif tok.kind == 'newline':
            node.newline = tok.value
        elif tok.kind in ('rbraceat', 'end', 'else', 'elif'):
            self.strm.prev()
        else:
            raise AST.SyntaxError('not found newline')

        return node

    def expr_list(self, dep):
        self.show_parse('expr_list', dep=dep)
        if self.strm.eof():
            return None

        node = ExprListNode()

        i = self.strm.index
        node.expr = self.expr(dep=dep+1)
        if node.expr is None:
            self.strm.index = i
            return None

        tok = self.strm.get()
        if tok == Stream.EOF:
            return None
        elif tok.kind == 'comma':
            node.expr_list = self.expr_list(dep=dep+1)
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

        i = self.strm.index
        node = ExprNode()
        node.gorasu = self.gorasu(dep=dep+1)
        if node.gorasu is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.value not in ('&&', '||'):
            self.strm.prev()
            return node
        node.op = t.value

        node.expr = self.expr(dep=dep+1)
        return node

    def gorasu(self, dep):
        self.show_parse('expr', dep=dep)
        if self.strm.eof():
            return None

        i = self.strm.index
        node = GorasuNode()
        node.kamiyu = self.kamiyu(dep=dep+1)
        if node.kamiyu is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif not self.is_comp_op(t):
            self.strm.prev()
            return node
        node.op = t.value

        node.gorasu = self.gorasu(dep=dep+1)
        return node

    def kamiyu(self, dep):
        self.show_parse('kamiyu', dep=dep)
        if self.strm.eof():
            return None

        i = self.strm.index
        node = KamiyuNode()
        node.mod = self.mod(dep=dep+1)
        if node.mod is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif not (t.kind == 'operator' and t.value in ('+', '-')):
            self.strm.prev()
            return node
        node.op = t.value

        node.kamiyu = self.kamiyu(dep=dep+1)
        return node

    def mod(self, dep):
        self.show_parse('mod', dep=dep)
        if self.strm.eof():
            return None

        i = self.strm.index
        node = ModNode()
        node.term = self.term(dep=dep+1)
        if node.term is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif not (t.kind == 'operator' and t.value in ('%')):
            self.strm.prev()
            return node
        node.op = t.value

        node.mod = self.mod(dep=dep+1)
        return node

    def term(self, dep):
        self.show_parse('term', dep=dep)
        if self.strm.eof():
            return None

        i = self.strm.index
        node = TermNode()
        node.factor = self.factor(dep=dep+1)
        if node.factor is None:
            self.strm.index = i
            return None

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif not (t.kind == 'operator' and t.value in ('*', '/')):
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
        elif t.value == '!':
            self.strm.prev()
            node.not_expr = self.not_expr(dep=dep+1)
        else:
            self.strm.prev()
            return None

        return node

    def not_expr(self, dep):
        self.show_parse('not_expr', dep=dep)
        if self.strm.eof():
            return None

        node = NotExprNode()
        tok = self.strm.get()
        if tok.value != '!':
            raise AST.ModuleError('impossible. not found "!" in not expr')

        node.expr = self.expr(dep=dep+1)
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
        return tok.kind == 'operator' and tok.value in ('=', '+=', '-=', '*=', '/=', '%=')

    def import_stmt(self, dep):
        self.show_parse('import', dep=dep)
        if self.strm.eof():
            return None

        node = ImportStmtNode()
        t = self.strm.get()
        if t.kind != 'import':
            self.strm.prev()
            return None

        t = self.strm.get()
        if t.kind != 'identifier':
            raise AST.SyntaxError('invalid import statement')

        node.identifier = t.value
        return node
