#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import SpaceTokenizer

class NilNode:

    def __init__(self):
        pass

class ValueNode:

    def __init__(self):
        self.value = None

class BinNode:

    def __init__(self):
        self.lhs = None
        self.rhs = None

class IfNode:

    def __init__(self):
        self.ncompare = None
        self.nthen = None
        self.nelif = None
        self.nelse = None

"""
program ::= [ plain | block ]*
plain ::= !('{{'|'}}')
block ::= '{{' [ code ]* '}}'
code ::= statement
statement ::= if-statement | print-statement

print-statement ::= 'print' '(' print-content ')'
print-content ::= [ a-z|A-Z|0-9 ]*

if-statement ::= ('if'|'elif') if-compare '{' code '}', if-else | if-elif
if-compare ::= 1 | 0
if-else ::= 'else' '{' code '}'
if-elif ::= if-statement

"""
class App:

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.root = None
        self.isdebug = False

    def run(self):
        self.tkr.parse(sys.stdin)
        self.root = self.program()
        self.traverse(self.root, side='R')

    def traverse(self, node, dep=0, side='?'):
        if self.isdebug:
            print('_' * dep, end='')
            print(side, type(node))

        if type(node) == ValueNode:
            print(node.value)
        elif type(node) == BinNode:
            self.traverse(node.lhs, dep=dep+1, side='L')
            self.traverse(node.rhs, dep=dep+1, side='R')
        elif type(node) == IfNode:
            if node.ncompare.value:
                self.traverse(node.nthen, dep+1, side='then')
            elif node.nelif:
                self.traverse(node.nelif, dep+1, side='elif')
            elif node.nelse:
                self.traverse(node.nelse, dep+1, side='else')

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
        nplain = ValueNode()
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
        return self.statement()

    def statement(self):
        if self.tkr.cur() == 'if':
            return self.if_statement()
        elif self.tkr.cur() == 'print':
            return self.print_statement()
        elif self.tkr.cur() == ';':
            self.tkr.get() # ;
            return NilNode()
        raise SyntaxError(self.tkr.cur())

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
        ncompare = ValueNode()
        ncompare.value = int(self.tkr.get())
        return ncompare

    def if_else(self):
        self.tkr.get() # else
        self.tkr.get() # {
        nelse = self.code()
        self.tkr.get() # }
        return nelse


def main():
    App().run()
    sys.exit(0)

if __name__ == '__main__':
    main()
