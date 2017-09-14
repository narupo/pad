#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import CapTokenizer

class NilNode:

    def __init__(self):
        pass

class TextNode:

    def __init__(self):
        self.value = None

    def __str__(self):
        return str(self.value)

class ValueNode:

    def __init__(self):
        self.value = None

    def __str__(self):
        return str(self.value)

class BinNode:

    def __init__(self):
        self.lhs = None
        self.rhs = None

class OperandNode:

    def __init__(self):
        self.value = None

    def calc(self):
        return int(self.value)

class OperatorNode:
    """オペレーターノードは必要があれば calc が呼び出される。
    calc が呼ばれるたびに参照している変数を更新する。
    その変数はシンボル・テーブルの参照になるので、
    オペレーターノードはシンボル・テーブルを参照できる必要がある。
    """

    def __init__(self):
        self.operator = None
        self.lhs = None
        self.rhs = None

    def __str__(self):
        return str(self.operator)

    def __calc(self, n, dep=0, side='?'):
        print('_' * dep, side, str(n))
        if type(n) == OperatorNode:
            v = self.__calc(n.lhs, dep+1, 'L')
            if n.operator == '+':
                v += self.__calc(n.rhs, dep+1, 'R')
            elif n.operator == '-':
                v -= self.__calc(n.rhs, dep+1, 'R')
            elif n.operator == '*':
                v *= self.__calc(n.rhs, dep+1, 'R')
            elif n.operator == '/':
                v /= self.__calc(n.rhs, dep+1, 'R')
            return v
        elif type(n) == OperandNode:
            return n.calc()
        elif type(n) == VariableNode:
            raise Exception('TODO')
        raise Exception('unsupported node')

    def calc(self):
        return self.__calc(self)

class IfNode:

    def __init__(self):
        self.ncompare = None
        self.nthen = None
        self.nelif = None
        self.nelse = None

class ForNode:
    """
        for
       /   \       \      \
      init compare update then
    """

    def __init__(self):
        self.ncompare = None
        self.nthen = None
        self.ninit = None
        self.nupdate = None

class VariableNode:

    def __init__(self):
        self.identifier = None

class AssignNode:

    def __init__(self):
        self.lhs = None
        self.rhs = None

