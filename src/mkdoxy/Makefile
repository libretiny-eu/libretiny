.PHONY: package release

all: package

package:
	rm -f dist/*
	python3 setup.py sdist bdist_wheel

install: package
	pip3 install --no-deps --force dist/*.whl

release: package
	twine upload --repository pypi dist/*

release-test: package
	twine upload --repository testpypi dist/*

clean:
	rm -rf dist build