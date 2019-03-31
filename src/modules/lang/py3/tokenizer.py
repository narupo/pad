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

    def parse(self, src):
        self.strm = Stream(src)
        self.tokens = []

        s = self.strm
        m = 'text block'
        buf = ''
        while not s.eof():
            c = s.get()
            if m == 'text block':
                if c == '{' and s.cur() == '@':
                    if len(buf):
                        self.tokens.append(Token(kind='text-block', value=buf))
                        buf = ''
                    s.get()
                    self.tokens.append(Token(kind='lbraceat', value='{@'))
                    m = 'code block'
                elif c == '{' and s.cur() == '{':
                    if len(buf):
                        self.tokens.append(Token(kind='text-block', value=buf))
                        buf = ''
                    s.get()
                    self.tokens.append(Token(kind='ldbrace', value='{{'))
                    m = 'ref block'
                else:
                    buf += c
            elif m == 'ref block':
                if c == '}' and s.cur() == '}':
                    s.get()
                    self.tokens.append(Token(kind='rdbrace', value='}}'))
                    m = 'text block'                    
            elif m == 'code block':
                if c == '@' and s.cur() == '}':
                    s.get()
                    self.tokens.append(Token(kind='rbraceat', value='@}'))
                    m = 'text block'
                elif c == '=':
                    s.prev()
                    self.read_assign()
                elif c == '.':
                    self.tokens.append(Token(kind='operator', value='.'))
                elif c == ',':
                    self.tokens.append(Token(kind='comma', value=','))
                elif c == '(':
                    self.tokens.append(Token(kind='lparen', value='('))
                elif c == ')':
                    self.tokens.append(Token(kind='rparen', value=')'))
                elif c == '"':
                    s.prev()
                    self.read_string()
                elif self.is_identifier_char(c):
                    s.prev()
                    self.read_identifier()
                elif c.isspace():
                    pass
                else:
                    raise Tokenizer.ParseError('unsupprted character %s' % c)

        if len(buf):
            self.tokens.append(Token(kind='text-block', value=buf))
            buf = ''

        return self.tokens

    def read_assign(self):
        c = self.strm.get()
        if c != '=':
            raise Tokenizer.ModuleError('not found "="')

        c = self.strm.get()
        if c == '=':
            self.tokens.append(Token(kind='compare', value='=='))
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
        elif t.value == 'if':
            t.kind = 'if'
        elif t.value == 'elif':
            t.kind = 'elif'
        elif t.value == 'else':
            t.kind = 'else'

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

    def read_lbrace(self):
        c = self.strm.get()
        if c != '{':
            raise Tokenizer.ModuleError('not found "{"')

        c = self.strm.get()
        if c == '{':
            self.tokens.append(Token(kind='ldbrace', value='{{'))
        elif c == '@':
            self.tokens.append(Token(kind='lbraceat', value='{@'))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='lbrace', value='{'))

    def read_rbrace(self):
        c = self.strm.get()
        if c != '}':
            raise Tokenizer.ModuleError('not found "}"')

        c = self.strm.get()
        if c == '}':
            self.tokens.append(Token(kind='rdbrace', value='}}'))
        else:
            self.strm.prev()
            self.tokens.append(Token(kind='rbrace', value='}'))

    def read_at(self):
        c = self.strm.get()
        if c != '@':
            raise Tokenizer.ModuleError('not found "@"')

        c = self.strm.get()
        if c == '}':
            self.tokens.append(Token(kind='rbraceat', value='@}'))
        else:
            raise Tokenizer.ParseError('invalid token. "@" is not supported')
