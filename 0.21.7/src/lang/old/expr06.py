#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
from tokenizer import SpaceTokenizer

"""
           bin-node
        /             \
 if-expr | print-expr  bin-node

    if-expr
    /    \      \        \
 compare if-then if-else if-elif

    compare
      /
    value

    if-elif
     /   \      \       \
 compare if-then if-else if-elif

"""
class PrintNode:

    def __init__(self):
        self.value = None

class BinaryNode:

    def __init__(self):
        self.lhs = None
        self.rhs = None

class IfNode:

    def __init__(self):
        self.ncompare = None
        self.nthen = None
        self.nelif = None
        self.nelse = None

class IfCompareNode:

    def __init__(self):
        self.value = None

    def calc(self):
        return self.value

"""
code ::= [ if-expr | print-expr ]*
if-expr ::= ('if'|'elif') if-compare '{' code '}', if-else | if-elif
if-compare ::= 1 | 0
if-else ::= 'else' '{' code '}'
if-elif ::= if-expr

"""
class App:

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.root = None

    def run(self):
        self.tkr.parse(sys.stdin)
        self.root = self.code()
        self.traverse(self.root, side='R')

    def traverse(self, node, dep=0, side='?'):
        print('_' * dep, end='')
        print(side, type(node))

        if type(node) == BinaryNode:
            self.traverse(node.lhs, dep=dep+1, side='L')
            self.traverse(node.rhs, dep=dep+1, side='R')
        elif type(node) == PrintNode:
            print(node.value)
        elif type(node) == IfNode:
            res = node.ncompare.calc()
            if res:
                self.traverse(node.nthen, dep+1, side='then')
            elif node.nelif:
                self.traverse(node.nelif, dep+1, side='elif')
            elif node.nelse:
                self.traverse(node.nelse, dep+1, side='else')

    def code(self):
        root = BinaryNode()
        prev = root

        while self.tkr.cur() in ['if', 'print']:
            cur = BinaryNode()

            if self.tkr.cur() == 'if':
                cur.lhs = self.if_expr()
            elif self.tkr.cur() == 'print':
                cur.lhs = self.print_expr()
            prev.rhs = cur
            prev = cur

        return root

    def print_expr(self):
        self.tkr.get() # print
        self.tkr.get() # (
        nprint = PrintNode()
        nprint.value = self.tkr.get()
        self.tkr.get() # )
        return nprint

    def if_expr(self):
        nif = IfNode()

        self.tkr.get() # if
        nif.ncompare = self.if_compare()
        self.tkr.get() # {
        nif.nthen = self.code()
        self.tkr.get() # }
        if self.tkr.cur() == 'else':
            nif.nelse = self.if_else()
        elif self.tkr.cur() == 'elif':
            nif.nelif = self.if_expr()

        return nif

    def if_compare(self):
        ncompare = IfCompareNode()
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
