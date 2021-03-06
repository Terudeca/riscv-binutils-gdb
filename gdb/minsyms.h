/* Minimal symbol table definitions for GDB.

   Copyright (C) 2011-2017 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef MINSYMS_H
#define MINSYMS_H

/* Several lookup functions return both a minimal symbol and the
   objfile in which it is found.  This structure is used in these
   cases.  */

struct bound_minimal_symbol
{
  /* The minimal symbol that was found, or NULL if no minimal symbol
     was found.  */

  struct minimal_symbol *minsym;

  /* If MINSYM is not NULL, then this is the objfile in which the
     symbol is defined.  */

  struct objfile *objfile;
};

/* This header declares most of the API for dealing with minimal
   symbols and minimal symbol tables.  A few things are declared
   elsewhere; see below.

   A minimal symbol is a symbol for which there is no direct debug
   information.  For example, for an ELF binary, minimal symbols are
   created from the ELF symbol table.

   For the definition of the minimal symbol structure, see struct
   minimal_symbol in symtab.h.

   Minimal symbols are stored in tables attached to an objfile; see
   objfiles.h for details.  Code should generally treat these tables
   as opaque and use functions provided by minsyms.c to inspect them.
*/

struct msym_bunch;

/* An RAII-based object that is used to record minimal symbols while
   they are being read.  */
class minimal_symbol_reader
{
 public:

  /* Prepare to start collecting minimal symbols.  This should be
     called by a symbol reader to initialize the minimal symbol
     module.  */

  explicit minimal_symbol_reader (struct objfile *);

  ~minimal_symbol_reader ();

  /* Install the minimal symbols that have been collected into the
     given objfile.  */

  void install ();

  /* Record a new minimal symbol.  This is the "full" entry point;
     simpler convenience entry points are also provided below.
   
     This returns a new minimal symbol.  It is ok to modify the returned
     minimal symbol (though generally not necessary).  It is not ok,
     though, to stash the pointer anywhere; as minimal symbols may be
     moved after creation.  The memory for the returned minimal symbol
     is still owned by the minsyms.c code, and should not be freed.
   
     Arguments are:

     NAME - the symbol's name
     NAME_LEN - the length of the name
     COPY_NAME - if true, the minsym code must make a copy of NAME.  If
     false, then NAME must be NUL-terminated, and must have a lifetime
     that is at least as long as OBJFILE's lifetime.
     ADDRESS - the address of the symbol
     MS_TYPE - the type of the symbol
     SECTION - the symbol's section
  */

  struct minimal_symbol *record_full (const char *name,
				      int name_len,
				      bool copy_name,
				      CORE_ADDR address,
				      enum minimal_symbol_type ms_type,
				      int section);

  /* Like record_full, but:
     - uses strlen to compute NAME_LEN,
     - passes COPY_NAME = true,
     - and passes a default SECTION, depending on the type

     This variant does not return the new symbol.  */

  void record (const char *name, CORE_ADDR address,
	       enum minimal_symbol_type ms_type);

  /* Like record_full, but:
     - uses strlen to compute NAME_LEN,
     - passes COPY_NAME = true.  */

  struct minimal_symbol *record_with_info (const char *name,
					   CORE_ADDR address,
					   enum minimal_symbol_type ms_type,
					   int section)
  {
    return record_full (name, strlen (name), true, address, ms_type, section);
  }

 private:

  /* No need for these.  They are intentionally not defined anywhere.  */
  minimal_symbol_reader &operator=
    (const minimal_symbol_reader &);
  minimal_symbol_reader (const minimal_symbol_reader &);

  struct objfile *m_objfile;

  /* Bunch currently being filled up.
     The next field points to chain of filled bunches.  */

  struct msym_bunch *m_msym_bunch;

  /* Number of slots filled in current bunch.  */

  int m_msym_bunch_index;

  /* Total number of minimal symbols recorded so far for the
     objfile.  */

  int m_msym_count;
};

/* Create the terminating entry of OBJFILE's minimal symbol table.
   If OBJFILE->msymbols is zero, allocate a single entry from
   OBJFILE->objfile_obstack; otherwise, just initialize
   OBJFILE->msymbols[OBJFILE->minimal_symbol_count].  */

void terminate_minimal_symbol_table (struct objfile *objfile);



