OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  flash (rxai!w) : ORIGIN = 0x20400000, LENGTH = 512M
  ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 16K
}

PHDRS
{
  flash PT_LOAD;
  ram_init PT_LOAD;
  ram PT_NULL;
}

SECTIONS
{
  .text           :
  {
    *(.text)
  } >flash AT>flash :flash

  .data          :
  {
    *(.data)
  } >ram AT>flash :ram_init

  .bss            :
  {
    *(.bss)
    . = ALIGN(4);
  } >ram AT>ram :ram

  .stack ORIGIN(ram) + LENGTH(ram) - 2K :
  {
    PROVIDE( _heap_end = . );
    . = 2K;
    PROVIDE( _sp = . );
  } >ram AT>ram :ram
}

