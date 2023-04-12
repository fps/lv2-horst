# Available at setup time due to pyproject.toml
# from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, Extension
import pkgconfig

__version__ = "0.0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

# pkgs = pkgconfig.parse('lilv-0 lv2 jack')

ext_modules = [
    Extension("horst",
        ["src/horst_python.cc"],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        include_dirs = ["src/include"] + pkgs['include_dirs'],
        library_dirs = pkgs['library_dirs'],
        extra_link_args = pkgs['extra_link_args'] +  ["-latomic", "-pthread"],
        libraries = pkgs['libraries']
        ),
]

pkgconfig.configure_extension(ext_modules[0])

setup(
    name="horst",
    version=__version__,
    author="Florian Paul Schmidt",
    author_email="mista.tapas@gmx.net",
    url="https://github.com/fps/horst",
    description="Python bindings for horst",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
