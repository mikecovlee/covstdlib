HEADER := covstdlib.h

TEST_UNIT := \
	covlist \
	covstdlib \
	covswitcher \
	covtuple \
	copyFile

CXXFLAGS += -std=c++11 -frtti -fexceptions -fPIC --pie -O2

all: $(TEST_UNIT)

covlist: $(HEADER) Testcovlist.cpp
	$(CXX) $(CXXFLAGS) -o $@ Testcovlist.cpp

covstdlib: $(HEADER) Testcovstdlib.cpp
	$(CXX) $(CXXFLAGS) -o $@ Testcovstdlib.cpp

covswitcher: $(HEADER) Testcovswitcher.cpp
	$(CXX) $(CXXFLAGS) -o $@ Testcovswitcher.cpp

covtuple: $(HEADER) Testcovtuple.cpp
	$(CXX) $(CXXFLAGS) -o $@ Testcovtuple.cpp

copyFile: $(HEADER) TestcopyFile.cpp
	$(CXX) $(CXXFLAGS) -o $@ TestcopyFile.cpp



clean:
	$(RM) $(TEST_UNIT)

