#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import CapTokenizer

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
        self.value = None

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
        self.value = None

    def calc(self):
        return int(self.value)

class OperatorNode(Node):
    """オペレーターノードは必要があれば calc が呼び出される。
    calc が呼ばれるたびに参照している変数を更新する。
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

    def __calc(self, n):
        if type(n) == OperatorNode:
            v = self.__calc(n.lhs)
            if n.operator == '+':
                v += self.__calc(n.rhs)
            elif n.operator == '-':
                v -= self.__calc(n.rhs)
            elif n.operator == '*':
                v *= self.__calc(n.rhs)
            elif n.operator == '/':
                v /= self.__calc(n.rhs)
            return v
        elif type(n) == OperandNode:
            return n.calc()
        elif type(n) == VariableNode:
            raise Exception('TODO')
        raise Exception('unsupported node')

    def calc(self):
        return self.__calc(self)

class IfNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.ncompare = None
        self.nthen = None
        self.nelif = None
        self.nelse = None

class VariableNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.identifier = None

class AssignNode(Node):

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.lhs = None
        self.rhs = None

class NamespaceNode(Node):
    """ parent.child
    """

    def __init__(self, name='', tok=''):
        super().__init__(name, tok)
        self.lhs = None # str
        self.rhs = None # other node

class FuncNode(Node):

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
        self.symtab = {}
        self.lcr = None

    def run(self):
        self.tkr.parse(sys.stdin)
        self.root = self.program()
        self.traverse(self.root)
        self.dump_tree(self.root, side='R')
        print('_' * 32)
        print('symtab', self.symtab)
        print('lcr', self.lcr)

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
        ))

        if type(node) == BinNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == IfNode:
            self.dump_tree(node.ncompare, dep+1, side='ncompare')
            self.dump_tree(node.nthen, dep+1, side='nthen')
            self.dump_tree(node.nelif, dep+1, side='nelif')
            self.dump_tree(node.nelse, dep+1, side='nelse')
        elif type(node) == OperatorNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == NamespaceNode:
            self.dump_tree(node.lhs, dep+1, side='lhs')
            self.dump_tree(node.rhs, dep+1, side='rhs')
        elif type(node) == FuncNode:
            self.dump_tree(node.identifier, dep+1, side='identifier')
            self.dump_tree(node.args, dep+1, side='args')

    def traverse(self, node):
        if type(node) == TextNode:
            print(node.value)
        elif type(node) == BinNode:
            self.traverse(node.lhs) # 左優先
            self.traverse(node.rhs)
        elif type(node) == IfNode:
            res = self.lcr = node.ncompare.calc()
            if res:
                self.traverse(node.nthen)
            elif node.nelif:
                self.traverse(node.nelif)
            elif node.nelse:
                self.traverse(node.nelse)
        elif type(node) == OperatorNode:
            self.lcr = node.calc()
        elif type(node) == AssignNode:
            self.lcr = self.symtab[node.lhs.identifier] = node.rhs.calc()

    def cur(self, ofs=0):
        return self.tkr.cur(ofs)

    def get(self):
        return self.tkr.get()

    def program(self):
        root = None

        if self.cur() == '{{':
            root = self.dblock()
        else:
            root = BinNode('program', self.cur())
            root.lhs = self.plain()

        if self.cur():
            root.rhs = self.program()

        return root

    def plain(self):
        root = TextNode('plain', self.cur())
        root.value = self.get()
        return root

    def dblock(self):
        root = BinNode('dblock', self.cur())

        if self.cur() != '{{':
            raise SyntaxError(self.cur())
        self.get()

        root.lhs = self.code()

        if self.cur() != '}}':
            raise SyntaxError(self.cur())
        self.get()

        return root

    def code(self):
        root = BinNode('code', self.cur())

        root.lhs = self.statement()
        if root.lhs == None:
            return None

        if self.cur() != '}}':
            root.rhs = self.code()

        return root

    def statement(self):
        if self.cur() == 'if':
            return self.if_statement()
        elif self.cur() == '{':
            return self.block_statement()
        elif self.cur() in ['}', '}}']: # 終端記号
            return None

        return self.expr()

    def block_statement(self):
        root = BinNode('block_statement', self.cur())

        if self.get() != '{':
            raise SyntaxError()

        root.lhs = self.code()

        if self.get() != '}':
            raise SyntaxError()

        return root

    def if_statement(self):
        nif = IfNode('if_statement', self.cur())

        if self.get() != 'if':
            raise SyntaxError()

        nif.ncompare = self.if_compare()

        if self.get() != '{':
            raise SyntaxError()

        nif.nthen = self.code()

        if self.get() != '}':
            raise SyntaxError()

        if self.cur() == 'else':
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

        if self.get() != '{':
            raise SyntaxError()

        nelse = self.code()
        
        if self.get() != '}':
            raise SyntaxError()
        
        return nelse

    def expr(self):
        if self.cur(1) == '=':
            return self.ass_expr()
        else:
            return self.pm_expr()

    def variable(self):
        root = VariableNode('variable', self.cur())
        root.identifier = self.get()
        return root

    def ass_expr(self):
        root = AssignNode('ass_expr', self.cur())
        root.lhs = self.variable()
        self.get() # =
        root.rhs = self.expr()
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
        root.value = self.get()
        return root

    def ababa(self):
        root = None

        if self.cur(1) == '.':
            root = self.namespace()
        elif self.cur(1) == '(':
            root = self.func()
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

    def namespace(self):
        root = NamespaceNode('namespace', self.cur())
        root.lhs = self.identifier()
        if self.get() != '.':
            raise SyntaxError(self.cur())
        root.rhs = self.ababa()
        return root

    def func(self):
        root = FuncNode('func', self.cur())
        root.identifier = self.identifier()

        if self.get() != '(':
            raise SyntaxError()

        cur = root.args = BinNode()

        while self.cur() != ')':
            cur.lhs = self.operand()
            if self.cur() == ',':
                self.get()
            cur.rhs = BinNode()
            cur = cur.rhs

        if self.get() != ')':
            raise SyntaxError()

        return root

def main():
    app = App()
    app.run()
    sys.exit(0)

if __name__ == '__main__':
    main()
