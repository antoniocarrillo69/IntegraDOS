/* s4init.c   (c)Copyright Sequiter Software Inc., 1991-1992.  All rights reserved. */

#include "d4all.h"
#include "e4error.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

int S4FUNCTION s4get_init( S4SORT *s4 )
{
   int entries_per_spool, spools_per_pool, entries_used ;
   void *ptr ;
   char *pool_entry, *pool_entry_iterate ;
   long spool_disk_i ;

   if ( s4->code_base->error_code < 0 )  return -1 ;
   if ( s4->spools_max > 0 )
   {
      s4flush(s4) ;
      if ( h4seq_write_flush(&s4->seqwrite) < 0 )  return -1 ;
      #ifdef S4DEBUG
         if (s4->seqwrite_buffer == 0 )  e4severe( e4info, "s4get_init", (char *) 0 ) ;
      #endif
      u4free( s4->seqwrite_buffer ) ;
      s4->seqwrite_buffer =  0 ;
   
      u4free( s4->pointers ) ;
      s4->pointers =  0 ;
   
      for (;;)
      {
         s4->spool_pointer =  (S4SPOOL *) u4alloc( sizeof(S4SPOOL) * (unsigned) s4->spools_max ) ;
         if ( s4->spool_pointer != 0 )  break ;

         if ( l4last(&s4->pool) == 0 )
         {
            s4free(s4) ;
            return e4error( s4->code_base, e4memory, E4_MEMORY_S, (char *) 0 ) ;
         }

         y4free( s4->pool_memory, l4pop( &s4->pool ) ) ;
         s4->pool_n-- ;
      }

      for(;;)
      {
         /* Split up the pools between the spools */
         if ( s4->pool_n == 0 )
            spools_per_pool =  s4->spools_max ;
         else
            spools_per_pool =  (s4->spools_max+s4->pool_n-1) / s4->pool_n ;
            entries_per_spool =  s4->pool_entries / spools_per_pool ;
         if ( entries_per_spool == 0 )
         {
            s4free(s4) ;
            return e4error( s4->code_base, e4memory, E4_MEMORY_S, (char *) 0 ) ;
         }
         if ( s4->pool_n != 0 )  break ;

         ptr =  y4alloc( s4->pool_memory ) ;
         if ( ptr != 0 ) 
         {
            l4add( &s4->pool, ptr ) ;
            s4->pool_n++ ;
         }
         else
         {
            s4->pool_entries /= 2 ;
            while ( ptr = l4pop(&s4->pool) )  y4free( s4->pool_memory, ptr ) ;
            y4release( s4->pool_memory ) ;
            s4->pool_memory =  y4memory_type( 1, (unsigned) s4->pool_entries*s4->tot_len+sizeof(L4LINK), 1, 1 ) ;
         }
      }

      s4->spool_mem_len  =   entries_per_spool*s4->tot_len ;
      s4->spool_disk_len =   (long) s4->pointers_init * s4->tot_len ;

      entries_used =  s4->pool_entries+1 ;  /* Entries used in current pool. */
      pool_entry_iterate =  0 ;

      for ( spool_disk_i = 0L; s4->spools_n < s4->spools_max; )
      {
         memmove( s4->spool_pointer+1, s4->spool_pointer, sizeof(S4SPOOL)*s4->spools_n ) ;
         if ( entries_used + entries_per_spool > s4->pool_entries )
         {
            entries_used =  0 ;
            pool_entry_iterate= (char *) l4next( &s4->pool, pool_entry_iterate);
            pool_entry =  pool_entry_iterate + sizeof(L4LINK) ;
         }
         s4->spool_pointer->ptr =  pool_entry ;
         pool_entry   +=  (s4->tot_len*entries_per_spool) ;
         entries_used +=  entries_per_spool ;

         s4->spool_pointer->spool_i =  s4->spools_n++ ;
         s4->spool_pointer->disk    =  spool_disk_i ;
         spool_disk_i +=  s4->spool_disk_len ;

         s4->spool_pointer->len = 0 ;
            if ( s4->spools_n < s4->spools_max )
               s4next_spool_entry(s4) ;
      }
   }
   else
   {
      s4quick( (void **) s4->pointers, s4->pointers_used, s4->cmp,
          s4->sort_len);
      #ifdef S4DEBUG
         if (s4->seqwrite_buffer == 0 )
            e4severe( e4info, "s4get_init()", (char *) 0 ) ;
      #endif
      u4free( s4->seqwrite_buffer ) ;
      s4->seqwrite_buffer =  0 ;
   }
   return 0 ;
}

