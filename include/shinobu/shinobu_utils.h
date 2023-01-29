#ifndef SHINOBU_UTILS_H
#define SHINOBU_UTILS_H

#ifdef SHINOBU_DEBUG
#define __SHINOBU_PRINT_ERR(error_message) \
	print_line(vformat("%s (%s:%d)", error_message, __FILE__, __LINE__));
#else
#define __SHINOBU_PRINT_ERR(error_message)
#endif

#define MA_ERR_RET(result, string)                                                 \
	if (result != MA_SUCCESS) {                                                    \
		error_message = vformat("%s (%s)", string, ma_result_description(result)); \
		__SHINOBU_PRINT_ERR(error_message);                                        \
		return FAILED;                                                             \
	}

#define MA_ERR(result, string)                                                     \
	if (result != MA_SUCCESS) {                                                    \
		error_message = vformat("%s (%s)", string, ma_result_description(result)); \
		__SHINOBU_PRINT_ERR(error_message);                                        \
	}

#endif