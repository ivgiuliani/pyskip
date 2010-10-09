from distutils.core import setup, Extension

module = Extension(
    "skip",
    sources = ["src/skip.c"],
)

setup(  name = "pyskip",
        version = "0.1",
        author = "Ivan Giuliani",
        author_email = "giuliani.v@gmail.com",
        description = "A skip list implementation for Python",
        url="http://github.com/kratorius/pyskip",
        license="GPLv3",
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Environment :: Console",
            "Intended Audience :: Developers",
            "Intended Audience :: Science/Research",
            "License :: OSI Approved :: GNU General Public License (GPL)",
            "Operating System :: POSIX",
            "Operating System :: POSIX :: Linux",
            "Operating System :: Microsoft",
            "Operating System :: MacOS",
            "Programming Language :: C",
            "Topic :: Software Development :: Libraries",
        ],
        ext_modules = [module]
)