"""
program ::= [ plain | block ]*
plain ::= !('{{'|'}}')
block ::= '{{' [ code ]* '}}'
code ::= statement | expr

expr ::= pm_expr, [ ('+'|'-') pm_expr ]*
pm_expr ::= md_expr, [ ('*'|'/') md_expr ]*
md_expr ::= mono | '(' expr ')'
mono ::= number | variable
number ::= [ 0-9]*
variable ::= [ a-z | A-Z | _ ]* [ 0-9 ]*

statement ::= for-statement | if-statement | print-statement

print_statement ::= 'print' '(' print_content ')'
print_content ::= [ a_z|A_Z|0_9 ]*

if_statement ::= ('if'|'elif') if_compare '{' code '}', if_else | if_elif
if_compare ::= 1 | 0
if_else ::= 'else' '{' code '}'
if_elif ::= if_statement

for_statement ::= for_statement_1 | for_statement_2
for_statement_1 ::= 'for' '{' code '}'
for_statement_2 ::= 'for' for_compare '{' code '}'
for_statement_3 ::= 'for' for_init ';' for_compare ';' for_update '{' code '}'

"""
class App:

    def __init__(self):
        self.tkr = CapTokenizer()
        self.root = None
        self.isdebug = False
        self.symtab = {} # シンボルのテーブル
        self.lcr = None # last calc result. 最後の演算結果が入るレジスタ

    def run(self):
        self.tkr.parse(sys.stdin)
        for t in self.tkr.toks:
            print(t)
        self.root = self.program()
        self.traverse(self.root, side='R')

        print('_' * 32)
        print('symtab', self.symtab)
        print('lcr', self.lcr)

    def traverse(self, node, dep=0, side='?'):
        if self.isdebug:
            print('_' * dep, side, type(node))

        if type(node) == ValueNode:
            print(node.value)
        elif type(node) == TextNode:
            print(node.value)
        elif type(node) == BinNode:
            self.traverse(node.lhs, dep=dep+1, side='L')
            self.traverse(node.rhs, dep=dep+1, side='R')
        elif type(node) == IfNode:
            res = self.lcr = node.ncompare.calc()
            if res:
                self.traverse(node.nthen, dep+1, side='then')
            elif node.nelif:
                self.traverse(node.nelif, dep+1, side='elif')
            elif node.nelse:
                self.traverse(node.nelse, dep+1, side='else')
        elif type(node) == ForNode:
            while node.ncompare.value:
                self.traverse(node.nthen, dep+1, side='then')
            raise Exception('TODO')
        elif type(node) == OperatorNode:
            self.lcr = node.calc()
        elif type(node) == AssignNode:
            self.lcr = self.symtab[node.lhs.identifier] = node.rhs.calc()

    def program(self):
        root = BinNode()
        prev = root

        while self.tkr.cur():
            cur = BinNode()
            if self.tkr.cur() == '{{':
                cur.lhs = self.block()
            else:
                cur.lhs = self.plain()
            prev.rhs = cur
            prev = cur

        return root

    def plain(self):
        nplain = TextNode()
        nplain.value = self.tkr.get()
        return nplain

    def block(self):
        root = BinNode()
        prev = root

        self.tkr.get() # {{
        while self.tkr.cur() != '}}':
            cur = BinNode()
            cur.lhs = self.code()
            prev.rhs = cur
            prev = cur
        self.tkr.get() # }}

        return root

    def code(self):
        n = self.statement()
        if not n:
            n = self.expr()
        return n

    """
expr ::= ass_expr | pm_expr
ass_expr :: = variable '=' pm_expr
pm_expr ::= md_expr, pm_op pm_expr
pm_op ::= ('+'|'_')
md_expr ::= paren_expr, md_op pm_expr
md_op ::= ('*'|'/')
paren_expr ::= mono | '(' expr ')'
mono ::= number | variable
number ::= [ 0-9]*
variable ::= [ a-z | A-Z | _ ]* [ 0-9 ]*
    """
    def expr(self):
        if self.tkr.cur(1) == '=':
            return self.ass_expr()
        else:
            return self.pm_expr()

    def variable(self):
        root = VariableNode()
        root.identifier = self.tkr.get()
        return root

    def ass_expr(self):
        root = AssignNode()
        root.lhs = self.variable()
        self.tkr.get() # =
        root.rhs = self.expr()
        return root

    def pm_expr(self):
        root = None

        n1 = self.md_expr()
        if self.tkr.cur() in ['+', '-']:
            root = OperatorNode()
            root.lhs = n1
            root.operator = self.tkr.get()
            root.rhs = self.pm_expr()
        else:
            root = n1

        return root

    def md_expr(self):
        root = None

        n1 = self.paren_expr()
        if self.tkr.cur() in ['*', '/']:
            root = OperatorNode()
            root.lhs = n1
            root.operator = self.tkr.get()
            root.rhs = self.pm_expr()
        else:
            root = n1

        return root

    def paren_expr(self):
        root = None

        if self.tkr.cur() == '(':
            self.tkr.get() # '('
            root = self.expr()
            self.tkr.get() # ')'
        else:
            root = self.mono()

        return root

    def mono(self):
        root = None

        n1 = self.tkr.cur()
        if n1.isdigit():
            root = OperandNode()
            root.value = self.tkr.get()
        else:
            root = self.variable()

        return root

    def statement(self):
        if self.tkr.cur() == 'if':
            return self.if_statement()
        elif self.tkr.cur() == 'print':
            return self.print_statement()
        elif self.tkr.cur() == 'for':
            return self.for_statement()
        elif self.tkr.cur() == ';':
            self.tkr.get() # ;
            return NilNode()
        return None

    def for_statement(self):
        if self.tkr.cur(1) == '{':
            return self.for_statement_1()
        return self.for_statement_2()

    def for_init(self):
        while self.tkr.cur() not in [';', '{']:
            self.tkr.get()
        return ValueNode() # TODO

    def for_compare(self):
        ncompare = self.expr()
        while self.tkr.cur() not in [';', '{']:
            self.tkr.get()
        return ncompare

    def for_update(self):
        while self.tkr.cur() not in ['{']:
            self.tkr.get()
        return ValueNode() # TODO

    def for_statement_1(self):
        """
            for { code }
        """
        nfor = ForNode()

        self.tkr.get() # for
        nfor.ncompare = ValueNode()
        nfor.ncompare.value = 1 # eternal loop
        self.tkr.get() # {

        nfor.nthen = self.code()
        self.tkr.get() # }

        return nfor

    def for_statement_2(self):
        """
            for init; compare; update { code }
        """
        nfor = ForNode()

        self.tkr.get() # for
        nfor.ninit = self.for_init()
        self.tkr.get() # ;
        nfor.ncompare = self.for_compare()
        self.tkr.get() # ;
        nfor.nupdate = self.for_update()
        self.tkr.get() # {

        nfor.nthen = self.code()
        self.tkr.get() # }

        return nfor

    def for_statement_3(self):
        """
            for compare { code }
        """
        nfor = ForNode()

        self.tkr.get() # for
        nfor.ncompare = self.for_compare()
        self.tkr.get() # {

        nfor.nthen = self.code()
        self.tkr.get() # }

        return nfor

    def print_statement(self):
        nprint = ValueNode()
        self.tkr.get() # print
        self.tkr.get() # (
        nprint.value = self.tkr.get()
        self.tkr.get() # )
        return nprint

    def if_statement(self):
        nif = IfNode()

        self.tkr.get() # if
        nif.ncompare = self.if_compare()
        self.tkr.get() # {
        nif.nthen = self.code()
        self.tkr.get() # }
        if self.tkr.cur() == 'else':
            nif.nelse = self.if_else()
        elif self.tkr.cur() == 'elif':
            nif.nelif = self.if_statement()

        return nif

    def if_compare(self):
        return self.expr()

    def if_else(self):
        self.tkr.get() # else
        self.tkr.get() # {
        nelse = self.code()
        self.tkr.get() # }
        return nelse

def main():
    app = App()
    app.isdebug = True
    app.run()
    sys.exit(0)

if __name__ == '__main__':
    main()
