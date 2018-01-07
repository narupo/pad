#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from tokenizer import SpaceTokenizer

"""
      if
     /  \   \     \
  comp  then else elif

      then
        \
        node

      else
        \
        node

      elif
      /  \    \    \
    comp then else elif

"""
class FuncNode:

    def __init__(self):
        self.value = None

    def calc(self):
        return self.value

class IfNode:

    def __init__(self):
        self.ncomp = None
        self.nthen = None
        self.nelse = None
        self.nelif = None

class IfCompNode:

    def __init__(self):
        self.value = None

    def calc(self):
        return self.value

class IfThenNode:

    def __init__(self):
        self.node = None

    def calc(self):
        return 'then'

class IfElseNode:

    def __init__(self):
        self.node = None

    def calc(self):
        return self.node.calc()

"""
code ::= if-expr | print
if-expr ::= ('if'|'elif') if-compare '{' code '}'
if-compare ::= 1 | 0
"""
class App:

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.root = None

    def run(self):
        self.tkr.parse(sys.stdin)
        print(self.tkr.toks)
        self.root = self.code()
        self.show(self.root, side='R')

    def show(self, node, dep=0, side='?'):
        if type(node) == IfNode:
            if node.ncomp.calc():
                print(node.nthen.calc())
            elif node.nelse:
                print(node.nelse.calc())
            elif node.nelif:
                self.show(node.nelif)

    def code(self):
        if self.tkr.cur() == 'if':
            return self.if_expr()
        else:
            return self.print()

    def print(self):
        nfunc = FuncNode()

        self.tkr.get() # print
        self.tkr.get() # (
        nfunc.value = self.tkr.get()
        self.tkr.get() # )
        return nfunc

    def if_expr(self):
        nif = IfNode()

        self.tkr.get() # if | elif
        nif.ncomp = self.if_compare()
        self.tkr.get() # '{'
        nif.nthen = self.code()
        self.tkr.get() # '}'
        if self.tkr.cur() == 'else':
            nif.nelse = self.if_else()
        elif self.tkr.cur() == 'elif':
            nif.nelif = self.if_expr()
        return nif

    def if_compare(self):
        ncomp = IfCompNode()
        ncomp.value = int(self.tkr.get())
        return ncomp

    def if_else(self):
        self.tkr.get() # else
        self.tkr.get() # {
        nelse = IfElseNode()
        nelse.node = self.code()
        self.tkr.get() # }
        return nelse

def main():
    App().run()
    sys.exit(0)

if __name__ == '__main__':
    main()
