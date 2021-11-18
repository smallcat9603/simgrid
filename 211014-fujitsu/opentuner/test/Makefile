
help:
	@echo "do make test"

test:	jansel-opentuner
	python mytest.py --no-dups --stop-after=3600

jansel-opentuner:
	git clone https://github.com/jansel/opentuner $@

clean:
	rm -rf *.pyc *~ jansel-opentuner opentuner.*
