from tokens import Token
from tokenizer import Tokenizer
from nodes import *
from ast import AST
import unittest


class Test(unittest.TestCase):
    def test_tokenizer(self):
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
        t = Tokenizer()
        a = AST()

        a.parse(t.parse(''))
        self.assertEqual(a.root, None)

        a.parse(t.parse('abc'))
        c = a.traverse()
        self.assertEqual(a.root.text_block.text, 'abc')

        a.parse(t.parse('{@'))

        a.parse(t.parse('{@ v = "v"'))

        with self.assertRaises(AST.SyntaxError):
            a.parse(t.parse('{@ 1: @}'))

    def test_ast_import(self):
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

        a.parse(t.parse('aaa{@ import alias @}bbb{@ import config @}ccc'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)
        self.assertEqual(c.imported_config, True)

        a.parse(t.parse('{@ import alias @}{@ import config @}'))
        c = a.traverse()
        self.assertEqual(c.imported_alias, True)
        self.assertEqual(c.imported_config, True)

        a.parse(t.parse('''{@
            import alias
            alias.set("dtl", "run bin/date-line/date-line.py")
@}'''))
        c = a.traverse()
        self.assertEqual(c.alias_map['dtl'], 'run bin/date-line/date-line.py')

        a.parse(t.parse('''{@
            import config
            config.set("editor", "subl")
@}'''))
        c = a.traverse()
        self.assertEqual(c.config_map['editor'], 'subl')

    def test_ast_expr(self):
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
        self.assertEqual(c.syms['v'], 3)

        a.parse(t.parse('''{@
            a = 1 + 2
            v = a + 3
        @}'''))
        c = a.traverse()
        self.assertEqual(c.last_expr_val, 6)
        self.assertEqual(c.syms['v'], 6)

    def test_ast_id_expr(self):
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            v = 1
            v++
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            v = 1
            ++v
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            v = 1
            v--
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 1)

        a.parse(t.parse('''{@
            v = 1
            --v
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 0)

        a.parse(t.parse('''{@
            v = 1
            ++v + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 4)

        a.parse(t.parse('''{@
            v = 1
            v++ + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 2)
        self.assertEqual(c.last_expr_val, 3)

        a.parse(t.parse('''{@
            v = 1
            --v + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            v = 1
            v-- + 2
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 3)

    def test_ast_assign(self):
        t = Tokenizer()
        a = AST()

        a.parse(t.parse('''{@
            a = "s"
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['a'], 's')

        a.parse(t.parse('''{@
            a = "s"
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['a'], 's')

        a.parse(t.parse('''{@
            v = "v"
            v = ""
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], '')

        a.parse(t.parse('''{@
            v = "v"
            v = "v2"
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@
            v = "v"
            v = ""
            v = "v2"
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@ v = "v" @}
{@ v = "" @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], '')

        a.parse(t.parse('''{@ v = "v" @}
{@ v = "" @}
{@ v = "v2" @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@ v = "" @}
{@ v = "v" @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v')
        
        a.parse(t.parse('{@ v = 1 + 2 @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 3)

        a.parse(t.parse('{@ v = v = 1 @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 1)

        a.parse(t.parse('{@ v = 1 + 2 * 3 @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 7)

        a.parse(t.parse('{@ v = (1 + 2) * 3 @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 9)

        a.parse(t.parse('{@ v = opts.get("a") @}'))
        c = a.traverse(opts={ 'a': 'b' })
        self.assertEqual(c.syms['v'], "b")

        # 以下の式はPython3ではエラーになる
        # PHP7, Ruby2.3ではエラーにならずlast_expr_valは2
        a.parse(t.parse('''{@
            1 + v = 1
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 1)
        self.assertEqual(c.last_expr_val, 2)

        a.parse(t.parse('''{@
            i = 0
            v = 0
            v = v + i
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 0)
        self.assertEqual(c.last_expr_val, 0)

    def test_ast_comparison(self):
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
        self.assertEqual(c.syms['i'], 10)

        a.parse(t.parse('''{@
            for i = 0; i < 10; ++i:
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 10)

        a.parse(t.parse('''{@
            for i = 10; i > 0; --i:
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 0)

        a.parse(t.parse('''{@
            v = 0
            for i = 0; i < 10; ++i:
                v = v + i
            end
        @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 10)
        self.assertEqual(c.syms['v'], 45)

        a.parse(t.parse('''{@ for i = 0; i < 10; ++i: @}a{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 10)
        self.assertEqual(c.buffer, 'aaaaaaaaaa')

        a.parse(t.parse('''{@ for i = 0; i < 2; ++i: @}{@ for j = 0; j < 2; ++j: @}a{@ end @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 2)
        self.assertEqual(c.syms['j'], 2)
        self.assertEqual(c.buffer, 'aaaa')

        ts = t.parse('''{@
            for i = 0; i < 1; ++i:
            end
@}i = {{ i }}''')
        a.parse(ts)
        c = a.traverse()
        self.assertEqual(c.syms['i'], 1)
        self.assertEqual(c.buffer, 'i = 1')

        a.parse(t.parse('''{@
            v = 0
            for i = 0; i < 1; ++i:
                v = v + i
            end
@}v = {{ v }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['i'], 1)
        self.assertEqual(c.buffer, 'v = 0')

    def test_ast_if(self):
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
        self.assertEqual(c.syms['v'], 'v')

        a.parse(t.parse('{@ if 0: v = "v" else: v = "v2" end @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('{@ if 0: v = "v" elif 1: v = "v2" end @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('{@ if 0: v = "v" elif 0: v = "v2" else: v = "v3" end @}'))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v3')

        a.parse(t.parse('''{@
            if 1:
                v = "s"
            end
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 's')

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
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    v = "a"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 0:
            else:
                if 2:
                    v = "abc"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'abc')

        a.parse(t.parse('''{@ if 1: @}{@ end @}'''))
        c = a.traverse()

        a.parse(t.parse('''{@ if 0: @}{@ elif 1: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@ if 0: @}{@ elif 0: @}{@ else: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@ v = "a" @}{@ if 1: @}{@ if 2: @}{{ v }}{@ end @}{@ end @}bbb'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')
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
        self.assertEqual(c.syms['v'], 'cat')
        self.assertEqual(c.buffer, 'cat')

        a.parse(t.parse('''{@ v = "a" @}
{@ if 1: @}
{{ v }}
{@ end @}
bbb'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')
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
        self.assertEqual(c.syms['v'], 'b')
        self.assertEqual(c.buffer, 'b\nc')

        a.parse(t.parse('''{@ if 0: @}{@ else: @}{@ v = "a" @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}abc{@ end @}'''))
        c = a.traverse()

        a.parse(t.parse('''{@ v = "a" @}{@ if 1: @}{{ v }}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')
        self.assertEqual(c.buffer, 'a')

        a.parse(t.parse('''{@ v = "a" @}{@ if 0: @}{@ else: @}{{ v }}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')
        self.assertEqual(c.buffer, 'a')

        a.parse(t.parse('''
            {@ if 0: @}
            {@ else: @}
                {@ v = "a" @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 0: @}
            {@ elif 1: @}
                {@ v = "a" @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}{@ if 2: @}{@ v = "a" @}{@ end @}{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 2: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 0: @}
                {@ elif 1: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

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
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}
                {@ if 0: @}
                {@ elif 1: @}
                    {@ v = "a" @}
                {@ end @}
            {@ end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    v = "a"
                end
            end
@}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

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
        self.assertEqual(c.syms['v1'], 'a')

        a.parse(t.parse('''
            {@ if 1: @}{@ if 2: @}{@ v = "a" @}{@ end @}{@ end @}
        '''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@  if 1: if 2: @}{@ v = "a" @}{@ end end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@ if 1: @}aaa{@ if 2: @}bbb{@ v = "ccc" @}{{ v }}{@ end @}ddd{@ end @}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'ccc')
        self.assertEqual(c.buffer, 'aaabbbcccddd')

        a.parse(t.parse('''aaa{@ if 1: @}bbb{@ if 2: @}ccc{@ v = "ddd" @}{{ v }}{@ end @}eee{@ end @}fff'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'ddd')
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
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

        a.parse(t.parse('''{@
            if 0:
                v = "a"
            elif 1:
                v = "b"
            else:
                v = "c"
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'b')

        a.parse(t.parse('''{@
            if 0:
                v = "a"
            elif 0:
                v = "b"
            else:
                v = "c"
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'c')

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
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual('v1' not in c.syms.keys(), True)
        self.assertEqual(c.syms['v2'], 'v2')
        self.assertEqual('v3' not in c.syms.keys(), True)
        self.assertEqual(c.syms['v4'], 'v4')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                end
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                    v = "v2"
                end
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@
            if 1:
                if 2:
                    if 3:
                        v = "v"
                    end
                end
                v = "v2"
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@
            if 1:
                v = "v"
                if 2:
                    if 3:
                        v = "v2"
                    end
                end
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

        a.parse(t.parse('''{@
            if 1:
                v = "v"
                if 2:
                    v = "v2"
                    if 3:
                    end
                end
            end
@}{{ a }}'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'v2')

    def test_ast_opts(self):
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
