/**
 ******************************************************************************
 * @file    bsp.h
 * @brief   BSP layer common header
 ******************************************************************************
 */

#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* BSP initialization status */
typedef enum {
    BSP_OK = 0,
    BSP_ERROR = 1
} BSP_Status_t;

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
