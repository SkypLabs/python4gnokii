=============
python4gnokii
=============

|Build Status| |Code Coverage|

python4gnokii is a Python bindings for the
`Gnokii <http://gnokii.org/>`__ project.

Dependencies
============

To build and make the Python module work, you need the following
elements :

-  libgnokii and its headers
-  The `setuptools <https://pypi.python.org/pypi/setuptools>`__
   package

On Fedora
---------

::

    yum install gnokii gnokii-devel python-setuptools

On Debian
---------

::

    aptitude install gnokii-cli libgnokii-dev python-setuptools

Installation
============

With pip (recommanded)
----------------------

::

    pip install python4gnokii

From sources
------------

::

    git clone git@github.com:SkypLabs/python4gnokii.git
    cd python4gnokii
    python setup.py install

How to
======

The best way to learn how it works is to look at the examples available
in the
`examples <https://github.com/SkypLabs/python4gnokii/tree/master/examples>`__
folder.

License
=======

`GPL version 3 <https://www.gnu.org/licenses/gpl.txt>`__

.. |Build Status| image:: https://travis-ci.org/SkypLabs/python4gnokii.svg
   :target: https://travis-ci.org/SkypLabs/python4gnokii
.. |Code Coverage| image:: https://api.codacy.com/project/badge/Grade/3989785db9e346a3a9c7f872dd0a61d8
   :target: https://www.codacy.com/app/skyper/python4gnokii?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SkypLabs/python4gnokii&amp;utm_campaign=Badge_Grade
