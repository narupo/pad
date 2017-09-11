class Tokenizer():

    def parse(self, fin): raise Exception()
    def cur(self): raise Exception()
    def get(self): raise Exception()

class SpaceTokenizer(Tokenizer):

    def __init__(self):
        self.toks = []
        self.pos = 0

    def parse(self, fin):
        s = fin.read()
        s = s.replace('(', ' ( ').replace(')', ' ) ')
        self.toks = s.split()

    def cur(self):
        if self.pos >= len(self.toks) or self.pos < 0:
            return None
        return self.toks[self.pos]
    
    def get(self):
        if self.pos >= len(self.toks):
            return None
        t = self.toks[self.pos]
        self.pos += 1
        return t
