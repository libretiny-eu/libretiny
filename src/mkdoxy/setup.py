from setuptools import setup, find_packages

def readme():
    with open('README.md') as f:
        return f.read()

setup(
    name='mkdoxy',
    version='1.0.0',
    description='MkDoxy -> MkDocs + Doxygen. Easy documentation with code snippets.',
    long_description=readme(),
    long_description_content_type='text/markdown',
    keywords=['python', 'open-source', 'documentation', 'mkdocs', 'doxygen', 'multilanguage'],
    url='https://github.com/JakubAndrysek/mkdoxy',
    author='Jakub Andrýsek',
    author_email='email@kubaandrysek.cz',
    license='MIT',
    python_requires='>=3.7',
    install_requires=[
        'mkdocs',
        'Jinja2',
        'ruamel.yaml',
    ],
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Operating System :: OS Independent',
    ],
    packages=find_packages(),
    package_data={'mkdoxy': ['templates/*.jinja2']},
    entry_points={
        'mkdocs.plugins': [
            'mkdoxy = mkdoxy.plugin:MkDoxy'
        ]
    }
)
