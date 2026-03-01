SECTIONS {
  /DISCARD/ : {*(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .eh_frame.*)
               *(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .eh_frame_hdr)
               *(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .eh_frame_entry)
               *(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .eh_frame_entry.*)
               *(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .eh_frame)}
}
