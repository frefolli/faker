BUILDDIR=./builddir
EXE=./builddir/main.exe
INCLUDE=./include
SRC=./src
MESON_CONF=meson.build

@all: ${EXE}

${BUILDDIR}: ${MESON_CONF}
	meson setup ${BUILDDIR}

${EXE}: ${BUILDDIR} ${SRC} ${INCLUDE}
	ninja -j 0 -C ${BUILDDIR}

run: ${EXE}
	${EXE}
