@all: builddir/main.exe

builddir/src/sigmod/query_set.o: src/sigmod/query_set.cc include/sigmod/query.hh include/sigmod/query_set.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/query_set.o -c src/sigmod/query_set.cc

builddir/src/sigmod/random.o: src/sigmod/random.cc include/sigmod/random.hh include/sigmod/config.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/random.o -c src/sigmod/random.cc

builddir/src/sigmod/record.o: src/sigmod/record.cc include/sigmod/config.hh include/sigmod/record.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/record.o -c src/sigmod/record.cc

builddir/src/sigmod/scoreboard.o: src/sigmod/scoreboard.cc include/sigmod/config.hh include/sigmod/record.hh include/sigmod/query.hh include/sigmod/flags.hh include/sigmod/scoreboard.hh include/sigmod/debug.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/scoreboard.o -c src/sigmod/scoreboard.cc

builddir/src/sigmod/debug.o: src/sigmod/debug.cc include/sigmod/debug.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/debug.o -c src/sigmod/debug.cc

builddir/src/sigmod/database.o: src/sigmod/database.cc include/sigmod/database.hh include/sigmod/record.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/database.o -c src/sigmod/database.cc

builddir/src/sigmod/query.o: src/sigmod/query.cc include/sigmod/query.hh include/sigmod/config.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/sigmod/query.o -c src/sigmod/query.cc

builddir/src/main.o: src/main.cc include/sigmod.hh include/sigmod/record.hh include/sigmod/flags.hh include/sigmod/memory.hh include/sigmod/metrics.hh include/sigmod/debug.hh include/sigmod/query_set.hh include/sigmod/config.hh include/sigmod/query.hh include/sigmod/random.hh include/sigmod/database.hh
	g++ -Iinclude --std=c++17 -O3 -o builddir/src/main.o -c src/main.cc

builddir/main.exe: builddir/src/sigmod/query_set.o builddir/src/sigmod/random.o builddir/src/sigmod/record.o builddir/src/sigmod/scoreboard.o builddir/src/sigmod/debug.o builddir/src/sigmod/database.o builddir/src/sigmod/query.o builddir/src/main.o
	g++ -Iinclude --std=c++17 -O3 -o builddir/main.exe builddir/src/sigmod/query_set.o builddir/src/sigmod/random.o builddir/src/sigmod/record.o builddir/src/sigmod/scoreboard.o builddir/src/sigmod/debug.o builddir/src/sigmod/database.o builddir/src/sigmod/query.o builddir/src/main.o

clean:
	rm -rf builddir
	mkdir builddir
	mkdir builddir/src
	mkdir builddir/src/sigmod

run: builddir/main.exe
	./builddir/main.exe 

