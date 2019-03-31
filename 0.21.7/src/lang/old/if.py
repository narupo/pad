#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys

"""
BNF

proc ::= if_expr | print
if_expr ::= 'if' cmp '{' proc '}', else_expr | elif_expr
cmp ::= '1' | '0'
"""
"""
if 1 {
    print("hello")    
}

   proc
      \
       if
   /   |   \
  1   then else
       /   /   \
"""
class Node:

    def __init__(self):
        pass

class IfNode:

    def __init__(self):
        pass

class ThenNode:

    def __init__(self):
        pass

class ElseNode:

    def __init__(self):
        self.

class App:

    def __init__(self):
        self.toks = sys.stdin.read().replace('(', ' ( ').replace(')', ' ) ').split()
        print(self.toks)

    def get(self):
        if len(self.toks) == 0:
            return None
        return self.toks.pop(0)

    def cur(self):
        if len(self.toks) == 0:
            return None
        return self.toks[0]

    def run(self):
        self.proc()

def main():
    App().run()
    sys.exit(0)

if __name__ == '__main__':
    main()
