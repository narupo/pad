#!/usr/local/bin/python
# -*- coding: utf-8 -*-
import sys
import io
from tokenizer import Tokenizer
from ast import AST


def main():
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')
    sys.stdin = io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8')

    try:
        src = sys.stdin.read()
        tokenizer = Tokenizer()
        tokens = tokenizer.parse(src)
        ast = AST()
        ast.parse(tokens)
        context = ast.traverse()
        print(context.buffer)
    except Tokenizer.ParseError as e:
        print(e)
        sys.exit(1)
    except AST.SyntaxError as e:
        print(e)
        sys.exit(2)

    sys.exit(0)


if __name__ == '__main__':
    main()