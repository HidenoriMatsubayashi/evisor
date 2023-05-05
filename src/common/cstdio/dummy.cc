extern "C" {
// Fix undefined reference to `atexit'
int atexit(void (*)()) {
  return 0;
}
}
