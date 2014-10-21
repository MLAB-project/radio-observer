
ifeq ($(UNAME),Darwin)
	LDFLAGS += -framework jackmp
else
	LDFLAGS += $(shell pkg-config --libs jack)
endif


client: client.c
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


clean:
	rm -f client client.o