int S4FUNCTION s4init( S4SORT *s4, C4CODE *c4, int sort_l, int info_l )
{
   memset( (void *)s4, 0, sizeof(S4SORT) ) ;
   s4->file.hand =  -1 ;

   s4->code_base =  c4 ;
   s4->cmp =  u4memcmp ;

/* file.name.set_max( 12 ) ;  Allocate name memory now. */

   s4->spool_pointer = 0 ;
   s4->pointers = 0 ;

   s4->spools_max =  s4->spools_n = s4->pointers_i = s4->pointers_used = s4->pointers_init =
         s4->pool_n = s4->spool_mem_len = 0 ;
   s4->spool_disk_len =  0L ;

   s4->sort_len =  sort_l ;
   s4->info_len =  info_l ;
   s4->info_offset =  s4->sort_len + sizeof(long) ;
   s4->tot_len  =  s4->info_offset + s4->info_len ;
   #ifdef S4PORTABLE
      if ( s4->tot_len % sizeof(double) != 0 )
         s4->tot_len +=  sizeof(double) -  s4->tot_len % sizeof(double) ;
   #endif
   s4->pool_entries =  (s4->code_base->mem_size_sort_pool-sizeof(L4LINK))/ s4->tot_len ;

   s4->pointers_max =  s4->code_base->mem_size_sort_pool/sizeof(char*) ;

   s4->is_mem_avail =  1 ;

   s4->seqwrite_buffer =  (char *) u4alloc( s4->code_base->mem_size_sort_buffer) ;
   if ( s4->seqwrite_buffer == 0 )
      return e4error( s4->code_base, e4memory, (char *) 0 ) ;

   h4seq_write_init( &s4->seqwrite, &s4->file, 0L, s4->seqwrite_buffer, 
           s4->code_base->mem_size_sort_buffer ) ;

   for(;;)
   {
      s4->pointers =  (char **) u4alloc( s4->pointers_max * sizeof(char *) ) ;
      if ( s4->pointers != 0 )  break ;

      s4->pointers_max /= 2 ;
      if ( s4->pointers_max < 256 )
      {
         s4free(s4) ;
         return e4error( s4->code_base, e4memory, E4_MEMORY_S, (char *) 0 ) ;
      }
   }

   #ifdef S4DEBUG
      if ( s4->pool_memory != 0 )
         e4severe( e4info, "s4init()", E4_INFO_ALR, (char *) 0 ) ;
   #endif
   s4->pool_memory = y4memory_type( 1, s4->code_base->mem_size_sort_pool,1,1);
   if ( s4->pool_memory == 0 )
   {
      s4free(s4) ;
      return e4error( s4->code_base, e4memory, E4_MEMORY_S, (char *) 0 ) ;
   }

   return 0 ;
}

void s4init_pointers( S4SORT *s4, char *avail_mem, unsigned len )
{
   /* Assign 'pointers' */
   int n, i ;

   n =  len / s4->tot_len ;
   i =  s4->pointers_init ;

   s4->pointers_init +=  n ;
   if ( s4->pointers_init > s4->pointers_max )
      s4->pointers_init =  s4->pointers_max ;

   for ( ; i < s4->pointers_init; i++, avail_mem += s4->tot_len )
      s4->pointers[i] =  avail_mem ;
}