/* Compute a hash code for the string argument.  */

unsigned int msymbol_hash (const char *);

/* Like msymbol_hash, but compute a hash code that is compatible with
   strcmp_iw.  */

unsigned int msymbol_hash_iw (const char *);

/* Compute the next hash value from previous HASH and the character C.  This
   is only a GDB in-memory computed value with no external files compatibility
   requirements.  */

#define SYMBOL_HASH_NEXT(hash, c)			\
  ((hash) * 67 + tolower ((unsigned char) (c)) - 113)



/* Look through all the current minimal symbol tables and find the
   first minimal symbol that matches NAME.  If OBJF is non-NULL, limit
   the search to that objfile.  If SFILE is non-NULL, the only
   file-scope symbols considered will be from that source file (global
   symbols are still preferred).  Returns a bound minimal symbol that
   matches, or an empty bound minimal symbol if no match is found.  */

struct bound_minimal_symbol lookup_minimal_symbol (const char *,
						   const char *,
						   struct objfile *);

/* Like lookup_minimal_symbol, but searches all files and
   objfiles.  */

struct bound_minimal_symbol lookup_bound_minimal_symbol (const char *);

/* Find the minimal symbol named NAME, and return both the minsym
   struct and its objfile.  This only checks the linkage name.  */

struct bound_minimal_symbol lookup_minimal_symbol_and_objfile (const char *);

/* Look through all the current minimal symbol tables and find the
   first minimal symbol that matches NAME and has text type.  If OBJF
   is non-NULL, limit the search to that objfile.  Returns a bound
   minimal symbol that matches, or an "empty" bound minimal symbol
   otherwise.

   This function only searches the mangled (linkage) names.  */

struct bound_minimal_symbol lookup_minimal_symbol_text (const char *,
							struct objfile *);

/* Look through all the current minimal symbol tables and find the
   first minimal symbol that matches NAME and is a solib trampoline.
   If OBJF is non-NULL, limit the search to that objfile.  Returns a
   pointer to the minimal symbol that matches, or NULL if no match is
   found.

   This function only searches the mangled (linkage) names.  */

struct bound_minimal_symbol lookup_minimal_symbol_solib_trampoline
    (const char *,
     struct objfile *);

/* Look through all the current minimal symbol tables and find the
   first minimal symbol that matches NAME and PC.  If OBJF is non-NULL,
   limit the search to that objfile.  Returns a pointer to the minimal
   symbol that matches, or NULL if no match is found.  */

struct minimal_symbol *lookup_minimal_symbol_by_pc_name
    (CORE_ADDR, const char *, struct objfile *);

/* Search through the minimal symbol table for each objfile and find
   the symbol whose address is the largest address that is still less
   than or equal to PC, and which matches SECTION.

   If SECTION is NULL, this uses the result of find_pc_section
   instead.

   The result has a non-NULL 'minsym' member if such a symbol is
   found, or NULL if PC is not in a suitable range.  */

struct bound_minimal_symbol lookup_minimal_symbol_by_pc_section
    (CORE_ADDR,
     struct obj_section *);

/* Backward compatibility: search through the minimal symbol table 
   for a matching PC (no section given).
   
   This is a wrapper that calls lookup_minimal_symbol_by_pc_section
   with a NULL section argument.  */

struct bound_minimal_symbol lookup_minimal_symbol_by_pc (CORE_ADDR);

/* Iterate over all the minimal symbols in the objfile OBJF which
   match NAME.  Both the ordinary and demangled names of each symbol
   are considered.  The caller is responsible for canonicalizing NAME,
   should that need to be done.

   For each matching symbol, CALLBACK is called with the symbol and
   USER_DATA as arguments.  */

void iterate_over_minimal_symbols (struct objfile *objf,
				   const char *name,
				   void (*callback) (struct minimal_symbol *,
						     void *),
				   void *user_data);

/* Compute the upper bound of MINSYM.  The upper bound is the last
   address thought to be part of the symbol.  If the symbol has a
   size, it is used.  Otherwise use the lesser of the next minimal
   symbol in the same section, or the end of the section, as the end
   of the function.  */

CORE_ADDR minimal_symbol_upper_bound (struct bound_minimal_symbol minsym);

#endif /* MINSYMS_H */
