#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import CapTokenizer

class g:
    symtab = {} # Symbol table
    lcr = None # Last Calculation Result

class Node:

    def __init__(self, name='', tok=''):
        self.name = name
        self.tok = tok

class NilNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)

class TextNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.text = None

class BlockTextNode(Node):
    """ '}}' text '{{'
    """

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.text = None

class BinNode(Node):
    """左優先探索
    """

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.lhs = None
        self.rhs = None

class OperandNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.operand = None

    def value(self):
        return int(self.operand)

class OperatorNode(Node):
    """オペレーターノードは必要があれば value が呼び出される。
    value が呼ばれるたびに参照している変数を更新する。
    その変数はシンボル・テーブルの参照になるので、
    オペレーターノードはシンボル・テーブルを参照できる必要がある。
    """

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.operator = None
        self.lhs = None
        self.rhs = None

    def __str__(self):
        return str(self.operator)

    def __value(self, n):
        if type(n) == OperatorNode:
            v = self.__value(n.lhs)
            if n.operator == '+':
                v += self.__value(n.rhs)
            elif n.operator == '-':
                v -= self.__value(n.rhs)
            elif n.operator == '*':
                v *= self.__value(n.rhs)
            elif n.operator == '/':
                v /= self.__value(n.rhs)
            elif n.operator == '<':
                v = v < self.__value(n.rhs)
            elif n.operator == '<=':
                v = v <= self.__value(n.rhs)
            elif n.operator == '>':
                v = v > self.__value(n.rhs)
            elif n.operator == '=>':
                v = v >= self.__value(n.rhs)                

            return v
        elif type(n) == OperandNode:
            return n.value()
        elif type(n) == VariableNode:
            if n.identifier not in g.symtab.keys():
                g.symtab[n.identifier] = 0
            return g.symtab[n.identifier]
        raise Exception('unsupported node')

    def value(self):
        return self.__value(self)

class IfNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.ncompare = None
        self.nthen = None
        self.nelif = None
        self.nelse = None

class ForNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.ninit = None
        self.ncompare = None
        self.nupdate = None
        self.nthen = None

class VariableNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.identifier = None

    def value(self):
        return self.identifier

class NamespaceNode(Node):
    """ parent.child
    """

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.lhs = None # str
        self.rhs = None # other node

class CallerNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.identifier = None # str
        self.args = None # BinNode

class IdentifierNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.identifier = None

