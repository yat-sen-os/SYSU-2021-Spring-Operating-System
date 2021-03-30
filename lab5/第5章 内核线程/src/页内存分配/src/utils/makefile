C_COMPLIER = gcc
CXX_COMPLIER = g++

INCLUDE_PATH = ../../include

bitmap_test.out : bitmap.cpp $(INCLUDE_PATH)/bitmap.h
	@$(CXX_COMPLIER) -o bitmap_test -I$(INCLUDE_PATH) test.cpp bitmap.cpp

clean:
	@rm -f *.out
