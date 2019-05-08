"""
{@
    import config
    config.set("editor", "subl")

    import run
    linux_script = "/virtualenv/bin/python"
    windows_script = "C:\\virtualenv/bin/python"
    if os.getName() == 'Windows':
        run.bind(windows_script, "bin/script.py")
    elif os.getName() == 'Linux':
        run.bind(linux_script, "bin/script")
    else:
        print('error')
    end
@}

{{ linux_script }}

"""
from stream import Stream
from tokens import Token


class Tokenizer:
    class ParseError(RuntimeError):
        pass

    class ModuleError(RuntimeError):
        pass

    def __init__(self, ldbrace_value='{{', rdbrace_value='}}'):
        self.strm = None
        self.tokens = []

        if len(ldbrace_value) != 2:
            raise ValueError('invalid ldbrace value "%s"', ldbrace_value)
        if len(rdbrace_value) != 2:
            raise ValueError('invalid rdbrace value "%s"', rdbrace_value)
        self.ldbrace_value = ldbrace_value
        self.rdbrace_value = rdbrace_value

    def show_parse(self, *args, **kwargs):
        if self.debug_parse:
            print(*args, **kwargs)

    def parse(self, src, debug=False):
        self.debug_parse = debug
        self.strm = Stream(src)
        self.tokens = []

        s = self.strm
        m = 'text block'
        buf = ''
        while not s.eof():
            c = s.get()
            self.show_parse('m', m, ', c', c)
            if m == 'text block':
                if c == '{' and s.cur() == '@':
                    if len(buf):
                        self.tokens.append(Token(kind='text-block', value=buf))
                        buf = ''
                    s.get()
                    self.tokens.append(Token(kind='lbraceat', value='{@'))
                    m = 'code block'
                elif c == self.ldbrace_value[0] and s.cur() == self.ldbrace_value[1]:
                    if len(buf):
                        self.tokens.append(Token(kind='text-block', value=buf))
                        buf = ''
                    s.get()
                    self.tokens.append(Token(kind='ldbrace', value=self.ldbrace_value))
                    m = 'ref block'
                else:
                    buf += c
            elif m == 'ref block':
                if c == self.rdbrace_value[0] and s.cur() == self.rdbrace_value[1]:
                    s.get()
                    self.tokens.append(Token(kind='rdbrace', value=self.rdbrace_value))
                    m = 'text block'
                elif c.isdigit():
                    s.prev()
                    self.read_digit()
                elif self.is_identifier_char(c):
                    s.prev()
                    self.read_identifier()
                elif c == '(':
                    self.tokens.append(Token(kind='lparen', value='('))
                elif c == ')':
                    self.tokens.append(Token(kind='rparen', value=')'))
                elif c == '.':
                    self.tokens.append(Token(kind='operator', value='.'))
                elif c == ',':
                    self.tokens.append(Token(kind='comma', value=','))
                elif c == '"':
                    s.prev()
                    self.read_string()
            elif m == 'code block':
                if c == '@' and s.cur() == '}':
                    s.get()
                    self.tokens.append(Token(kind='rbraceat', value='@}'))
                    m = 'text block'
                elif c == '\n':
                    s.prev()
                    self.read_newline()
                elif c == '\r' and s.cur() == '\n':
                    s.prev()
                    self.read_newline()
                elif c == '.':
                    self.tokens.append(Token(kind='operator', value='.'))
                elif c == ',':
                    self.tokens.append(Token(kind='comma', value=','))
                elif c == '(':
                    self.tokens.append(Token(kind='lparen', value='('))
                elif c == ')':
                    self.tokens.append(Token(kind='rparen', value=')'))
                elif c == ':':
                    self.tokens.append(Token(kind='colon', value=':'))
                elif c == ';':
                    self.tokens.append(Token(kind='semicolon', value=';'))
                elif c == '=':
                    s.prev()
                    self.read_assign()
                elif c == '!':
                    s.prev()
                    self.read_not()
                elif c == '<':
                    s.prev()
                    self.read_lt()
                elif c == '>':
                    s.prev()
                    self.read_gt()
                elif c == '+':
                    s.prev()
                    self.read_add()
                elif c == '-':
                    s.prev()
                    self.read_sub()
                elif c == '*':
                    s.prev()
                    self.read_mul()
                elif c == '/':
                    if s.cur() == '/':
                        s.get()
                        self.read_at_newline()
                    elif s.cur() == '*':
                        s.get()
                        self.read_at_starslash()
                    else:
                        s.prev()
                        self.read_div()
                elif c == '%':
                    s.prev()
                    self.read_mod()
                elif c == '&':
                    if s.cur() == '&':
                        s.get()
                        self.tokens.append(Token(kind='operator', value='&&'))
                    else:
                        self.tokens.append(Token(kind='operator', value='&'))
                elif c == '|':
                    if s.cur() == '|':
                        s.get()
                        self.tokens.append(Token(kind='operator', value='||'))
                    else:
                        self.tokens.append(Token(kind='operator', value='|'))
                elif c == '"':
                    s.prev()
                    self.read_string()
                elif c.isdigit():
                    s.prev()
                    self.read_digit()
                elif self.is_identifier_char(c):
                    s.prev()
                    self.read_identifier()
                elif c.isspace():
                    pass
                else:
                    raise Tokenizer.ParseError('unsupported character "%s"' % c)

        if len(buf):
            self.tokens.append(Token(kind='text-block', value=buf))
            buf = ''

        return self.tokens

    def read_newline(self):
        c = self.strm.get()
        if c == '\n':
            self.tokens.append(Token(kind='newline', value='\n'))
        elif c == '\r':
            c = self.strm.get()
            if c == '\n':
                self.tokens.append(Token(kind='newline', value='\r\n'))
            else:
                raise AST.ModuleError('impossible. not found "\n" in read newline')
        else:
            raise AST.ModuleError('impossible. invalid character in read newline')

    def read_at_newline(self):
        while not self.strm.eof():
            c = self.strm.get()
            if c == '\r' and self.strm.cur() == '\n':
                self.strm.get()
                return
            elif c == '\n':
                return

    def read_at_starslash(self):
        while not self.strm.eof():
            c = self.strm.get()
            if c == '*' and self.strm.cur() == '/':
                self.strm.get()
                return

    def read_add(self):
        c = self.strm.get()
        if c != '+':
            raise Tokenizer.ModuleError('not found "+"')

        c = self.strm.get()
        if c == '+':
            self.tokens.append(Token(kind='operator', value='++'))
        elif c == '=':
            self.tokens.append(Token(kind='operator', value='+='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='operator', value='+'))

    def read_sub(self):
        c = self.strm.get()
        if c != '-':
            raise Tokenizer.ModuleError('not found "-"')

        c = self.strm.get()
        if c == '-':
            self.tokens.append(Token(kind='operator', value='--'))
        elif c == '=':
            self.tokens.append(Token(kind='operator', value='-='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='operator', value='-'))

    def read_mul(self):
        c = self.strm.get()
        if c != '*':
            raise Tokenizer.ModuleError('not found "*"')

        if self.strm.cur() == '=':
            self.strm.get()
            self.tokens.append(Token(kind='operator', value='*='))
        else:
            self.tokens.append(Token(kind='operator', value='*'))

    def read_div(self):
        c = self.strm.get()
        if c != '/':
            raise Tokenizer.ModuleError('not found "/"')

        if self.strm.cur() == '=':
            self.strm.get()
            self.tokens.append(Token(kind='operator', value='/='))
        else:
            self.tokens.append(Token(kind='operator', value='/'))

    def read_mod(self):
        c = self.strm.get()
        if c != '%':
            raise Tokenizer.ModuleError('not found "%"')

        if self.strm.cur() == '=':
            self.strm.get()
            self.tokens.append(Token(kind='operator', value='%='))
        else:
            self.tokens.append(Token(kind='operator', value='%'))


    def read_mod(self):
        c = self.strm.get()
        if c != '%':
            raise Tokenizer.ModuleError('not found "%"')

        if self.strm.cur() == '=':
            self.strm.get()
            self.tokens.append(Token(kind='operator', value='%='))
        else:
            self.tokens.append(Token(kind='operator', value='%'))

    def read_gt(self):
        c = self.strm.get()
        if c != '>':
            raise Tokenizer.ModuleError('not found ">"')

        c = self.strm.get()
        if c == '=':
            self.tokens.append(Token(kind='comp_op', value='>='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='comp_op', value='>'))

    def read_lt(self):
        c = self.strm.get()
        if c != '<':
            raise Tokenizer.ModuleError('not found "<"')

        c = self.strm.get()
        if c == '=':
            self.tokens.append(Token(kind='comp_op', value='<='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='comp_op', value='<'))

    def read_not(self):
        c = self.strm.get()
        if c != '!':
            raise Tokenizer.ModuleError('not found "!"')

        c = self.strm.get()
        if c == '=':
            self.tokens.append(Token(kind='comp_op', value='!='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='comp_op', value='!'))

    def read_digit(self):
        buf = ''
        while not self.strm.eof():
            c = self.strm.get()
            if not c.isdigit():
                self.strm.prev()
                break
            buf += c

        if not len(buf):
            raise Tokenizer.ModuleError('not found digit')

        self.tokens.append(Token(kind='digit', value=int(buf)))

    def read_assign(self):
        c = self.strm.get()
        if c != '=':
            raise Tokenizer.ModuleError('not found "="')

        c = self.strm.get()
        if c == '=':
            self.tokens.append(Token(kind='comp_op', value='=='))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='operator', value='='))

    def is_identifier_char(self, c):
        return c.isalpha() or c.isdigit() or c == '_'

    def read_identifier(self):
        t = Token(kind='identifier', value='')
        while not self.strm.eof():
            c = self.strm.get()
            if not self.is_identifier_char(c):
                self.strm.prev()
                break
            else:
                t.value += c

        if not len(t.value):
            raise Tokenizer.ModuleError('not found identifier') # impossible

        if t.value == 'import':
            t.kind = 'import'
        elif t.value == 'for':
            t.kind = 'for'
        elif t.value == 'if':
            t.kind = 'if'
        elif t.value == 'elif':
            t.kind = 'elif'
        elif t.value == 'else':
            t.kind = 'else'
        elif t.value == 'end':
            t.kind = 'end'
        elif t.value == 'def':
            t.kind = 'def'
        elif t.value == 'return':
            t.kind = 'jmp'

        self.tokens.append(t)

    def read_string(self):
        c = self.strm.get()
        if c != '"':
            raise Tokenizer.ModuleError("not found '"'')

        t = Token(kind='string', value='')
        while not self.strm.eof():
            c = self.strm.get()
            if c == '"':
                break
            else:
                t.value += c

        self.tokens.append(t)

    def read_at(self):
        c = self.strm.get()
        if c != '@':
            raise Tokenizer.ModuleError('not found "@"')

        c = self.strm.get()
        if c == '}':
            self.tokens.append(Token(kind='rbraceat', value='@}'))
        else:
            raise Tokenizer.ParseError('invalid token. "@" is not supported')
