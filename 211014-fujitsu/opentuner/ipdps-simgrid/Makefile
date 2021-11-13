
help:
	@echo "do make test"

test:	jansel-opentuner
	python ipdps-simgrid.py

jansel-opentuner:
	git clone https://github.com/jansel/opentuner $@

clean:
	rm -rf *.pyc *~ jansel-opentuner opentuner.*
