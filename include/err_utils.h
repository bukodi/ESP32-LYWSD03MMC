#ifndef __ERR_UTILS_H__
#define __ERR_UTILS_H__
#include <stdlib.h>
#include <assert.h>
#include "esp_log.h"

#define ERROR_RETURN(x) do {                                         \
        int __rc = (x);                                              \
        if ( __rc != 0) {                                            \
            ESP_LOGE(TAG, "%s returned %d. [In function %s, %s:%d]", \
                #x, __rc, __ASSERT_FUNC, __FILE__, __LINE__);        \
            return __rc;                                             \
        }                                                            \
    } while(0)

#define ESP_ERROR_RETURN(x) do {                                        \
        esp_err_t __err_rc = (x);                                       \
        if (__err_rc != ESP_OK) {                                       \
            _esp_error_check_failed_without_abort(__err_rc, __FILE__,   \
                __LINE__, __ASSERT_FUNC, #x);                           \
            return __err_rc;                                            \
        }                                                               \
    } while(0)

#define ERROR_CLEANUP_RETURN(x,y) do {                               \
        int __rc = (x);                                              \
        if ( __rc != 0) {                                            \
            ESP_LOGE(TAG, "%s returned %d. [In function %s, %s:%d]", \
                #x, __rc, __ASSERT_FUNC, __FILE__, __LINE__);        \
            (y);                                                     \
            return __rc;                                             \
        }                                                            \
    } while(0)

#define ESP_ERROR_CLEANUP_RETURN(x,y) do {                              \
        esp_err_t __err_rc = (x);                                       \
        if (__err_rc != ESP_OK) {                                       \
            _esp_error_check_failed_without_abort(__err_rc, __FILE__,   \
                __LINE__, __ASSERT_FUNC, #x);                           \
            (y);                                                        \
            return __err_rc;                                            \
        }                                                               \
    } while(0)

#define ERROR_GOTO(x,y) do {                                         \
        int __rc = (x);                                              \
        if ( __rc != 0) {                                            \
            ESP_LOGE(TAG, "%s returned %d. [In function %s, %s:%d]", \
                #x, __rc, __ASSERT_FUNC, __FILE__, __LINE__);        \
            goto y;                                                  \
        }                                                            \
    } while(0)

#define ESP_ERROR_GOTO(x,y) do {                                        \
        esp_err_t __err_rc = (x);                                       \
        if (__err_rc != ESP_OK) {                                       \
            _esp_error_check_failed_without_abort(__err_rc, __FILE__,   \
                __LINE__, __ASSERT_FUNC, #x);                           \
            goto y;                                                     \
        }                                                               \
    } while(0)

#endif /* __ERR_UTILS_H__ */