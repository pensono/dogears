#!/usr/bin/env python3

from setuptools import setup

setup(name='dogears',
      version='1.0',
      description='Python driver for the dog ears cape',
      author='Ethan Shea',
      author_email='ethan.shea1@gmail.com',
      packages=['dogears'],
      data_files=[
          ('bin', '../../bin/pydogears.so')
      ]
     )
