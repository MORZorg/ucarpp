OBJS = main.cpp graph.cpp edge.cpp solver.cpp
LIBS = headings.h main.h graph.h edge.h solver.h

ucarpp: $(OBJS) $(LIBS)
	$(CXX)	-I/usr/include/boost \
		$(OBJS) \
		-lboost_regex $(LIBS) \
		-std=c++0x
	mv a.out $@

run: ucarpp
	./$< ../instances\&Results/instances/test1A.dat

