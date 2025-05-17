include config.mk

all: $(TARGET).a
	ar -t $^
	gcc $(CFLAGS) $(INCLUDE_FLAGS) $(PATH_EXAMPLES)/code.c -L. -lLZ77_c -o code.$(EXTENSION)

$(TARGET).a: $(OBJECTS)
	$(ARR) $(ARR_FLAGS) $@ $^
	ranlib $@

lz77.o: $(PATH_SRC)/lz77.c
	$(CC) $(CFLAGS) -c $^ -o $@

cleanobj:
	$(RM) $(RMFLAGS) *.o

cleanall: cleanobj
	$(RM) $(RMFLAGS) *.o *.a \
	$(TARGET_DEBUG).a *.$(EXTENSION) 

.SILENT: clean cleanobj cleanall
.IGNORE: cleanobj cleanall
.PHONY:  cleanobj cleanall