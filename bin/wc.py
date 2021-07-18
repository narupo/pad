import os


def wc(path):
    fin = open(path, 'r')
    return len(fin.readlines())

n = 0

for curdir, dirs, files in os.walk('.'):
    for f in files:
        path = os.path.join(curdir, f)
        _, ext = os.path.splitext(path)
        if ext == '.c' or ext == '.h':
            n += wc(path)

print(n)