// Wrapper to redirect XMP-Toolkit-SDK's hardcoded expat include to the system expat.
// Using angle brackets so the search skips this wrapper (found via the deep relative
// path "third-party/expat/lib/expat.h") and finds the system expat.h directly.
#include <expat.h>
