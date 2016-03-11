# Makefile for building autobouquetsreader

#CXXFLAGS += -g -Wall -O2

ABR := autobouquetsreader

.PHONY: default
default: $(ABR)

$(ABR): $(ABR).cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
	$(STRIP) $@

.PHONY: clean
clean:
	rm -f $(ABR)

