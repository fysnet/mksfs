/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef FYSOS_MISC
#define FYSOS_MISC

// set it to 1 (align on byte)
#pragma pack (1)

// huge file seek and ftell
#ifdef _MSC_VER
  extern "C" __int64 __cdecl _ftelli64(FILE *);
  extern "C" int __cdecl _fseeki64(FILE *, __int64, int);
  #define FSEEK(f, o, s) _fseeki64(f, (o), s)
  #define FTELL(f) _ftelli64(f)
#elif defined DJGPP
  #define _FILE_OFFSET_BITS  64  
  #define FSEEK(f, o, s) fseek(f, (o), s)
  #define FTELL(f) ftell(f)
#else
# error 64 bit ftell(), fseek() not defined
#endif


#define PART_START      63  // default base address

struct PART_TBLE_CHS {
  bit8u  head;    // 8 bit head count
  bit8u  sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  bit8u  cyl;     // bottom 8 bits
};

struct PART_TBLE {
  bit8u  bi;
  struct PART_TBLE_CHS start_chs;
  bit8u  si;
  struct PART_TBLE_CHS end_chs;
  bit32u startlba;
  bit32u size;
};

// Convert LBA to CHS.  If LBA >= (1024 * spt * heads) use max values */
#define _SECTOR ((lba % 63) + 1)
#define _HEAD   ((lba / 63) % 16)
#define _CYL    ((lba / 63) / 16)
// Converts LBA to the BIOS form of CHS with the high 2 bits of the cylinder
//  in the high 2 bits of the sector field
void lba_to_chs(struct PART_TBLE_CHS *chs, const bit32u lba) {
  if (lba < (1024 * 16 * 63)) {
    chs->head = (bit8u) _HEAD;
    chs->sector = (bit8u) (((_CYL & 0x300) >> 2) | (_SECTOR & 0x3F));
    chs->cyl = (bit8u) (_CYL & 0x0FF);
  } else {
    chs->head = 0xFE;
    chs->sector = 0xFF;
    chs->cyl = 0xFF;
  }
}


#define NAME_LEN_MAX  512

// resource file contents (after we parse it)
struct S_RESOURCE {
    char mbr_filename[NAME_LEN_MAX];
    char boot_filename[NAME_LEN_MAX];
    char targ_filename[NAME_LEN_MAX];
  bit32u file_cnt;
  bit64u base_lba;
  bit64u tot_sectors;
  int    param0;
  int    param1;
  int    heads;
  int    spt;
  struct {
    char path_filename[NAME_LEN_MAX];
    char filename[NAME_LEN_MAX];
    bit32u param;
  } files[1];  // allocated later
};

void get_a_string(FILE *fp, char *str) {
  char ch, *t = str;
  int c = NAME_LEN_MAX - 1;
  
  // skip all leading spaces
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == ' ')
      continue;
    else
      break;
  }
  
  while (1) {
    if (feof(fp))
      break;
    if (ch == 13)
      continue;
    if (strchr("\xA,=#", ch))
      break;
    *t++ = ch;
    if (--c == 0)
      break;
    ch = fgetc(fp);
  }
  
  // if it was the '#' char, skip to eol
  if (ch == '#') {
    while (!feof(fp)) {
      ch = fgetc(fp);
      if (ch == 10)
        break;
    }
  }
  
  // kill all trailing spaces
  while (t > str) {
    t--;
    if (*t != ' ') {
      t++;
      break;
    }
  }
  
  // asciiz it
  *t = 0;
}

/*
 *  # line is a comment
 *  mbrfile=filename   - This is the path/filename of the mbr file to use 
 *  bootfile=filename  - This is the path/filename of the boot code file to use
 *  imgfile=filename   - This is the path/filename of the target file to create/modify
 *  base_lba=0         - Base LBA to write the Super Block to
 *  tot_sects=0        - Total sectors to "allocate"
 *  path=path_to_source_file, filename=filename_to_use, param
 *
 */
struct S_RESOURCE *parse_resource(const char *filename) {
  struct S_RESOURCE *r;
  int cnt = 0;
  int limit = 10;  // start with allowing 10 pairs
  char str[NAME_LEN_MAX];
  FILE *fp;
  
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("\nError opening resource file.");
    return NULL;
  }
  
  r = (struct S_RESOURCE *) calloc(sizeof(struct S_RESOURCE) - sizeof(r->files) + (limit * sizeof(r->files)), 1);
  r->base_lba = 0;                // assuming none given
  r->tot_sectors = (20*16*63);    // default size
  r->param0 = 12;                 // 
  
  while (!feof(fp)) {
    get_a_string(fp, str);
    if (strcmp(str, "mbrfile") == 0) {
      get_a_string(fp, str);
      strcpy(r->mbr_filename, str);
    } else if (strcmp(str, "bootfile") == 0) {
      get_a_string(fp, str);
      strcpy(r->boot_filename, str);
    } else if (strcmp(str, "imgfile") == 0) {
      get_a_string(fp, str);
      strcpy(r->targ_filename, str);
    } else if (strcmp(str, "base_lba") == 0) {
      get_a_string(fp, str);
      r->base_lba = strtoul(str, NULL, 0);
    } else if (strcmp(str, "tot_sects") == 0) {
      get_a_string(fp, str);
      r->tot_sectors = strtoul(str, NULL, 0);
    } else if (strcmp(str, "param0") == 0) {
      get_a_string(fp, str);
      r->param0 = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "param1") == 0) {
      get_a_string(fp, str);
      r->param1 = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "heads") == 0) {
      get_a_string(fp, str);
      r->heads = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "spt") == 0) {
      get_a_string(fp, str);
      r->spt = (bit8u) strtoul(str, NULL, 0);
    } else if (strlen(str) > 0) {
      strcpy(r->files[cnt].path_filename, str);
      get_a_string(fp, r->files[cnt].filename);
      get_a_string(fp, str);
      r->files[cnt].param = strtoul(str, NULL, 0);
      cnt++;
      if (cnt == limit) {
        limit += 10;
        r = (struct S_RESOURCE *) realloc(r, sizeof(struct S_RESOURCE) - sizeof(r->files) + (limit * sizeof(r->files)));
      }
    }
  }
  
  // close the file
  fclose(fp);
  
  r->file_cnt = cnt;
  return r;
}

#endif // FYSOS_MISC
