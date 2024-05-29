TESTDIRS=val vec buf

all:
	@echo "Test directories: $(TESTDIRS)"

runtest:
	@for f in $(TESTDIRS) ; do \
	  cd $$f ; make runtest ; cd .. ;\
	done
	
clean:
	@for f in $(TESTDIRS) ; do \
	  cd $$f ; make clean ; cd .. ;\
	done
	