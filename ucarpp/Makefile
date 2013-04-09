OBJS = main.cpp graph.cpp edge.cpp
LIBS = main.h graph.h edge.h

ucarpp: $(OBJS) $(LIBS)
	$(CXX)	$(OBJS) $(LIBS) \
	-std=c++11 \
	-o $@

run: ucarpp
	./$< ../../instances\&Results/instances/test1A.dat

