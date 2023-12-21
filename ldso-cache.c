/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

/*
 * Stolen from glibc (sysdeps/generic/dl-cache.h)
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#ifndef LD_SO_CACHE
# define LD_SO_CACHE SYSCONFDIR "/ld.so.cache"
#endif

#define CACHEMAGIC "ld.so-1.7.0"

/* libc5 and glibc 2.0/2.1 use the same format.  For glibc 2.2 another
   format has been added in a compatible way:
   The beginning of the string table is used for the new table:
	old_magic
	nlibs
	libs[0]
	...
	libs[nlibs-1]
	pad, new magic needs to be aligned
	     - this is string[0] for the old format
	new magic - this is string[0] for the new format
	newnlibs
	...
	newlibs[0]
	...
	newlibs[newnlibs-1]
	string 1
	string 2
	...
*/
struct file_entry
{
  int32_t flags;		/* This is 1 for an ELF library.  */
  uint32_t key, value;		/* String table indices.  */
};

struct cache_file
{
  char magic[sizeof CACHEMAGIC - 1];
  unsigned int nlibs;
  struct file_entry libs[0];
};

#define CACHEMAGIC_NEW "glibc-ld.so.cache"
#define CACHE_VERSION "1.1"
#define CACHEMAGIC_VERSION_NEW CACHEMAGIC_NEW CACHE_VERSION

struct file_entry_new
{
  union
  {
    /* Fields shared with struct file_entry.  */
    struct file_entry entry;
    /* Also expose these fields directly.  */
    struct
    {
      int32_t flags;		/* This is 1 for an ELF library.  */
      uint32_t key, value;	/* String table indices.  */
    };
  };
  uint32_t osversion_unused;	/* Required OS version (unused).  */
  uint64_t hwcap;		/* Hwcap entry.	 */
};

/* See flags member of struct cache_file_new below.  */
enum
  {
    /* No endianness information available.  An old ldconfig version
       without endianness support wrote the file.  */
    cache_file_new_flags_endian_unset = 0,

    /* Cache is invalid and should be ignored.  */
    cache_file_new_flags_endian_invalid = 1,

    /* Cache format is little endian.  */
    cache_file_new_flags_endian_little = 2,

    /* Cache format is big endian.  */
    cache_file_new_flags_endian_big = 3,

    /* Bit mask to extract the cache_file_new_flags_endian_*
       values.  */
    cache_file_new_flags_endian_mask = 3,

    /* Expected value of the endian bits in the flags member for the
       current architecture.  */
    cache_file_new_flags_endian_current
      = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	 ? cache_file_new_flags_endian_little
	 : cache_file_new_flags_endian_big),
  };

struct cache_file_new
{
  char magic[sizeof CACHEMAGIC_NEW - 1];
  char version[sizeof CACHE_VERSION - 1];
  uint32_t nlibs;		/* Number of entries.  */
  uint32_t len_strings;		/* Size of string table. */

  /* flags & cache_file_new_flags_endian_mask is one of the values
     cache_file_new_flags_endian_unset, cache_file_new_flags_endian_invalid,
     cache_file_new_flags_endian_little, cache_file_new_flags_endian_big.

     The remaining bits are unused and should be generated as zero and
     ignored by readers.  */
  uint8_t flags;

  uint8_t padding_unsed[3];	/* Not used, for future extensions.  */

  /* File offset of the extension directory.  See struct
     cache_extension below.  Must be a multiple of four.  */
  uint32_t extension_offset;

  uint32_t unused[3];		/* Leave space for future extensions
				   and align to 8 byte boundary.  */
  struct file_entry_new libs[0]; /* Entries describing libraries.  */
  /* After this the string table of size len_strings is found.	*/
};
_Static_assert (sizeof (struct cache_file_new) == 48,
		"size of struct cache_file_new");

/* Randomly chosen magic value, which allows for additional
   consistency verification.  */
enum { cache_extension_magic = (uint32_t) -358342284 };

/* Tag values for different kinds of extension sections.  Similar to
   SHT_* constants.  */
enum cache_extension_tag
  {
   /* Array of bytes containing the glibc version that generated this
      cache file.  */
   cache_extension_tag_generator,

   /* glibc-hwcaps subdirectory information.  An array of uint32_t
      values, which are indices into the string table.  The strings
      are sorted lexicographically (according to strcmp).  The extra
      level of indirection (instead of using string table indices
      directly) allows the dynamic loader to compute the preference
      order of the hwcaps names more efficiently.

      For this section, 4-byte alignment is required, and the section
      size must be a multiple of 4.  */
   cache_extension_tag_glibc_hwcaps,

   /* Total number of known cache extension tags.  */
   cache_extension_count
  };

/* Element in the array following struct cache_extension.  Similar to
   an ELF section header.  */
struct cache_extension_section
{
  /* Type of the extension section.  A enum cache_extension_tag value.  */
  uint32_t tag;

  /* Extension-specific flags.  Currently generated as zero.  */
  uint32_t flags;

  /* Offset from the start of the file for the data in this extension
     section.  Specific extensions can have alignment constraints.  */
  uint32_t offset;

  /* Length in bytes of the extension data.  Specific extensions may
     have size requirements.  */
  uint32_t size;
};

