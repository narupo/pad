def TrimDomain(url):
	domain = ''
	urllen = len(url)
	i = 0
	while i < urllen:
		ch = url[i]
		if ch == ':':
			domain = ''
			i += 2
		else:
			if ch == '/':
				break
			domain += ch
		i += 1
	return domain

# print(TrimDomain('http://localhost'))
# print(TrimDomain('http://www.localhost.com'))
# print(TrimDomain('http://www.localhost.com/example/site'))
# print(TrimDomain('www.localhost.com'))
# print(TrimDomain('www.localhost.com/example/site'))
