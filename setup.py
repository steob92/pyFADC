from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
from Cython.Build import cythonize

# Remove the "-Wstrict-prototypes" compiler option, which isn't valid for C++.
import distutils.sysconfig
cfg_vars = distutils.sysconfig.get_config_vars()
for key, value in cfg_vars.items():
    if type(value) == str:
        cfg_vars[key] = value.replace("-Wstrict-prototypes", "")


'''
extensions = [Extension('pyFADC',
              sources=['pyFADC.pyx'],
              include_dirs=['./'],
              extra_compile_args=['-std=c++11','-D_LINUX', '-lAqDrv4',
                                  '-lAgMD1', '-L ../lib/','-I ./'],
              language='c++')]
'''

setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = [Extension('pyFADC',
              sources=['pyFADC.pyx'],
              include_dirs=['./'],
              extra_compile_args=['-std=c++11','-D_LINUX', '-lAqDrv4',
                                  '-lAgMD1', '-L ../lib/','-I ./'],
              language='c++')]

)
