BUILDDIR=./builddir
EXE=./builddir/main.exe
INCLUDE=./include/**/*.hh ./include/*.hh
SRC=./src/**/*.cc ./src/*.cc
MESON_CONF=meson.build

@all: ${EXE}

${BUILDDIR}: ${MESON_CONF}
	meson setup ${BUILDDIR}

${EXE}: ${BUILDDIR} ${SRC} ${INCLUDE}
	ninja -j 0 -C ${BUILDDIR}

clean:
	rm -rf ${BUILDDIR}

run: ${EXE}
	${EXE} 

dummy: ${EXE}
	${EXE} dummy-data.bin dummy-queries.bin

contest-1m: ${EXE}
	${EXE} contest-data-release-1m.bin contest-queries-release-1m.bin

contest-10m: ${EXE}
	${EXE} contest-data-release-10m.bin contest-queries-release-10m.bin

