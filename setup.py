#!/usr/bin/env python3

from setuptools import setup, Extension
from os.path import dirname, abspath, join
from codecs import open as fopen

DIR = dirname(abspath(__file__))
VERSION = '0.3.0'

gnokii = Extension(
    'gnokii',
    sources = [
        DIR + '/src/python4gnokii.c',
    ],
    libraries = [
        'gnokii',
    ],
    library_dirs = [
        '/usr/local/lib/',
    ],
)

with fopen(join(DIR, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name = 'python4gnokii',
    version = VERSION,
    description = 'Python bindings for the Gnokii library',
    long_description = long_description,
    license = 'GPLv3',
    keywords = 'gnokii bindings gsm sms',
    author = 'Paul-Emmanuel Raoul',
    author_email = 'skyper@skyplabs.net',
    url = 'https://github.com/SkypLabs/python4gnokii',
    download_url = 'https://github.com/SkypLabs/python4gnokii/archive/v{0}.zip'.format(VERSION),
    classifiers = [
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Programming Language :: C',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
    ],
    ext_modules = [gnokii],
    python_requires='>=3',
)
