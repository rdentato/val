TESTDIR=val vec

all:
	@echo "CD to the proper test directory to make tests"

runtest:
	@for f in $(TESTDIR) ; do \
	  cd $$f ; make runtest ; cd .. ;\
	done
	
clean:
	@for f in $(TESTDIR) ; do \
	  cd $$f ; make clean ; cd .. ;\
	done
	