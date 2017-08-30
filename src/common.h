#ifndef TC__COMMON_H

#define TC__COMMON_H

#include <stdio.h>

#define _TC_KB(x) ((size_t)(x) << 10)
#define _TC_MB(x) ((size_t)(x) << 20)

#define _TC_INFO(message, ...)                                                 \
	do {                                                                   \
		fprintf(stderr, "[info] ");                                    \
		fprintf(stderr, message, ##__VA_ARGS__);                       \
		fprintf(stderr, "\n");                                         \
	} while (0)

#ifdef DEBUG
#define _TC_DEBUG(message, ...)                                                \
	do {                                                                   \
		fprintf(stderr, "[debug] ");                                   \
		fprintf(stderr, message, ##__VA_ARGS__);                       \
		fprintf(stderr, "\n");                                         \
	} while (0)
#else
#define _TC_DEBUG(message, ...)                                                \
	do {                                                                   \
	} while (0)
#endif

#define _TC_MUST(condition, message, ...)                                      \
	do {                                                                   \
		if (!(condition)) {                                            \
			fprintf(stderr, "Error:\n  ");                         \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, message, ##__VA_ARGS__);               \
			fprintf(stderr, "\n\n");                               \
			fprintf(stderr, "  File: %s \n", __FILE__);            \
			fprintf(stderr, "  Line: %d \n", __LINE__);            \
			fprintf(stderr, "\n");                                 \
			fprintf(stderr, "Aborting.\n");                        \
			exit(EXIT_FAILURE);                                    \
		}                                                              \
	} while (0)

#define _TC_MUST_GO(condition, label, message, ...)                            \
	do {                                                                   \
		if (!(condition)) {                                            \
			fprintf(stderr, "Error:\n  ");                         \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, message, ##__VA_ARGS__);               \
			fprintf(stderr, "\n\n");                               \
			fprintf(stderr, "  File: %s \n", __FILE__);            \
			fprintf(stderr, "  Line: %d \n", __LINE__);            \
			fprintf(stderr, "\n");                                 \
			fprintf(stderr, "Aborting.\n");                        \
			goto label;                                            \
		}                                                              \
	} while (0)

#define _TC_MUST_P(condition, pmessage, message, ...)                          \
	do {                                                                   \
		if (!(condition)) {                                            \
			fprintf(stderr, "Error:\n  ");                         \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, message, ##__VA_ARGS__);               \
			fprintf(stderr, "\n\n");                               \
			fprintf(stderr, "  File: %s \n", __FILE__);            \
			fprintf(stderr, "  Line: %d \n", __LINE__);            \
			fprintf(stderr, "\n");                                 \
			fprintf(stderr, "System Error:\n  ");                  \
			perror(pmessage);                                      \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, "Aborting.\n");                        \
			exit(EXIT_FAILURE);                                    \
		}                                                              \
	} while (0)

#define _TC_MUST_P_GO(condition, pmessage, label, message, ...)                \
	do {                                                                   \
		if (!(condition)) {                                            \
			fprintf(stderr, "Error:\n  ");                         \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, message, ##__VA_ARGS__);               \
			fprintf(stderr, "\n\n");                               \
			fprintf(stderr, "  File: %s \n", __FILE__);            \
			fprintf(stderr, "  Line: %d \n", __LINE__);            \
			fprintf(stderr, "\n");                                 \
			fprintf(stderr, "System Error:\n  ");                  \
			perror(pmessage);                                      \
			fprintf(stderr, "\n  ");                               \
			fprintf(stderr, "Aborting.\n");                        \
			goto label;                                            \
		}                                                              \
	} while (0)

#endif
