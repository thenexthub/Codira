# cmpcodesize/setup.py - Install script for cmpcodesize -*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import os

import cmpcodesize

import setuptools

# setuptools expects to be invoked from within the directory of setup.py,
# but it is nice to allow `python path/to/setup.py install` to work
# (for scripts, etc.)
os.chdir(os.path.dirname(os.path.abspath(__file__)))

setuptools.setup(
    name="cmpcodesize",
    version=cmpcodesize.__version__,

    author=cmpcodesize.__author__,
    author_email=cmpcodesize.__email__,
    url='https://language.org',
    license='Apache',

    description="A tool to compare the size of Codira compiler build products.",
    keywords='compare size language',

    test_suite='tests',

    classifiers=[
        'Development Status :: 3 - Alpha',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Topic :: Software Development :: Compilers',
    ],

    zip_safe=False,
    packages=setuptools.find_packages(),
    entry_points={
        'console_scripts': [
            'cmpcodesize = cmpcodesize:main',
        ],
    }
)
