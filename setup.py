from distutils.core import setup, Extension
 
gnokii = Extension(
	'gnokii',
	libraries = ['gnokii'],
	library_dirs = ['/usr/local/lib/'],
	sources = ['python4gnokii.c']
)
 
setup(
	name = 'python4gnokii',
	version = '0.1.0',
	description = 'Python bindings for the Gnokii library',
	author = 'Paul-Emmanuel Raoul',
	author_email = 'skyper@skyplabs.net',
	url = 'https://github.com/SkypLabs/python4gnokii',
	ext_modules = [gnokii],
)
