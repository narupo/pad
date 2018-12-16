#!/usr/bin/python3
# coding: utf-8
import sys

toks = []
ti = 0

def gett():
    global toks, ti
    if ti >= len(toks):
        return None
    t = toks[ti]
    ti += 1
    return t

"""

expr   ::= number [ op number ]*
op     ::= '+' | '-'
number ::= [ 0~9 ]*

"""

def expr():
    n = number()
    while True:
        op = gett()
        if op == None:
            break
        n2 = number()
        if op == '+':
            n += n2
        else:
            n -= n2
    return n

def number():
    return int(gett())

def main():
    global toks
    s = sys.stdin.read()
    toks = s.split()
    n = expr()
    print(n)
    sys.exit(0)

if __name__ == '__main__':
    main()
