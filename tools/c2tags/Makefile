CFLAGS=-Wall -Wextra -Wshadow -O2
C2C_DIR= ../../c2c

SOURCES = \
    main.cpp \
    TagReader.cpp \
    $(C2C_DIR)/Builder/RootFinder.cpp \
    $(C2C_DIR)/FileUtils/FileMap.cpp

HEADERS = \
    TagReader.h \
    $(C2C_DIR)/Builder/RootFinder.h \
    $(C2C_DIR)/Builder/BuilderConstants.h \
    $(C2C_DIR)/FileUtils/FileMap.h

c2tags: $(SOURCES) $(HEADERS)
	g++ $(CFLAGS) -I $(C2C_DIR) $(SOURCES) -o c2tags
	strip c2tags

clean:
	rm -f c2tags

