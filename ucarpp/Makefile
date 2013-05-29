INSTPATH = ../instances\&Results/instances/
OBJS = main.cpp graph.cpp edge.cpp solver.cpp meta.cpp
LIBS = headings.h main.h graph.h edge.h solver.h meta.h

all: clean ucarpp

ucarpp: $(OBJS) $(LIBS)
	$(CXX)	-I/usr/include/boost \
		$(OBJS) \
		-lboost_regex $(LIBS) \
		-std=c++0x -O3
	mv a.out $@

run: ucarpp
	./$< $(INSTPATH)val10A.dat 2 MDF

OURS = ../instances\&Results/ours
INSTANCES = $(shell ls $(INSTPATH))
VEHICLES = 2 3 4
batch: ucarpp
	$(foreach test, \
		$(INSTANCES), \
		$(foreach i, $(VEHICLES), \
			echo $(test)_$(i) ORG... && \
			./$< $(INSTPATH)$(test) $(i) ORG > $(OURS)_ORG/Detailed_Sol_$(test)_$(i).txt 2> /dev/null && \
			echo $(test)_$(i) MDF... && \
			./$< $(INSTPATH)$(test) $(i) MDF > $(OURS)_MDF/Detailed_Sol_$(test)_$(i).txt 2> /dev/null;))

batchBg: ucarpp
	$(foreach test, \
		$(INSTANCES), \
		$(foreach i, $(VEHICLES), \
			./$< $(INSTPATH)$(test) $(i) ORG > $(OURS)_ORG/Detailed_Sol_$(test)_$(i).txt 2> /dev/null & \
			./$< $(INSTPATH)$(test) $(i) MDF > $(OURS)_MDF/Detailed_Sol_$(test)_$(i).txt 2> /dev/null &))

.PHONY: clean
clean:
	rm	-f ucarpp ./*.gch ./*~

first: ucarpp
	./$< $(INSTPATH)val2A.dat 2 2>/dev/null
