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

    def test_ast(self):
        t = Tokenizer()
        a = AST()

        a.parse(t.parse(''))
        self.assertEqual(a.root, None)

        a.parse(t.parse('abc'))
        c = a.traverse()

        a.parse(t.parse('{@ import alias @}'))
        c = a.traverse()

        a.parse(t.parse('aaa{@ import alias @}bbb{@ import config @}ccc'))
        c = a.traverse()

        a.parse(t.parse('''{@
            import alias
            alias.set("dtl", "run bin/date-line/date-line.py")
@}'''))
        c = a.traverse()

        a.parse(t.parse('''{@
            import config
            config.set("editor", "subl")
@}'''))
        c = a.traverse()

        a.parse(t.parse('''{@
            a = "s"
@}'''))
        c = a.traverse()

        a.parse(t.parse('''{@
            a = "s"
@}{{ a }}'''))
        c = a.traverse()

        a.parse(t.parse('''{@
            if 1:
                v = "s"
            end
@}'''))
        c = a.traverse()

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
            {@  if 1:
                    if 2: @}
                        {@ v = "a" @}
            {@      end
                end @}
'''))
        c = a.traverse()
        self.assertEqual(c.syms['v'], 'a')

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