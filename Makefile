PROGS=marc_grep
CCC=g++
CCOPTS=-g -std=gnu++11 -Wall -Wextra -Werror -Wunused-parameter -O3 -c

%.o: %.cc
	$(CCC) $(CCOPTS) $<


all: $(PROGS)

marc_grep: marc_grep.o libmarc.a
	$(CCC) -o $@ $< -L. -lmarc -lpcre

marc_grep.o: marc_grep.cc MarcUtil.h DirectoryEntry.h Leader.h RegexMatcher.h util.h StringUtil.h
	$(CCC) $(CCOPTS) $<

libmarc.a: Subfields.o RegexMatcher.o Leader.o StringUtil.o DirectoryEntry.o MarcUtil.o util.o
	@echo "Linking $@..."
	@ar cqs $@ $^

Subfields.o: Subfields.cc Subfields.h util.h
	$(CCC) $(CCOPTS) $<

RegexMatcher.o: RegexMatcher.cc RegexMatcher.h util.h
	$(CCC) $(CCOPTS) $<

Leader.o: Leader.cc Leader.h StringUtil.h
	$(CCC) $(CCOPTS) $<

StringUtil.o: StringUtil.cc StringUtil.h
	$(CCC) $(CCOPTS) $<

DirectoryEntry.o: DirectoryEntry.cc DirectoryEntry.h StringUtil.h util.h
	$(CCC) $(CCOPTS) $<

MarcUtil.o: MarcUtil.cc MarcUtil.h DirectoryEntry.h Leader.h
	$(CCC) $(CCOPTS) $<


clean:
	rm -f *~ $(PROGS) *.o
