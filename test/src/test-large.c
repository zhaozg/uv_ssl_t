#include "test-common.h"
#include <assert.h>

static int write_cb_called;
static int test_arg;
static size_t large_size = 16*1024*1024;
static char *wbuf = NULL;
static int  roff = 0;

static void write_client() {
  char *rbuf = malloc(large_size);
  int bytes = 0;
  int err=0;

  while (err==0 && roff < large_size)
  {
    bytes = SSL_read(client.ssl, rbuf+roff, large_size - roff);
    if (bytes>0)
    {
      roff += bytes;
      continue;
    }

    err = SSL_get_error(client.ssl, bytes);
    switch (err)
    {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_X509_LOOKUP:
    case SSL_ERROR_WANT_CONNECT:
    case SSL_ERROR_WANT_ACCEPT:
      err = 0;
      break;
    case SSL_ERROR_ZERO_RETURN:
      err = 1;
      break;
    case SSL_ERROR_SSL:
    case SSL_ERROR_SYSCALL:
    default:
      err = 1;
      break;
    }
  }
  assert(roff==large_size);
  assert(memcmp(rbuf, wbuf, roff)==0);
}


static void write_cb(uv_link_t* link, int status, void* arg) {
  CHECK_EQ(status, 0, "write_cb() status");
  CHECK_EQ(link, (uv_link_t*) &server.observer,
           "write_cb() link must be server");
  CHECK_EQ(arg, &test_arg, "write_cb() link arg be &test_arg");

  write_cb_called++;
}


static void write_info_cb(const SSL* ssl, int where, int val) {
  if ((where & SSL_CB_HANDSHAKE_DONE) != 0) {
    CHECK_EQ(uv_link_read_stop((uv_link_t*) &server.observer), 0,
             "uv_link_read_stop(server)");
  }
}


static void write_server() {
  uv_link_t* serv;
  uv_buf_t buf;
  size_t len;
  FILE *f;

  wbuf = malloc(large_size);
  f = fopen("/dev/random", "rb");
  len = fread(wbuf, 1, large_size, f);
  assert(len==large_size);
  fclose(f);

  serv = (uv_link_t*) &server.observer;
  SSL_set_info_callback(server.ssl, write_info_cb);
  CHECK_EQ(uv_run(loop, UV_RUN_DEFAULT), 0, "uv_run()");

  buf = uv_buf_init(wbuf, large_size);
  CHECK_EQ(uv_link_write(serv, &buf, 1, NULL, write_cb, &test_arg), 0,
           "uv_link_write(server)");

  CHECK_EQ(uv_run(loop, UV_RUN_DEFAULT), 0, "uv_run()");
  CHECK_EQ(write_cb_called, 1, "number of write_cb's");
}


TEST_IMPL(large) {
  ssl_client_server_test(write_client, write_server);
}
