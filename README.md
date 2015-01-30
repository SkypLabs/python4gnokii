# python4gnokii

python4gnokii is a Python bindings for the [Gnokii][1] project.

## Dependencies

To build and make the Python module work, you need the following elements :

 * **libgnokii** and its headers
 * The **[setuptools][2]** package

### On Fedora

    yum install gnokii gnokii-devel python-setuptools

### On Debian

    aptitude install gnokii-cli libgnokii-dev python-setuptools

## Installation

    git clone git@github.com:SkypLabs/python4gnokii.git
    cd python4gnokii
    python setup.py install

## How to

The best way to learn how it works is to look at the examples available in the "examples" folder.

## License

[GPL version 3][3]

  [1]: http://gnokii.org/
  [2]: https://pypi.python.org/pypi/setuptools
  [3]: https://www.gnu.org/licenses/gpl.txt "GPL version 3"