/* The extension directory in the cache.  An array of struct
   cache_extension_section entries.  */
struct cache_extension
{
  uint32_t magic;		/* Always cache_extension_magic.  */
  uint32_t count;		/* Number of following entries.  */

  /* count section descriptors of type struct cache_extension_section
     follow.  */
  struct cache_extension_section sections[];
};

/* A relocated version of struct cache_extension_section.  */
struct cache_extension_loaded
{
  /* Address and size of this extension section.  base is NULL if the
     section is missing from the file.  */
  const void *base;
  size_t size;

  /* Flags from struct cache_extension_section.  */
  uint32_t flags;
};

/* All supported extension sections, relocated.  Filled in by
   cache_extension_load below.  */
struct cache_extension_all_loaded
{
  struct cache_extension_loaded sections[cache_extension_count];
};

/* Performs basic data validation based on section tag, and removes
   the sections which are invalid.  */
static void
cache_extension_verify (struct cache_extension_all_loaded *loaded)
{
  {
    /* Section must not be empty, it must be aligned at 4 bytes, and
       the size must be a multiple of 4.  */
    struct cache_extension_loaded *hwcaps
      = &loaded->sections[cache_extension_tag_glibc_hwcaps];
    if (hwcaps->size == 0
	|| ((uintptr_t) hwcaps->base % 4) != 0
	|| (hwcaps->size % 4) != 0)
      {
	hwcaps->base = NULL;
	hwcaps->size = 0;
	hwcaps->flags = 0;
      }
  }
}

static bool __attribute__ ((unused))
cache_extension_load (const struct cache_file_new *cache,
		      const void *file_base, size_t file_size,
		      struct cache_extension_all_loaded *loaded)
{
  memset (loaded, 0, sizeof (*loaded));
  if (cache->extension_offset == 0)
    /* No extensions present.  This is not a format error.  */
    return true;
  if ((cache->extension_offset % 4) != 0)
    /* Extension offset is misaligned.  */
    return false;
  size_t size_tmp;
  if (__builtin_add_overflow (cache->extension_offset,
			      sizeof (struct cache_extension), &size_tmp)
      || size_tmp > file_size)
    /* Extension extends beyond the end of the file.  */
    return false;
  const struct cache_extension *ext = file_base + cache->extension_offset;
  if (ext->magic != cache_extension_magic)
    return false;
  if (__builtin_mul_overflow (ext->count,
			      sizeof (struct cache_extension_section),
			      &size_tmp)
      || __builtin_add_overflow (cache->extension_offset
				 + sizeof (struct cache_extension), size_tmp,
				 &size_tmp)
      || size_tmp > file_size)
    /* Extension array extends beyond the end of the file.  */
    return false;
  for (uint32_t i = 0; i < ext->count; ++i)
    {
      if (__builtin_add_overflow (ext->sections[i].offset,
				  ext->sections[i].size, &size_tmp)
	  || size_tmp > file_size)
	/* Extension data extends beyond the end of the file.  */
	return false;

      uint32_t tag = ext->sections[i].tag;
      if (tag >= cache_extension_count)
	/* Tag is out of range and unrecognized.  */
	continue;
      loaded->sections[tag].base = file_base + ext->sections[i].offset;
      loaded->sections[tag].size = ext->sections[i].size;
      loaded->sections[tag].flags = ext->sections[i].flags;
    }
  cache_extension_verify (loaded);
  return true;
}

#include <errno.h>
#include <sys/types.h>

#include "iamroot.h"

ssize_t
__ldso_cache (const char *soname, char *buf, size_t bufsiz)
{
  int fd = open (LD_SO_CACHE, O_RDONLY);
  if (fd == -1)
    {
      return -1;
    }

  ssize_t ret = -1;
  struct stat statbuf;
  int err = fstat (fd, &statbuf);
  if (err == -1)
    {
      goto close;
    }

  size_t len = statbuf.st_size;
  void *addr_new = MAP_FAILED;
  void *addr = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED)
    {
      goto close;
    }

  /* Compatibility */
  addr_new = addr;
  err = memcmp (addr_new, CACHEMAGIC, strlen (CACHEMAGIC));
  if (err == 0)
    {
      struct cache_file *cache = NULL;

      cache = (struct cache_file *)addr_new;
      addr_new = &cache->libs[cache->nlibs];
    }

  /* New format */
  err = memcmp (addr_new, CACHEMAGIC_VERSION_NEW,
                strlen (CACHEMAGIC_VERSION_NEW));
  if (err != 0)
    {
      ret = __set_errno (EINVAL, -1);
      goto munmap;
    }

  struct cache_file_new *cache_new = (struct cache_file_new *)addr_new;
  const char *string_table = (const char *)addr_new;

  for (uint32_t i = 0; i < cache_new->nlibs; i++)
    {
      const char *path = &string_table[cache_new->libs[i].value];
      if (streq (soname, __basename(path)))
        {
          const char *root = __getrootdir ();
          if (streq (root, "/"))
            root = "";

          int n = _snprintf (buf, bufsiz, "%s%s", root, path);
          if (n == -1)
            goto munmap;

          ret = n;
          goto munmap;
        }
    }

  ret = __set_errno (ENOENT, -1);

munmap:
  __munmap (addr, len);

close:
  __close (fd);

  return ret;
}
