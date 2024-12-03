#define COUNT(x)  (sizeof x / sizeof x[0])
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define UNUSED(x) (void) (x)

void terminate(const char *fmt, ...);
