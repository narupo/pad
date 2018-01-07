#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import SpaceTokenizer
from ast import Node

"""
BNF

expr ::= term, ('+'|'-') expr
term ::= factor, ('*'|'/') term
factor ::= [ 0~9 ]* | '(' expr ')'
"""
class App:

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.root = None

    def run(self):
        self.tkr.parse(sys.stdin)
        self.root = self.expr()
        self.show_tree(self.root)
        print(self.traverse(self.root))

    def traverse(self, n):
        if n.type == 'add':
            a = self.traverse(n.rhs)
            a += self.traverse(n.lhs)
            return a
        elif n.type == 'sub':
            a = self.traverse(n.rhs)
            a -= self.traverse(n.lhs)
            return a
        elif n.type == 'mul':
            a = self.traverse(n.rhs)
            a *= self.traverse(n.lhs)
            return a
        elif n.type == 'div':
            a = self.traverse(n.rhs)
            a /= self.traverse(n.lhs)
            return a
        else:
            return n.val

    def show_tree(self, n, dep=0, head='?'):
        for i in range(dep):
            print('-', end='')
        print(head, n.type, n.val)
        if n.lhs:
            self.show_tree(n.lhs, dep+1, 'l')
        if n.rhs:
            self.show_tree(n.rhs, dep+1, 'r')

    def expr(self):
        r = None
        n = self.term()

        if self.tkr.cur() in ['+', '-']:
            r = Node()
            r.val = self.tkr.get()
            if r.val == '+':
                r.type = 'add'
            else:
                r.type = 'sub'
            r.rhs = n
            r.lhs = self.expr()
        else:
            r = n
        return r

    def term(self):
        r = None
        n = self.factor()

        if self.tkr.cur() in ['*', '/']:
            r = Node()
            r.val = self.tkr.get()
            if r.val == '*':
                r.type = 'mul'
            else:
                r.type = 'div'
            r.rhs = n
            r.lhs = self.term()
        else:
            r = n
        return r

    def factor(self):
        n = None

        if self.tkr.cur() == '(':
            self.tkr.get()
            n = self.expr()
            self.tkr.get()
        else:
            n = Node()
            n.val = int(self.tkr.get())
            n.type = 'number'

        return n

def main():
    App().run()
    sys.exit(0)

if __name__ == '__main__':
    main()
