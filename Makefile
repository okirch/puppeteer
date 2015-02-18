CXXFLAGS= -Wall -I/usr/include/Qt -O0 -g

LIB	= libpuppeteer.so
LIBSRCS	= puppeteer.cpp puppeteer_moc.cpp \
	  script.cpp namespace.cpp

APP	= hello-world
APPSRCS	= hello-world.cpp hello-world_moc.cpp

APPOBJS	= $(addprefix obj/,$(APPSRCS:.cpp=.o))
LIBOBJS	= $(addprefix obj.shared/,$(LIBSRCS:.cpp=.o))

all: $(APP)

hello-world: $(APPOBJS) $(LIB)
	$(CXX) -o $@ $(LDFLAGS) $(APPOBJS) -L. -lpuppeteer -lQtGui -lQtXml

libpuppeteer.so: $(LIBOBJS)
	$(CXX) -o $@ -shared $(LIBOBJS) -lQtGui -lQtXml

obj.shared/%.o: %.cpp
	@mkdir -p obj.shared
	$(CXX) -c -o $@ $(CXXFLAGS) -fPIC $<

obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) -c -o $@ $(CXXFLAGS) $<

clean:
	rm -f $(APP) $(LIB) *_moc.cpp
	rm -rf obj.shared obj
	rm -f core

%_moc.cpp: %.h
	moc -o$@ $<
