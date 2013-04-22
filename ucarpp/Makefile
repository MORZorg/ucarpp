OBJS = main.cpp graph.cpp edge.cpp solver.cpp
LIBS = headings.h main.h graph.h edge.h solver.h

ucarpp: $(OBJS) $(LIBS)
	c++	-I/usr/include/boost \
		$(OBJS) \
		-lboost_regex $(LIBS) \
		-std=c++0x \
		-o $@

run: ucarpp
	./$< ../instances\&Results/instances/test1A.dat

