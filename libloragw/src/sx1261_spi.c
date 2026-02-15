/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <string.h>     /* memset */

#include <sys/ioctl.h>
#if defined(__FreeBSD__)
#include <sys/spigenio.h>
#elif defined(__linux__)
#include <linux/spi/spidev.h>
#else
#error "SPI header not available for this platform"
#endif

#include "loragw_spi.h"
#include "loragw_aux.h"
#include "sx1261_spi.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_LBT == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define WAIT_BUSY_SX1250_MS  1

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1261_spi_w(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) {
    int com_device;
    int cmd_size = 1; /* op_code */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
#if defined(__linux__)
    struct spi_ioc_transfer k;
#elif defined(__FreeBSD__)
    struct spigen_transfer st;
    struct iovec cmd_iov, data_iov;
#else
#error "unimplemented"
#endif
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
#if defined(__linux__)
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.len = command_size;
    k.speed_hz = SPI_SPEED;
    k.cs_change = 0;
    k.bits_per_word = 8;
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);
#elif defined(__FreeBSD__)
    memset(&st, 0, sizeof(st));
    cmd_iov.iov_base = out_buf;
    cmd_iov.iov_len = command_size;
    data_iov.iov_base = NULL;
    data_iov.iov_len = 0;
    st.st_command = cmd_iov;
    st.st_data = data_iov;
    a = ioctl(com_device, SPIGENIOC_TRANSFER, &st);
#else
#error "unimplemented"
#endif

    /* determine return code */
#if defined(__linux__)
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI write success\n");
        return LGW_SPI_SUCCESS;
    }
#elif defined(__FreeBSD__)
    if (a < 0) {
        DEBUG_MSG("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI write success\n");
        return LGW_SPI_SUCCESS;
    }
#else
#error "unimplemented"
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1261_spi_r(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) {
    int com_device;
    int cmd_size = 1; /* op_code */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];
#if defined(__linux__)
    struct spi_ioc_transfer k;
#elif defined(__FreeBSD__)
    struct spigen_transfer st;
    struct iovec cmd_iov, data_iov;
#endif
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
#if defined(__linux__)
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.rx_buf = (unsigned long) in_buf;
    k.len = command_size;
    k.cs_change = 0;
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);
#elif defined(__FreeBSD__)
    memset(&st, 0, sizeof(st));
    cmd_iov.iov_base = out_buf;
    cmd_iov.iov_len = command_size;
    data_iov.iov_base = in_buf;
    data_iov.iov_len = command_size;
    st.st_command = cmd_iov;
    st.st_data = data_iov;
    a = ioctl(com_device, SPIGENIOC_TRANSFER, &st);
#else
#error "unimplemented"
#endif

    /* determine return code */
#if defined(__linux__)
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        memcpy(data, in_buf + cmd_size, size);
        return LGW_SPI_SUCCESS;
    }
#elif defined(__FreeBSD__)
    if (a < 0) {
        DEBUG_MSG("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        memcpy(data, in_buf + cmd_size, size);
        return LGW_SPI_SUCCESS;
    }
#else
#error "unimplemented"
#endif
}

/* --- EOF ------------------------------------------------------------------ */
