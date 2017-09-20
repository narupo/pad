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

class Tokenizer():

    def __init__(self):
        self.toks = []
        self.buf = ''
        self.pos = 0

    def parse(self, fin):
        raise Exception()

    def cur(self, ofs=0):
        if self.pos+ofs >= len(self.toks) or self.pos+ofs < 0:
            return None
        return self.toks[self.pos+ofs]
    
    def get(self):
        if self.pos >= len(self.toks):
            return None
        t = self.toks[self.pos]
        self.pos += 1
        return t

    def append(self, tok):
        self.toks.append(tok)

    def insert(self, ofs, tok):
        self.toks.insert(ofs, tok)

class SpaceTokenizer(Tokenizer):

    def __init__(self):
        super().__init__()

    def parse(self, fin):
        s = fin.read()
        s = s.replace('(', ' ( ').replace(')', ' ) ')
        self.toks = s.split()

class CapTokenizer(Tokenizer):

    def __init__(self):
        super().__init__()
        self.stream = None

    def _push_buf(self):
        if len(self.buf):
            self.toks.append(str(self.buf))
            self.buf = ''

    def parse(self, fin):
        self.stream = Stream(fin.read())
        self.toks = []
        self.pos = 0
        self.buf = ''
        m = 0

        while not self.stream.eof():
            c = self.stream.get()

            if m == 0: # first
                if c.isspace():
                    self._push_buf()
                elif c == '+':
                    self._push_buf()
                    m = 100
                elif c in ['=', ':', ';', '<', '>', '(', ')', '.', ',']:
                    self._push_buf()
                    self.buf = c
                    self._push_buf()
                else:
                    self.buf += c
            elif m == 100: # +
                if c == '+':
                    self.buf = '++'
                    self._push_buf()
                    m = 0
                else:
                    self.stream.prev()
                    self.buf = '+'
                    self._push_buf()
                    m = 0

        self._push_buf()

    def append(self, tok):
        self.toks.append(tok)
