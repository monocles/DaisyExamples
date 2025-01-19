#include "voice_manager.h"

// Определяем буферы в SDRAM здесь
__attribute__((section(".sdram_bss"))) char voice_buffers[8][16384];