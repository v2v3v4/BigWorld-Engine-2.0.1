from distutils.core import setup, Extension
import os

BZ2_PATH = "./bz2"
if __file__.startswith( '/' ):
	BZ2_PATH = os.path.join( os.path.basename( __file__ ), "bz2" )

module1 = Extension( "_bsdiff",
	sources = ["bsdiffmodule.cpp", "bslib.cpp"],
	libraries = ['bz2'],
	library_dirs = ["./"],
	include_dirs =[BZ2_PATH]
)

setup( name = "_bsdiff",
	version = "1.0",
	description = "bsdiff python built-in extension",
	ext_modules = [module1] )
