#ifndef DISPLAY_VFD_H
#define DISPLAY_VFD_H

#include "main.h"

// ...existing code...

// Define comandos do display
#define VFD_CMD_CLEAR_DISPLAY    0x01
#define VFD_CMD_RETURN_HOME      0x02
#define VFD_CMD_SET_ADDRESS      0x80
#define VFD_CMD_ADDRESS_MODE     0x06
#define VFD_CMD_DISPLAY_MODE     0x38
#define VFD_CMD_DISPLAY_CONTROL  0x0C

// Protótipos de funções
void vfd_init(void);
void vfd_clear(void);
void vfd_write_command(uint8_t cmd, uint8_t data);
void vfd_write_data(uint8_t data);
void vfd_set_cursor(uint8_t x, uint8_t y);
void vfd_print_at(uint8_t x, uint8_t y, char* str);
// ...existing code...

#endif // DISPLAY_VFD_H
