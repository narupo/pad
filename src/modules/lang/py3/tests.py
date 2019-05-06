from tokens import Token
from tokenizer import Tokenizer
from nodes import *
from ast import AST
import unittest


class Test(unittest.TestCase):
    def setUp(self):
        self.silent = False

    def test_tokenizer(self):
        if self.silent: return

        t = Tokenizer()
        
        ts = t.parse('')
        self.assertEqual(len(ts), 0)

        ts = t.parse('abc+/%123')
        self.assertEqual(len(ts), 1)
        self.assertEqual(ts[0].kind, 'text-block')
        self.assertEqual(ts[0].value, 'abc+/%123')

        ts = t.parse('{@')
        self.assertEqual(len(ts), 1)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')

        ts = t.parse('{@@}')
        self.assertEqual(len(ts), 2)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'rbraceat')
        self.assertEqual(ts[1].value, '@}')

        ts = t.parse('{{')
        self.assertEqual(len(ts), 1)
        self.assertEqual(ts[0].kind, 'ldbrace')
        self.assertEqual(ts[0].value, '{{')

        ts = t.parse('{{}}')
        self.assertEqual(len(ts), 2)
        self.assertEqual(ts[0].kind, 'ldbrace')
        self.assertEqual(ts[0].value, '{{')
        self.assertEqual(ts[1].kind, 'rdbrace')
        self.assertEqual(ts[1].value, '}}')

        ts = t.parse('{{ a }}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'ldbrace')
        self.assertEqual(ts[0].value, '{{')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'rdbrace')
        self.assertEqual(ts[2].value, '}}')

        ts = t.parse('{{ opts.get("") }}')
        self.assertEqual(len(ts), 8)
        self.assertEqual(ts[0].kind, 'ldbrace')
        self.assertEqual(ts[0].value, '{{')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'opts')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '.')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'get')
        self.assertEqual(ts[4].kind, 'lparen')
        self.assertEqual(ts[4].value, '(')
        self.assertEqual(ts[5].kind, 'string')
        self.assertEqual(ts[5].value, '')
        self.assertEqual(ts[6].kind, 'rparen')
        self.assertEqual(ts[6].value, ')')
        self.assertEqual(ts[7].kind, 'rdbrace')
        self.assertEqual(ts[7].value, '}}')

        ts = t.parse('aaa{@bbb@}ccc{@ddd@}eee')
        self.assertEqual(len(ts), 9)
        self.assertEqual(ts[0].kind, 'text-block')
        self.assertEqual(ts[0].value, 'aaa')
        self.assertEqual(ts[1].kind, 'lbraceat')
        self.assertEqual(ts[1].value, '{@')
        self.assertEqual(ts[2].kind, 'identifier')
        self.assertEqual(ts[2].value, 'bbb')
        self.assertEqual(ts[3].kind, 'rbraceat')
        self.assertEqual(ts[3].value, '@}')
        self.assertEqual(ts[4].kind, 'text-block')
        self.assertEqual(ts[4].value, 'ccc')
        self.assertEqual(ts[5].kind, 'lbraceat')
        self.assertEqual(ts[5].value, '{@')
        self.assertEqual(ts[6].kind, 'identifier')
        self.assertEqual(ts[6].value, 'ddd')
        self.assertEqual(ts[7].kind, 'rbraceat')
        self.assertEqual(ts[7].value, '@}')
        self.assertEqual(ts[8].kind, 'text-block')
        self.assertEqual(ts[8].value, 'eee')

        with self.assertRaises(Tokenizer.ParseError):
            t.parse('{@@@}')

        ts = t.parse('{@ . @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'operator')
        self.assertEqual(ts[1].value, '.')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ , @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'comma')
        self.assertEqual(ts[1].value, ',')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ : @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'colon')
        self.assertEqual(ts[1].value, ':')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ ; @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'semicolon')
        self.assertEqual(ts[1].value, ';')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ ( @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'lparen')
        self.assertEqual(ts[1].value, '(')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ ) @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'rparen')
        self.assertEqual(ts[1].value, ')')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ "" @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'string')
        self.assertEqual(ts[1].value, '')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ "content" @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'string')
        self.assertEqual(ts[1].value, 'content')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ "aa""bb" @}')
        self.assertEqual(len(ts), 4)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'string')
        self.assertEqual(ts[1].value, 'aa')
        self.assertEqual(ts[2].kind, 'string')
        self.assertEqual(ts[2].value, 'bb')
        self.assertEqual(ts[3].kind, 'rbraceat')
        self.assertEqual(ts[3].value, '@}')

        ts = t.parse('{@ import @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'import')
        self.assertEqual(ts[1].value, 'import')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ if @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'if')
        self.assertEqual(ts[1].value, 'if')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ elif @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'elif')
        self.assertEqual(ts[1].value, 'elif')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ else @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'else')
        self.assertEqual(ts[1].value, 'else')
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        ts = t.parse('{@ 123 @}')
        self.assertEqual(len(ts), 3)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 123)
        self.assertEqual(ts[2].kind, 'rbraceat')
        self.assertEqual(ts[2].value, '@}')

        # operators
        ts = t.parse('{@ a = b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '=')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a & b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '&')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a && b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '&&')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a | b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '|')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a || b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '||')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a + b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '+')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a - b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '-')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a * b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '*')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ a / b @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'identifier')
        self.assertEqual(ts[1].value, 'a')
        self.assertEqual(ts[2].kind, 'operator')
        self.assertEqual(ts[2].value, '/')
        self.assertEqual(ts[3].kind, 'identifier')
        self.assertEqual(ts[3].value, 'b')
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')

        ts = t.parse('{@ 0 == 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '==')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

        ts = t.parse('{@ 0 != 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '!=')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

        ts = t.parse('{@ 0 > 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '>')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

        ts = t.parse('{@ 0 < 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '<')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

        ts = t.parse('{@ 0 >= 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '>=')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

        ts = t.parse('{@ 0 <= 0 @}')
        self.assertEqual(len(ts), 5)
        self.assertEqual(ts[0].kind, 'lbraceat')
        self.assertEqual(ts[0].value, '{@')
        self.assertEqual(ts[1].kind, 'digit')
        self.assertEqual(ts[1].value, 0)
        self.assertEqual(ts[2].kind, 'comp_op')
        self.assertEqual(ts[2].value, '<=')
        self.assertEqual(ts[3].kind, 'digit')
        self.assertEqual(ts[3].value, 0)
        self.assertEqual(ts[4].kind, 'rbraceat')
        self.assertEqual(ts[4].value, '@}')        

    def test_ast_basic(self):
        if self.silent: return

        t = Tokenizer()
        a = AST()

        a.parse(t.parse(''))
        self.assertEqual(a.root, None)

        a.parse(t.parse('abc'))
        a.traverse()
        self.assertEqual(a.root.text_block.text, 'abc')

        a.parse(t.parse('{@'))

        a.parse(t.parse('{@ v = "v"'))

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ 1: @}'))

    def test_ast_import(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ import @}'))

        a.parse(t.parse('{@ import alias'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)

        a.parse(t.parse('{@ import alias @}'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)

        a.parse(t.parse('aaa{@ import alias @}bbb{@ import alias @}ccc'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)

        a.parse(t.parse('{@ import alias @}{@ import alias @}'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)

    def test_ast_alias(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            import alias
            alias.set("dtl", "run bin/date-line/date-line.py")
@}'''))
        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                import alias
                alias.set(1, "run bin/date-line/date-line.py")
            @}'''))
            a.traverse()
        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                import alias
                alias.set("dtl", 1)
            @}'''))
            a.traverse()

    def test_ast_opts(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            v = opts.get("name")
@}{{ v }}'''))
        c = a.traverse(opts={
            'name': 'value',
        })
        self.assertEqual(c.buffer, 'value')

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                opts.get(1)
            @}'''))
            a.traverse(opts={
                'name': 'value',
            })
        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                opts.get("unknown")
            @}'''))
            a.traverse(opts={
                'name': 'value',
            })

    def test_ast_expr(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('{@ 1 + 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 3)

        a.parse(t.parse('{@ 2 - 1 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 2 * 3 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 6)

        a.parse(t.parse('{@ 4 / 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('{@ 1 + 2 * 3 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 7)

        a.parse(t.parse('{@ 1 + 2 * 3 / 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 4)

        a.parse(t.parse('{@ (1 + 2) * 3 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 9)

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ (1 + 2 @}'))

        a.parse(t.parse('{@ v = 1 + 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 3)
        self.assertEqual(a.current_scope.syms['v'], 3)

        a.parse(t.parse('''{@
            a = 1 + 2
            v = a + 3
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 6)
        self.assertEqual(a.current_scope.syms['v'], 6)

        a.parse(t.parse('{@ 1 && 1 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 1 && 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 0 && 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 1 || 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 0 || 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 1 + 1 && 1 + 1 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('{@ 2 * 2 && 4 / 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('{@ 1 && 1 && 1 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 1 && 1 && 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 1 || 1 || 1 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ a = 1, b = 2 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, (1, 2))

        a.parse(t.parse('{@ a = 1, b = 2, c = 3 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, (1, 2, 3))

    def test_ast_id_expr(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            v = 1
            v++
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            v = 1
            ++v
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            v = 1
            v--
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            v = 1
            --v
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('''{@
            v = 1
            ++v + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 4)

        a.parse(t.parse('''{@
            v = 1
            v++ + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 3)

        a.parse(t.parse('''{@
            v = 1
            --v + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            v = 1
            v-- + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 3)

    def test_ast_call_stmt(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                def f():
                    return 1
                end
                a, b = f()
            @}'''))
            c = a.traverse()

        a.parse(t.parse('''{@
            def f():
                return 1
            end
            a = f()
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['a'], 1)

        a.parse(t.parse('''{@
            def f():
                return 1, 2
            end
            a, b = f()
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['a'], 1)
        self.assertEqual(a.current_scope.syms['b'], 2)

    def test_ast_callable(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            def f():
                return 1, 2
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)

    def test_ast_def_func(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            def f():
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)

        a.parse(t.parse('''{@
            def f():
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                return 1
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)

        a.parse(t.parse('''{@
            def f():
                return 1, 2
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)

        a.parse(t.parse('''{@
            def f1():
            end
            def f2():
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f1']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f2']), DefFuncNode)

        a.parse(t.parse('''{@
            def f1():
            end
@}abc{@
            def f2():
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f1']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f2']), DefFuncNode)
        self.assertEqual(c.buffer, 'abc')

        a.parse(t.parse('''{@
            def f(arg1):
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].dmy_args.dmy_arg.identifier, 'arg1')

        a.parse(t.parse('''{@
            def f(arg1, arg2):
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].dmy_args.dmy_arg.identifier, 'arg1')
        self.assertEqual(a.current_scope.syms['f'].dmy_args.dmy_args.dmy_arg.identifier, 'arg2')

        a.parse(t.parse('''{@
            def f():
                v = 1
            end
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        with self.assertRaises(KeyError):
            self.assertEqual(a.current_scope.syms['v'], 1)

        a.parse(t.parse('''{@
            def f():
                v = 1
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                return 1
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            def f():
                if 1:
                    return 1
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                elif 1:
                    return 2
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                elif 0:
                    return 2
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                if 1:
                    if 1:
                        return 1
                    end
                end
                return 0
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                else:
                    return 2
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                end

                return 2

                if 1:
                    return 3
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            def f():
                if 0:
                    return 1
                end
                def f2():
                    if 1:
                        return 2
                    end
                    return 0
                end
                return f2()
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            def f():
                v = 1
                return v
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            def f():
                return 1
            end
            v = f()
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            def f():
                return 1 + 2
            end
            v = f()
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['v'], 3)
        self.assertEqual(c.buffer, '3')

        a.parse(t.parse('''{@
            def f():
                v = 1
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['v'], 1)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            v = 1
            def f():
                a = 2
                return v
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(a.current_scope.syms['f'].syms['a'], 2)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            v = 1
            def f():
                return v
            end
            def f2():
                return v + 1
            end
@}{{ f() }}, {{ f2() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(c.buffer, '1, 2')

        a.parse(t.parse('''{@
            def f():
                v = 1
                def f2():
                    v = 2
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f'].syms['f2']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                v = 1
                def f2():
                    v = 2
                end
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f'].syms['f2']), DefFuncNode)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                v = 1
                def f2():
                    v = 2
                end
                return f2()
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f'].syms['f2']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['f2'].syms['v'], 2)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                v = 1
                def f2():
                    v = 2
                    return v
                end
                return f2()
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f'].syms['f2']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['f2'].syms['v'], 2)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            def f():
                v = 1
                def f2():
                    v = 2
                    return v
                end
                return f2()
            end
            def f2():
                return f()
            end
@}{{ f2() }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f2']), DefFuncNode)
        self.assertEqual(type(a.current_scope.syms['f'].syms['f2']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['f2'].syms['v'], 2)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            f("lval") + "rval"
@}'''))

        a.parse(t.parse('''{@
            def f(arg1):
                v = arg1
            end
            f("arg1")
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['arg1'], 'arg1')
        self.assertEqual(a.current_scope.syms['f'].syms['v'], 'arg1')

        a.parse(t.parse('''{@
            def f(arg):
                return arg
            end
            a = f("lval") + "rval"
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['arg'], 'lval')
        self.assertEqual(c.buffer, 'lvalrval')

        a.parse(t.parse('''{@
            def f(arg1, arg2):
                a = arg1
                b = arg2
            end
            f("arg1", "arg2")
@}'''))
        c = a.traverse()
        self.assertEqual(type(a.current_scope.syms['f']), DefFuncNode)
        self.assertEqual(a.current_scope.syms['f'].syms['arg1'], 'arg1')
        self.assertEqual(a.current_scope.syms['f'].syms['arg2'], 'arg2')
        self.assertEqual(a.current_scope.syms['f'].syms['a'], 'arg1')
        self.assertEqual(a.current_scope.syms['f'].syms['b'], 'arg2')

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                def f(arg1, arg2):
                end
                f("arg1")
    @}'''))
            c = a.traverse()

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('''{@
                def f(arg1):
                end
                f("arg1", "arg2")
    @}'''))
            c = a.traverse()

    def test_ast_return(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            v = 0
            return 1
            v = 1
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.buffer, '')

        a.parse(t.parse('''{@
            def f():
                return 1
            end
@}{{ f() }}'''))
        c = a.traverse()
        self.assertEqual(c.buffer, '1')

    def test_ast_assign(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            a = "s"
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['a'], 's')

        a.parse(t.parse('''{@
            a = 0 b = 1
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['a'], 0)
        self.assertEqual(a.current_scope.syms['b'], 1)

        a.parse(t.parse('''{@
            a = "s"
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['a'], 's')

        a.parse(t.parse('''{@
            v = "v"
            v = ""
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], '')

        a.parse(t.parse('''{@
            v = "v"
            v = "v2"
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')

        a.parse(t.parse('''{@
            v = "v"
            v = ""
            v = "v2"
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')

        a.parse(t.parse('''{@ v = "v" @}
{@ v = "" @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], '')

        a.parse(t.parse('''{@ v = "v" @}
{@ v = "" @}
{@ v = "v2" @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')

        a.parse(t.parse('''{@ v = "" @}
{@ v = "v" @} '''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v')
        
        a.parse(t.parse('{@ v = 1 + 2 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 3)

        a.parse(t.parse('{@ v = v = 1 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 1)

        a.parse(t.parse('{@ v = 1 + 2 * 3 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 7)

        a.parse(t.parse('{@ v = (1 + 2) * 3 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 9)

        a.parse(t.parse('{@ v = opts.get("a") @}'))
        c = a.traverse(opts={ 'a': 'b' })
        self.assertEqual(a.current_scope.syms['v'], "b")

        # 以下の式はPython3ではエラーになる
        # PHP7, Ruby2.3ではエラーにならずlast_expr_valは2
        a.parse(t.parse('''{@
            1 + v = 1
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            i = 0
            v = 0
            v = v + i
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 0)

    def test_ast_comparison(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('{@ 0 == 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 0 != 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 1 > 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 1 < 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ 1 >= 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ 1 <= 0 @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('''{@
            v = 0
            v == 0 
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            lhs = 0
            rhs = 0
            lhs == rhs
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            lhs = "a"
            rhs = 0
            lhs == rhs
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('''{@
            lhs = 0
            rhs = "a"
            lhs == rhs
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ "a" == "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ "a" != "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ "a" < "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ "a" > "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('{@ "a" <= "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('{@ "a" >= "b" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

        """
        この式はPythonではTrueになる
        CではFalseだ
        PHP,Rubyではパースエラーになる
        == 演算子の結果が bool（または int）であることを考えればこの式の結果は False になるべきだという印象を受ける
        しかし、ぱっと見た感じでは True が正しいようにも見える
        Cap ではこれは実装上の簡易さから False として扱う
        """
        a.parse(t.parse('{@ "a" == "a" == "a" @}'))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 0)

    def test_ast_for(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        # a.parse(t.parse('''{@
        #     for ;;:
        #     end
        # @}'''))
        # c = a.traverse()

        a.parse(t.parse('''{@
            i = 0
            for ; i < 10;:
                i = i + 1
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 10)

        a.parse(t.parse('''{@
            for i = 0; i < 10; ++i:
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 10)

        a.parse(t.parse('''{@
            for i = 10; i > 0; --i:
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 0)

        a.parse(t.parse('''{@
            v = 0
            for i = 0; i < 10; ++i:
                v = v + i
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 10)
        self.assertEqual(a.current_scope.syms['v'], 45)

        a.parse(t.parse('''{@ for i = 0; i < 10; ++i: @}a{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 10)
        self.assertEqual(c.buffer, 'aaaaaaaaaa')

        a.parse(t.parse('''{@ for i = 0; i < 2; ++i: @}{@ for j = 0; j < 2; ++j: @}a{@ end @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 2)
        self.assertEqual(a.current_scope.syms['j'], 2)
        self.assertEqual(c.buffer, 'aaaa')

        ts = t.parse('''{@
            for i = 0; i < 1; ++i:
            end
@}i = {{ i }}''')
        a.parse(ts)
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 1)
        self.assertEqual(c.buffer, 'i = 1')

        a.parse(t.parse('''{@
            v = 0
            for i = 0; i < 1; ++i:
                v = v + i
            end
@}v = {{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 1)
        self.assertEqual(c.buffer, 'v = 0')

        a.parse(t.parse('''{@
            v = 0
            for i = 0, j = 0; i < 4; ++i, ++j:
                v = v + i
            end
@}v = {{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['i'], 4)
        self.assertEqual(a.current_scope.syms['j'], 4)
        self.assertEqual(c.buffer, 'v = 6')

    def test_ast_not_expr(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('{@ v = !0 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 1)

        a.parse(t.parse('{@ v = !1 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], False)

        a.parse(t.parse('{@ v = !0 + 1 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], False) # Fix me on the C implementation

        a.parse(t.parse('{@ v = (!0) + 1 @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)

        a.parse(t.parse('{@ v = !"str" @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], False)

    def test_ast_if(self):
        if self.silent: return
        
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('{@ if 1: @}abc{@ end @}'))
        c = a.traverse()
        self.assertEqual(c.buffer, 'abc')

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ if 1 @}{@ end @}'))

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ if @}{@ end @}'))

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ if 1: @}{@ @}'))

        a.parse(t.parse('{@ if 1: v = "v" end @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v')

        a.parse(t.parse('{@ if 0: v = "v" else: v = "v2" end @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')

        a.parse(t.parse('{@ if 0: v = "v" elif 1: v = "v2" end @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')

        a.parse(t.parse('{@ if 0: v = "v" elif 0: v = "v2" else: v = "v3" end @}'))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v3')

        a.parse(t.parse('''{@
            if 1:
                v = "s"
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 's')

        a.parse(t.parse('''{@
            if 1:
                v = "a"
            elif 2:
                v = "b"
            else:
                v = "c"
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    v = "a"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 0:
            else:
                if 2:
                    v = "abc"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'abc')

        a.parse(t.parse('''{@ if 1: @}{@ end @}'''))
        c = a.traverse()

        a.parse(t.parse('''{@ if 0: @}{@ elif 1: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@ if 0: @}{@ elif 0: @}{@ else: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@ v = "a" @}{@ if 1: @}{@ if 2: @}{{ v }}{@ end @}{@ end @}bbb'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')
        self.assertEqual(c.buffer, "abbb")

        a.parse(t.parse('''{@ v = "a" @}{@ if 1: @}{{ v }}{{ v }}{@ end @}'''))

        a.parse(t.parse('''{@
    v = "cat"
    if 1:
        @}{{ v }}{@
    end
    if 1:
    end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'cat')
        self.assertEqual(c.buffer, 'cat')

        a.parse(t.parse('''{@ v = "a" @}
{@ if 1: @}
{{ v }}
{@ end @}
bbb'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')
        self.assertEqual(c.buffer, "\n\na\n\nbbb")

        a.parse(t.parse('''{@
    v = "a"
    if 1:
        v = "b"
        @}{{ v }}{@
    end
@}
c'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'b')
        self.assertEqual(c.buffer, 'b\nc')

        a.parse(t.parse('''{@ if 0: @}{@ else: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}abc{@ end @}'''))
        c = a.traverse()

        a.parse(t.parse('''{@ v = "a" @}{@ if 1: @}{{ v }}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')
        self.assertEqual(c.buffer, 'a')

        a.parse(t.parse('''{@ v = "a" @}{@ if 0: @}{@ else: @}{{ v }}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')
        self.assertEqual(c.buffer, 'a')

        a.parse(t.parse('''
            {@ if 0: @}
            {@ else: @}
                {@ v = "a" @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 0: @}
            {@ elif 1: @}
                {@ v = "a" @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}{@ if 2: @}{@ v = "a" @}{@ end @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 2: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 0: @}
                {@ elif 1: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 0: @}
                {@ elif 0: @}
                {@ else: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 0: @}
                {@ elif 1: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    v = "a"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    v1 = "a"
                end
            else:
                if 0:
                elif 4:
                    v2 = "b"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v1'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}{@ if 2: @}{@ v = "a" @}{@ end @}{@ end @}
        '''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@  if 1: if 2: @}{@ v = "a" @}{@ end end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}aaa{@ if 2: @}bbb{@ v = "ccc" @}{{ v }}{@ end @}ddd{@ end @}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'ccc')
        self.assertEqual(c.buffer, 'aaabbbcccddd')

        a.parse(t.parse('''aaa{@ if 1: @}bbb{@ if 2: @}ccc{@ v = "ddd" @}{{ v }}{@ end @}eee{@ end @}fff'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'ddd')
        self.assertEqual(c.buffer, 'aaabbbcccdddeeefff')

        c = a.traverse()
        a.parse(t.parse('''{@
            if 1:
                v = "a"
            elif 0:
                v = "b"
            else:
                v = "c"
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'a')
        self.assertEqual(c.buffer, 'a')

        a.parse(t.parse('''{@
            if 0:
                v = "a"
            elif 1:
                v = "b"
            else:
                v = "c"
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'b')
        self.assertEqual(c.buffer, 'b')

        a.parse(t.parse('''{@
            if 0:
                v = "a"
            elif 0:
                v = "b"
            else:
                v = "c"
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'c')
        self.assertEqual(c.buffer, 'c')

        a.parse(t.parse('''{@
            if 0:
                v1 = "v1"
            elif 1:
                v2 = "v2"
                if 0:
                    v3 = "v3"
                else:
                    v4 = "v4"
                end
            end
@}{{ v4 }}'''))
        c = a.traverse()
        self.assertEqual('v1' not in a.current_scope.syms.keys(), True)
        self.assertEqual(a.current_scope.syms['v2'], 'v2')
        self.assertEqual('v3' not in a.current_scope.syms.keys(), True)
        self.assertEqual(a.current_scope.syms['v4'], 'v4')
        self.assertEqual(c.buffer, 'v4')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                end
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v')
        self.assertEqual(c.buffer, 'v')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                    v = "v2"
                end
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')
        self.assertEqual(c.buffer, 'v2')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                end
                v = "v2"
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')
        self.assertEqual(c.buffer, 'v2')

        a.parse(t.parse('''{@
            if 1:
                v = "v"
                if 2:
                    if 3:
                        v = "v2"
                    end
                end
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')
        self.assertEqual(c.buffer, 'v2')

        a.parse(t.parse('''{@
            if 1:
                v = "v"
                if 2:
                    v = "v2"
                    if 3:
                    end
                end
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 'v2')
        self.assertEqual(c.buffer, 'v2')

        a.parse(t.parse('''{@
            if 1 && 1:
                v = 1
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            if 1 || 1:
                v = 1
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 1)
        self.assertEqual(c.buffer, '1')

        a.parse(t.parse('''{@
            v = 1
            if v && v:
                v = 2
            end
@}{{ v }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['v'], 2)
        self.assertEqual(c.buffer, '2')

        a.parse(t.parse('''{@
            a = 1
            b = 1
            if a && b:
                a = 2
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(a.current_scope.syms['a'], 2)
        self.assertEqual(c.buffer, '2')

    def test_ast_opts(self):
        if self.silent: return
        
        a = AST()
        t = Tokenizer()

        opts = {}
        opts['get-me'] = 'I am superman'

        a.parse(t.parse('{{ opts.get("get-me") }}'))
        c = a.traverse(opts=opts)
        self.assertEqual(c.buffer, 'I am superman')

        a.parse(t.parse('{@ if opts.get("get-me"): @}I am superman{@ end @}'))
        c = a.traverse(opts=opts)
        self.assertEqual(c.buffer, 'I am superman')
