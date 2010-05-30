from distutils.core import setup, Extension

module = Extension('skip',
                   sources = ['skip.c'],
         )

setup (name = 'pyskip',
       version = '0.1',
       author = 'Ivan Giuliani',
       author_email = 'giuliani.v@gmail.com',
       description = 'A skip list implementation for Python',
       ext_modules = [module])
