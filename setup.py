from distutils.core import setup, Extension
 
module1 = Extension('gnokii',
	libraries = ['gnokii'],
	library_dirs = ['/usr/local/lib/'],
	sources = ['python4gnokii.c'])
 
setup (name = 'python4gnokii',
	version = '0.1.0',
	description = 'Python for Gnokii',
	author = 'Paul-Emmanuel Raoul',
	author_email = 'skyper@skyplabs.net',
	url = 'http://blog.skyplabs.net',
	ext_modules = [module1])
