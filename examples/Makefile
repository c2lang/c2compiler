all:
	@echo "---- building examples ----"
	@../output/c2c/c2c --quiet

errors:
	@( grep -n 'error:' `find . -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

warnings:
	@( grep -n '[[]-W' `find . -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

clean:
	rm -rf output
