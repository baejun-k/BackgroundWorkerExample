#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#if defined(_DEBUG)
#include <string.h>

#define __FILENAME__ ( strrchr(__FILE__, '\\') ? \
						strrchr(__FILE__, '\\') + 1 : strrchr(__FILE__, '/') ? \
							strrchr(__FILE__, '/') + 1 : __FILE__ )

#define DebugOut(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define DebugDetailOut(fmt, ...) fprintf(stderr, "[%s::Lines:%d::%s] "##fmt, \
												  __FILENAME__, \
												  __LINE__, \
												  __func__, \
												  __VA_ARGS__)
#else
#define DebugOut(fmt, ...) 
#define DebugDetailOut(fmt, ...) 
#endif

#endif // !__DEBUG_UTILS_H__
