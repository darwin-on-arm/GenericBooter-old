
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x84000040;
  .text      :
  {
    *(.text)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
  } =0
  _etext = .;
  PROVIDE (etext = .);
  .init      : { *(.init)    } =0
  .fini      : { *(.fini)    } =0
  .rodata    : { *(.rodata) *(.gnu.linkonce.r*) }
  .rodata1   : { *(.rodata1) }
  .sdata2    : { *(.sdata2)  }
  .sbss2     : { *(.sbss2)   }
  /* Adjust the address for the data segment to the next page up. */
  . = ((. + 0x1000) & ~(0x1000 - 1));
  .data    :
  {
    *(.data)
    *(.gnu.linkonce.d*)
    CONSTRUCTORS
  }
  .data1   : { *(.data1) }
  .got1           : { *(.got1) }
  .dynamic        : { *(.dynamic) }
  /* Put .ctors and .dtors next to the .got2 section, so that the pointers
     get relocated with -mrelocatable. Also put in the .fixup pointers.
     The current compiler no longer needs this, but keep it around for 2.7.2  */
                PROVIDE (_GOT2_START_ = .);
  .got2           :  { *(.got2) }
                PROVIDE (__CTOR_LIST__ = .);
  .ctors          : { *(.ctors) }
                PROVIDE (__CTOR_END__ = .);
                PROVIDE (__DTOR_LIST__ = .);
  .dtors          : { *(.dtors) }
                PROVIDE (__DTOR_END__ = .);
                PROVIDE (_FIXUP_START_ = .);
  .fixup          : { *(.fixup) }
                PROVIDE (_FIXUP_END_ = .);
                PROVIDE (_GOT2_END_ = .);
                PROVIDE (_GOT_START_ = .);
  .got            : { *(.got) }
  .got.plt        : { *(.got.plt) }
                PROVIDE (_GOT_END_ = .);
  /* We want the small data sections together, so single-instruction 
offsets
     can access them all, and initialized data all before uninitialized, 
so
     we can shorten the on-disk segment size.  */
  .sdata     : { *(.sdata) }
  _edata  =  .;
  PROVIDE (edata = .);
  .sbss      :
  {
    PROVIDE (__sbss_start = .);
    *(.sbss)
    *(.scommon)
    *(.dynsbss)
    PROVIDE (__sbss_end = .);
  }
  .plt   : { *(.plt) }
  .bss       :
  {
   PROVIDE (__bss_start = .);
   *(.dynbss)
   *(.bss)
   *(COMMON)
  }
  . = _start + 32768;
  __end = . ;
  .mach : { mach.o }
  .xmdt : { xmdt.o }
  

  PROVIDE (__end = .);
}

