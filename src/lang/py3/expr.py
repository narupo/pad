#!/usr/local/bin/python
# -*- coding: utf-8 -*-
import sys
import io
import unittest


class Stream:
    EOF = (-1)

    def __init__(self, src):
        self._src = src
        self._limit = len(src)
        self._index = 0

    def eof(self):
        return self._index >= self._limit or self._index < 0

    def get(self):
        if self.eof():
            return self.EOF

        ch = self._src[self._index]
        self.next()
        return ch

    def cur(self, ofs=0):
        i = self._index + ofs
        if i < self._limit and i >= 0:
            return self._src[i]
        return self.EOF

    def prev(self):
        if self._index > 0:
            self._index -= 1

    def next(self):
        if self._index < self._limit:
            self._index += 1

    @property
    def index(self):
        return self._index
    
    @index.setter
    def index(self, val):
        self._index = val


class Token:
    def __init__(self, kind, value):
        self.kind = kind
        self.value = value

    def __str__(self):
        return '%s:%s' % (self.kind, self.value)


class Tokenizer:
    def parse(self, src):
        self.strm = Stream(src)
        self.tokens = []

        while not self.strm.eof():
            c = self.strm.get()
            if c.isdigit():
                self.strm.prev()
                self.read_digit()
            elif c == '(':
                self.tokens.append(Token(kind='lparen', value='('))
            elif c == ')':
                self.tokens.append(Token(kind='rparen', value=')'))
            elif c == '+':
                self.tokens.append(Token(kind='operator', value='+'))
            elif c == '-':
                self.tokens.append(Token(kind='operator', value='-'))
            elif c == '*':
                self.tokens.append(Token(kind='operator', value='*'))
            elif c == '/':
                self.tokens.append(Token(kind='operator', value='/'))

        return self.tokens

    def read_digit(self):
        buf = ''
        while not self.strm.eof():
            c = self.strm.get()
            if not c.isdigit():
                self.strm.prev()
                break
            else:
                buf += c

        self.tokens.append(Token(kind='digit', value=int(buf)))


class ExprNode:
    def __init__(self):
        self.term = None
        self.op = None
        self.expr = None


class TermNode:
    def __init__(self):
        self.factor = None
        self.op = None
        self.term = None


class FactorNode:
    def __init__(self):
        self.digit = None
        self.expr = None


'''
BNF:

(v = 2) * 2
    expr: term '+' expr | term '-' expr | term
    term: factor '*' term | factor '/' term | factor
    factor: '(' expr ')' | digit
    digit: [0-9]+

'''


class AST:
    def parse(self, tokens, debug=True):
        self.parse_debug = debug
        self.strm = Stream(tokens)
        self.root = self.expr()

    def traverse(self):
        return self._traverse(self.root)

    def _traverse(self, node):
        if node is None:
            return 0

        if isinstance(node, ExprNode):
            if node.term and node.expr:
                lval = self._traverse(node.term)
                rval = self._traverse(node.expr)
                if node.op == '+':
                    return lval + rval
                elif node.op == '-':
                    return lval - rval
                else:
                    raise ValueError('invalid operator %s in traverse expr' % node.op)
            elif node.term:
                return self._traverse(node.term)
        elif isinstance(node, TermNode):
            if node.factor and node.term:
                lval = self._traverse(node.factor)
                rval = self._traverse(node.term)
                if node.op == '*':
                    return lval * rval
                elif node.op == '/':
                    return lval / rval
                else:
                    raise ValueError('invalid operator %s in traverse term' % node.op)
            elif node.factor:
                return self._traverse(node.factor)
        elif isinstance(node, FactorNode):
            if node.expr:
                return self._traverse(node.expr)
            else:
                return node.digit
        else:
            raise TypeError('invalid node')

    def show_parse(self, name):
        if self.parse_debug:
            print(name, self.strm.cur())

    def expr(self):
        self.show_parse('expr')
        if self.strm.eof():
            return None

        node = ExprNode()
        node.term = self.term()

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.value not in ('+', '-'):
            self.strm.prev()
            return node
        node.op = t.value

        node.expr = self.expr()

        return node

    def term(self):
        self.show_parse('term')
        if self.strm.eof():
            return None

        node = TermNode()
        node.factor = self.factor()

        t = self.strm.get()
        if t == Stream.EOF:
            return node
        elif t.value not in ('*', '/'):
            self.strm.prev()
            return node
        node.op = t.value

        node.term = self.term()

        return node

    def factor(self):
        self.show_parse('factor')
        if self.strm.eof():
            return None

        node = FactorNode()
        t = self.strm.get()
        if t.kind == 'digit':
            node.digit = t.value
            return node
        elif t.kind == 'lparen':
            node.expr = self.expr()
            t = self.strm.get()
            if t == Stream.EOF:
                raise SyntaxError('not found rparen in factor. reached EOF')
            elif t.kind != 'rparen':
                raise ValueError('not found rparen in factor. token is %s' % t)
            return node

        raise ValueError('impossible in factor')


class Test(unittest.TestCase):
    def test_tokenizer(self):
        t = Tokenizer()

        ts = t.parse('1 + 2')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'digit')
        self.assertEqual(ts[0].value, 1)
        self.assertEqual(ts[1].kind, 'operator')
        self.assertEqual(ts[1].value, '+')
        self.assertEqual(ts[2].kind, 'digit')
        self.assertEqual(ts[2].value, 2)

        ts = t.parse('1 + 2 + 3')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'digit')
        self.assertEqual(ts[0].value, 1)
        self.assertEqual(ts[1].kind, 'operator')
        self.assertEqual(ts[1].value, '+')
        self.assertEqual(ts[2].kind, 'digit')
        self.assertEqual(ts[2].value, 2)
        self.assertEqual(ts[3].kind, 'operator')
        self.assertEqual(ts[3].value, '+')
        self.assertEqual(ts[4].kind, 'digit')
        self.assertEqual(ts[4].value, 3)


def main():
    t = Tokenizer()
    a = AST()

    while True:
        line = sys.stdin.readline()
        a.parse(t.parse(line), debug=True)
        print(a.traverse())

    sys.exit(0)


if __name__ == '__main__':
    main()
