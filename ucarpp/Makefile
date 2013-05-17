INSTPATH = ../instances\&Results/instances/
OBJS = main.cpp graph.cpp edge.cpp solver.cpp meta.cpp
LIBS = headings.h main.h graph.h edge.h solver.h meta.h

all: clean ucarpp

ucarpp: $(OBJS) $(LIBS)
	$(CXX)	-I/usr/include/boost \
		$(OBJS) \
		-lboost_regex $(LIBS) \
		-std=c++0x
	mv a.out $@

run: ucarpp
	./$< $(INSTPATH)test1A.dat

OURS = ../instances\&Results/ours/
INSTANCES = $(shell ls $(INSTPATH))
VEHICLES = 2 3 4
batch: ucarpp
	$(foreach test, \
		$(INSTANCES), \
		$(foreach i, $(VEHICLES), \
			./$< $(INSTPATH)$(test) $(i) \
				> $(OURS)Detailed_Sol_$(test)_$(i).txt \
				2> /dev/null;))

.PHONY: clean
clean:
	rm	-f ucarpp ./*.gch ./*~
