#!/usr/local/bin/python
# -*- coding: utf-8 -*-
import sys
import io
from tokenizer import Tokenizer
from ast import AST


def create_opts():
    opts = {}
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        arg = args[i]
        if arg[0] == '-' and arg[1] == '-':
            # long option
            i += 1
            if i < len(args):
                val = args[i]
                if val[0] == '-':
                    opts[arg[2:]] = 1
                else:
                    opts[arg[2:]] = val
            else:
                opts[arg[2:]] = 1
        elif arg[0] == '-':
            # short option
            i += 1
            if i < len(args):
                val = args[i]
                if val[0] == '-':
                    opts[arg[1:]] = 1
                else:
                    opts[arg[1:]] = val
            else:
                opts[arg[1:]] = 1
        else:
            i += 1

    return opts


def main():
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')
    sys.stdin = io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8')

    try:
        opts = create_opts()
        src = sys.stdin.read()
        tokenizer = Tokenizer()
        tokens = tokenizer.parse(src)
        ast = AST()
        ast.parse(tokens)
        context = ast.traverse(
            opts=opts,
        )
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
