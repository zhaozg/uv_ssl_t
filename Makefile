
SRC	= src/bio.c src/link_methods.c src/ringbuffer.c src/uv_ssl_t.c \
	  ../uv_link/src/uv_link_observer_t.c ../uv_link/src/uv_link_t.c \
	  ../uv_link/src/defaults.c ../uv_link/src/uv_link_source_t.c \
	  test/src/main.c test/src/test-error-on-eof.c test/src/test-try-write.c \
	  test/src/test-close-in-read-cb.c test/src/test-error.c test/src/test-read-incoming.c \
	  test/src/test-write.c test/src/test-handshake.c test/src/test-shutdown.c \
	  test/src/test-large.c

all:
	${CC} -o ssl -g $(SRC) -Iinclude -I../uv_link/include -I$(HOME)/.usr/openssl_1.0.2/include \
		-I/usr/local/include -L/usr/local/lib -luv \
		-L$(HOME)/.usr/openssl_1.0.2/lib -lcrypto -lssl
	${CC} -o rb -g test/ringbuffer.c src/ringbuffer.c -Isrc

