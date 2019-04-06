class Token:
    def __init__(self, kind='', value=''):
        self.kind = kind
        self.value = value

    def __str__(self):
        return '<{0},{1}>'.format(self.kind, self.value)