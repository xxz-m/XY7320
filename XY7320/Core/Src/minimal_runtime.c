/*
 * Minimal hooks required by newlib's __libc_init_array when the firmware is
 * linked with -nostartfiles. The STM32 startup file owns Reset_Handler, data
 * copy, and bss clear, so these hooks intentionally do nothing.
 */

void _init(void)
{
}

void _fini(void)
{
}
