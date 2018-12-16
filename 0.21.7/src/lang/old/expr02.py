#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import SpaceTokenizer

class Node:

    def __init__(self):
        self.type = 'nil'
        self.val = None
        self.lhs = None
        self.rhs = None

"""
expr ::= number [ '+' number ]*
number ::= [ 0~9 ]*

    +
   / \
  1   +
     / \
    2   2
"""
class App:

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.root = None

    def show_tree(self, n, dep=0, head='?'):
        for i in range(dep):
            print('-', end='')
        print(head, n.type, n.val)
        if n.lhs:
            self.show_tree(n.lhs, dep+1, 'l')
        if n.rhs:
            self.show_tree(n.rhs, dep+1, 'r')

    def run(self):
        self.tkr.parse(sys.stdin)
        self.root = self.expr()
        self.show_tree(self.root)
        val = self.follow(self.root)
        print(val)

    def follow(self, n):
        val = 0
        if n.type == 'add':
            val = self.follow(n.lhs)
            val += self.follow(n.rhs)
        elif n.type == 'sub':
            val = self.follow(n.lhs)
            val -= self.follow(n.rhs)            
        elif n.type == 'number':
            val = int(n.val)
        return val

    def expr(self):
        ret = None
        n = self.number()

        if self.tkr.cur() in ['+', '-']:
            op = Node()
            op.val = self.tkr.get()
            if op.val == '+':
                op.type = 'add'
            else:
                op.type = 'sub'
            op.lhs = n
            op.rhs = self.expr()
            ret = op
        else:
            ret = n

        return ret

    def number(self):
        n = Node()
        n.val = self.tkr.get()
        n.type = 'number'
        return n

def main():
    app = App()
    app.run()
    sys.exit(0)

if __name__ == '__main__':
    main()
