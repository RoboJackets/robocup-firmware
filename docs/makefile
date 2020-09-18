.PHONY : docs
docs:
	mkdir -p build
	cd build && cmake ../ ./ && make sphinx
	@echo "\n=> Open up 'build/docs_build/sphinx_docs/index.html' in a browser to view a local copy of the documentation"

clean:
	rm -rf build
