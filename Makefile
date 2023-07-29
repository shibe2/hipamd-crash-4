# several copies of a module for autodetected architecture
NATIVE_LIBS=native1.so native2.so native3.so native4.so native5.so native6.so native7.so native8.so native9.so

# modules for different architectures
ARCH_LIBS=gfx801.so gfx802.so gfx803.so gfx900.so gfx902.so gfx904.so gfx906.so gfx908.so gfx90a.so gfx90c.so gfx1010.so gfx1011.so gfx1012.so gfx1013.so gfx1030.so gfx1031.so gfx1032.so gfx1033.so gfx1034.so gfx1035.so

all: app $(NATIVE_LIBS) $(ARCH_LIBS)

test: all

# this passes
	./app $(NATIVE_LIBS)

# this crashes
	./app $(ARCH_LIBS)

app: app.cpp
	c++ -o $@ $<

native%.so: lib.cpp
	hipcc -o $@ -shared $<

gfx%.so: lib.cpp
	hipcc --offload-arch=gfx$* -o $@ -shared $<

clean:
	rm -f app $(NATIVE_LIBS) $(ARCH_LIBS)

.PHONY: all test clean
