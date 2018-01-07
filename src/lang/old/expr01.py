#!/usr/bin/python3
# coding: utf-8
import sys
from tokenizer import SpaceTokenizer

"""
1 + 2 * 3
1 * 2 + 3

expr ::= term [ pm term ]*
pm ::= '+' | '-'

term ::= factor [ md factor ]
md ::= '*' | '/'

factor ::= n | '(' expr ')'
n ::= [ 0~9 ]*

"""

class App():

    def __init__(self):
        self.tkr = SpaceTokenizer()
        self.tkr.parse(sys.stdin)

    def run(self):
        n = self.expr()
        print(n)

    def expr(self):
        n = self.term()
        while self.tkr.cur() in ['+', '-']:
            op = self.tkr.get()
            n2 = self.term()
            if op == '+':
                n += n2
            else:
                n -= n2
        return n

    def term(self):
        n = self.factor()
        while self.tkr.cur() in ['*', '/']:
            op = self.tkr.get()
            n2 = self.factor()
            if op == '*':
                n *= n2
            else:
                n /= n2
        return n

    def factor(self):
        if self.tkr.cur() == '(':
            self.tkr.get() # skip (
            return self.expr()
            self.tkr.get() # skip )
        else:
            return int(self.tkr.get())


def main():
    app = App()
    app.run()
    sys.exit(0)

if __name__ == '__main__':
    main()
