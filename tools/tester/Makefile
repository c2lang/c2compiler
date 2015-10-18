CFLAGS=-Wall -Wextra -Wno-unused -Wno-unused-parameter -Wshadow -O2
C2C_DIR= ../../c2c

SOURCES = \
    main.cpp \
    $(C2C_DIR)/FileUtils/FileMap.cpp \
    $(C2C_DIR)/Utils/StringBuilder.cpp

HEADERS = \
    $(C2C_DIR)/FileUtils/FileMap.h \
    $(C2C_DIR)/Utils/StringBuilder.h \
    $(C2C_DIR)/Utils/color.h

tester: $(SOURCES) $(HEADERS)
	g++ $(CFLAGS) -I $(C2C_DIR) $(SOURCES) -o tester
	strip tester

clean:
	rm -f *.o tester


