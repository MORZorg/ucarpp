INSTPATH = ../instances\&Results/instances/
OBJS = main.cpp graph.cpp edge.cpp meta.cpp solver.cpp solution.cpp vehicle.cpp
LIBS = headings.h main.h graph.h edge.h meta.h solver.h solution.h vehicle.h

all: clean ucarpp

ucarpp: $(OBJS) $(LIBS)
	$(CXX)	-I/usr/include/boost \
		$(OBJS) \
		-lboost_regex $(LIBS) \
		-std=c++0x -O3
	mv a.out $@

run: ucarpp
		./$< $(INSTPATH)val1C.dat 4 VND

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
