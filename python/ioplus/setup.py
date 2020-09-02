import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="libioplus",
    version="1.0.1",
    author="Sequent Microsystems",
    author_email="olcitu@gmail.com",
    description="A set of functions to control Sequent Microsystems IO_PLUS board",
	license='MIT',
    url="https://www.sequentmicrosystems.com",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 2/3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
)