class App(Node):

    def __init__(self):
        self.tkr = CapTokenizer()
        self.root = None

    def run(self):
        self.tkr.parse(sys.stdin)
        self.tkr.insert(0, '}}')
        self.tkr.append('{{')
        print(self.tkr.toks)
        self.root = self.program()
        self.traverse(node=self.root)
        self.dump_tree(node=self.root, side='R')
        print('_' * 32)
        print('symtab', g.symtab)
        print('lcr', g.lcr)

    def dump_tree(self, node, dep=0, side='?'):
        if node == None:
            t = str(node)
            name = ''
            tok = ''
        else:
            t = str(type(node)).split('.')[1].split("'")[0]
            name = node.name
            tok = node.tok
        print(' |' * dep + '{side}:{type} {name} {tok}'.format(
            type=t, side=side, name=name, tok=tok,
        ), file=sys.stderr)

        if type(node) == BinNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == IfNode:
            self.dump_tree(node.ncompare, dep+1, side='ncompare')
            self.dump_tree(node.nthen, dep+1, side='nthen')
            self.dump_tree(node.nelif, dep+1, side='nelif')
            self.dump_tree(node.nelse, dep+1, side='nelse')
        elif type(node) == ForNode:
            self.dump_tree(node.ninit, dep+1, side='ninit')
            self.dump_tree(node.ncompare, dep+1, side='ncompare')
            self.dump_tree(node.nupdate, dep+1, side='nupdate')
            self.dump_tree(node.nthen, dep+1, side='nthen')
        elif type(node) == OperatorNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == NamespaceNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == CallerNode:
            self.dump_tree(node.identifier, dep+1, side='identifier')
            self.dump_tree(node.args, dep+1, side='args')

    def traverse(self, node):
        if type(node) == BlockTextNode:
            print(node.text)
        elif type(node) == BinNode:
            self.traverse(node.lhs) # 左優先
            self.traverse(node.rhs)
        elif type(node) == IfNode:
            res = g.lcr = node.ncompare.value()
            if res:
                self.traverse(node.nthen)
            elif node.nelif:
                self.traverse(node.nelif)
            elif node.nelse:
                self.traverse(node.nelse)
        elif type(node) == ForNode:
            if node.ninit:
                self.traverse(node.ninit)
            
            while node.ncompare.value():
                self.traverse(node.nthen)
                self.traverse(node.nupdate)

        elif type(node) == OperatorNode:
            if node.operator == '=':
                if type(node.lhs) == VariableNode:
                    g.lcr = g.symtab[node.lhs.identifier] = node.rhs.value()
                else:
                    raise SyntaxError('can\'t assign to left')
            elif node.operator == '+=':
                if type(node.lhs) == VariableNode:
                    g.symtab[node.lhs.identifier] += node.rhs.value()
                    g.lcr = g.symtab[node.lhs.identifier]
                else:
                    raise SyntaxError('can\'t assign to left')
            elif node.operator == '-=':
                if type(node.lhs) == VariableNode:
                    g.symtab[node.lhs.identifier] -= node.rhs.value()
                    g.lcr = g.symtab[node.lhs.identifier]
                else:
                    raise SyntaxError('can\'t assign to left')
            else:
                g.lcr = node.value()
        elif type(node) == CallerNode:
            funcname = node.identifier.identifier
            args = []
            self.grep_caller_args(node.args, args)
            if funcname == 'print':
                print(*args)

    def grep_caller_args(self, binnode, args):
        if binnode.rhs == None:
            return
        args.append(binnode.lhs.value())
        self.grep_caller_args(binnode.rhs, args)

    def cur(self, ofs=0):
        return self.tkr.cur(ofs)

    def get(self):
        return self.tkr.get()

    def program(self):
        root = None
        root = self.statement()
        return root

    def statement(self):
        if self.cur() in [None, 'elif', 'else', 'end']:
            return None

        root = BinNode('statement', self.cur())

        if self.cur() == '}}':
            root.lhs = self.block_statement()
        elif self.cur() == 'if':
            root.lhs = self.if_statement()
        elif self.cur() == 'for':
            root.lhs = self.for_statement()
        else:
            root.lhs = self.expr()

        root.rhs = self.statement()
        return root

    def block_statement(self):
        if self.cur() == None:
            return None

        if self.get() != '}}':
            raise SyntaxError(self.cur(-1))

        root = BlockTextNode('block_statement', self.cur())
        if self.cur() != '{{':
            root.text = self.get()
        else:
            root.text = ''

        if self.get() != '{{':
            raise SyntaxError(self.cur(-1))        

        return root

        return root

    def if_statement(self):
        nif = IfNode('if_statement', self.cur())

        if self.get() not in ['if', 'elif']:
            raise SyntaxError(self.cur(-1))

        nif.ncompare = self.if_compare()

        if self.get() != ':':
            raise SyntaxError(self.cur(-1))

        nif.nthen = self.statement()

        if self.cur() == 'end':
            self.get()
        elif self.cur() == 'else':
            nif.nelse = self.if_else()
        elif self.cur() == 'elif':
            nif.nelif = self.if_statement()

        return nif

    def if_compare(self):
        root = self.expr()
        if root == None:
            raise SyntaxError()
        return root

    def if_else(self):
        if self.get() != 'else':
            raise SyntaxError()

        if self.get() != ':':
            raise SyntaxError()

        nelse = self.statement()

        if self.get() != 'end':
            raise SyntaxError()
        
        return nelse

    def for_statement(self):
        if self.cur() != 'for':
            raise SyntaxError(self.cur())

        if self.cur(1) == ':':
            return self.for_1()
        return self.for_what()

    def for_1(self):
        root = None

        if self.get() != 'for':
            raise SyntaxError(self.cur())

        if self.get() != ':':
            raise SyntaxError(self.cur())

        root = ForNode('for_1', self.cur())
        root.ncompare = OperandNode('for_1', self.cur())
        root.ncompare.operand = 1

        root.nthen = self.statement()

        if self.get() != 'end':
            raise SyntaxError(self.cur())

        return root

    def for_what(self):
        root = ForNode('for_what', self.cur())

        if self.get() != 'for':
            raise SyntaxError(self.cur())

        expr = self.expr()

        if self.cur() == ':':
            self.get() # ':'
            root.ncompare = expr
            root.nthen = self.statement()
            if self.get() != 'end':
                raise SyntaxError(self.cur())
        else:
            if self.get() != ';':
                raise SyntaxError(self.cur())

            root.ncompare = self.expr()

            if self.get() != ';':
                raise SyntaxError(self.cur())

            root.nupdate = self.expr()

            if self.get() != ':':
                raise SyntaxError(self.cur())

            root.nthen = self.statement()

            if self.get() != 'end':
                raise SyntaxError(self.cur())        

        return root

    def expr(self):
        return self.ass_expr()

    def ass_expr(self):
        root = None

        n1 = self.cmp_expr()
        if self.cur() in ['=', '+=', '-=']:
            root = OperatorNode('ass_expr', self.cur())
            root.lhs = n1
            root.operator = self.get()
            root.rhs = self.ass_expr()
        else:
            root = n1

        return root

    def cmp_expr(self):
        root = None

        n1 = self.pm_expr()
        if self.cur() in ['<', '<=', '=>', '>']:
            root = OperatorNode('cmp_expr', self.cur())
            root.lhs = n1
            root.operator = self.get()
            root.rhs = self.cmp_expr()
        else:
            root = n1

        return root

    def pm_expr(self):
        root = None

        n1 = self.md_expr()
        if self.cur() in ['+', '-']:
            root = OperatorNode('pm_expr', self.cur())
            root.lhs = n1
            root.operator = self.get()
            root.rhs = self.pm_expr()
        else:
            root = n1

        return root

    def md_expr(self):
        root = None

        n1 = self.paren_expr()
        if self.cur() in ['*', '/']:
            root = OperatorNode('md_expr', self.cur())
            root.lhs = n1
            root.operator = self.get()
            root.rhs = self.pm_expr()
        else:
            root = n1

        return root

    def paren_expr(self):
        root = None

        if self.cur() == '(':
            self.get() # '('
            root = self.expr()
            self.get() # ')'
        else:
            root = self.operand()

        return root

    def operand(self):
        root = None

        n1 = self.cur()
        if n1.isdigit():
            root = self.number()
        else:
            root = self.ababa()

        return root

    def number(self):
        root = OperandNode('operand', self.cur())
        root.operand = self.get()
        return root

    def ababa(self):
        root = None

        if self.cur(1) == '(':
            root = self.caller()
        else:
            root = self.variable()

        return root

    def identifier(self):
        c = self.cur()[0]
        if not c.isalpha() and c != '_':
            raise SyntaxError(self.cur())

        root = IdentifierNode('identifier', self.cur())
        root.identifier = self.get()
        return root

    def caller(self):
        root = CallerNode('caller', self.cur())
        root.identifier = self.identifier()

        if self.get() != '(':
            raise SyntaxError()

        cur = root.args = BinNode('caller', self.cur())

        while self.cur() != ')':
            cur.lhs = self.operand()
            if self.cur() == ',':
                self.get()
            cur.rhs = BinNode('caller', self.cur())
            cur = cur.rhs

        if self.get() != ')':
            raise SyntaxError()

        return root

    def variable(self):
        root = VariableNode('variable', self.cur())
        root.identifier = self.get()
        return root

def main():
    app = App()
    app.run()
    sys.exit(0)

if __name__ == '__main__':
    main()
