/*
 * main.c
 *
 * Copyright (C) 2025.10.21 TOP WAYE topwaye@hotmail.com
 *
 * traverse all files in the current directory including subdirectories
 * 
 * copy and replace pattern words with predefined words in each file
 *
 * a wildcard (*) matches zero or more characters
 *
 * as a result, a number sign (#) represents a runtime current matched string
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define MAX_PATH_SIZE          4096 /* 4KB */
#define MAX_BUFFER_SIZE     4194304 /* 4MB */

char * src_buf = NULL;
char * dst_buf = NULL;

char pattern [ ] = "function*(*)*{";
char replace [ ] = "#\n\t\tprintf(\"topwaye: %s: #\\n\", $_SERVER['PHP_SELF']);";

int copy_and_replace ( char * src, int src_len, char * dst, int dst_size );

void do_command ( const char * filename )
{
	int fh;
	int bytes_read, bytes_copied, bytes_written;

    /* open file for input */
    if ( _sopen_s ( &fh, filename, _O_BINARY | _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE) )
    {
        printf ( "open failed on input file\n" );
        return;
    }

    /* read in input */
    bytes_read = _read ( fh, src_buf, MAX_BUFFER_SIZE );
    if ( bytes_read == -1 )
    {
        printf ( "problem reading file\n" );
        _close ( fh );
        return;
    }

    printf ( "%u bytes read, ", bytes_read );

	bytes_copied = copy_and_replace ( src_buf, bytes_read, dst_buf, MAX_BUFFER_SIZE );

	printf ( "%u bytes copied, ", bytes_copied );

	/* seek the beginning of the file */
	_lseek ( fh, 0L, SEEK_SET );
	
	/* change file size */
	if ( _chsize_s ( fh, bytes_copied ) )
	{
		printf ( "problem in changing the size\n" );
		_close(fh);
		return;
	}
	
	/* write out output */
	bytes_written = _write ( fh, dst_buf, bytes_copied );
	if ( bytes_written == -1 )
    {
        printf ( "problem writing file\n" );
        _close ( fh );
        return;
    }

	printf ( "%u bytes written\n", bytes_written );

    _close(fh);
}

void traverse ( const char * directory )
{
    char path [ MAX_PATH_SIZE ];
    struct __finddata64_t info;
    intptr_t handle;

    strcpy_s ( path, directory );
    strcat_s ( path, "\\*.php" );

    handle = _findfirst64 ( path, &info );
    if ( handle != -1 )
    {
        do
        {
            printf ( info.attrib & _A_RDONLY ? " Y  " : " N  " );
            printf ( info.attrib & _A_SYSTEM ? " Y  " : " N  " );
            printf ( info.attrib & _A_HIDDEN ? " Y  " : " N  " );
            printf ( info.attrib & _A_ARCH ? " Y  " : " N  " );
            printf ( "%9lld %-36s ", info.size, info.name );

            strcpy_s ( path, directory );
            strcat_s ( path, "\\" );
            strcat_s ( path, info.name );

            /* determine whether this is a subdirectory */
            if ( info.attrib & _A_SUBDIR )
            {
                printf ( "<DIR>\n" );

                if ( strcmp ( info.name, "." ) == 0 || strcmp ( info.name, ".." ) == 0 )
                    continue;

                traverse ( path );
            }
            else
            {
                do_command ( path );
            }

        } while ( _findnext64 ( handle, &info ) != -1 );

        _findclose ( handle );
    }
}

int main ( )
{
    /* make sure this directory is there, else change accordingly */
    char path [ MAX_PATH_SIZE ] = "c:\\test";
    char * buffer = NULL;

    buffer = ( char * ) malloc ( MAX_BUFFER_SIZE + MAX_BUFFER_SIZE );
    if ( ! buffer )
    {
        printf ( "insufficient memory available\n" );
        return 1;
    }

    src_buf = buffer;
    dst_buf = buffer + MAX_BUFFER_SIZE;

    /* list the files... */
    printf ( "listing of files in the directory %s\n\n", path );
    printf ( "RDO HID SYS ARC      SIZE FILE %31c COMMAND\n", ' ' );
    printf ( "--- --- --- ---      ---- ---- %31c -------\n", ' ' );

    traverse ( path );

    free ( buffer );

    return 0;
}

int copy_and_replace ( char * src, int src_len, char * dst, int dst_size )
{
	char * pos;
	int i, j, h, k;
	int a, b, bb, ii;
	int r;

	if ( dst_size < 1 ) /* size >= len + 1 */
		return 0;

	h = 0, i = 0; 
	while ( i < src_len )
	{
		a = 0, b = 0;
		
		pos = pattern; 
		
		k = 0; 
		while ( *( pos + k ) )
		{
			if ( a == 0 && b == 0 )
			{
				b = '*' == *( pos + k );

				if ( ! b )
				{
					if ( * ( src + i ) != *( pos + k ) )
						break;

					ii = i ++;
				}
				else
				{
					ii = i;
				}

				a = 1;
			}
			else if ( a == 1 && b == 1 )
			{
				bb = '*' == *( pos + k );

				if ( ! bb )
				{
					if ( i == src_len ) /* must be here, do NOT move this line */
					{
						a = 0;
						i = ii;
						break;
					}

					if ( *( src + i ) != *( pos + k ) )
					{
						i ++;
						continue;
					}

					b = 0;
					i ++;
				}
			}
			else if ( a == 1 && b == 0 )
			{
				b = '*' == *( pos + k );

				if ( ! b )
				{
					if ( i == src_len ) /* must be here, do NOT move this line */
					{
						a = 0;
						i = ii;
						break;
					}

					if ( *( src + i ) != *( pos + k ) )
					{
						a = 0;
						i = ii;
						break;
					}

					i ++;
				}
			}
			else /* a == 0 && b == 1 */
			{
				/* never be here */
			}

			k ++;
		} /* end while */

		if ( a )
		{
			if ( b ) /* must be here, do NOT move this line */
				i = src_len;

			pos = replace;

			k = 0;
			while ( *( pos + k ) )
			{
				if ( h + 1 == dst_size )
					return 0;

				r = '#' == *( pos + k );
				
				if ( r )
				{
					j = ii;
					while ( j < i )
					{
						if ( h + 1 == dst_size )
							return 0;

						*( dst + h ++ ) = *( src + j ++ );
					}

					k ++;
					continue;
				}

				*( dst + h ++ ) = *( pos + k ++ );
			}

			continue;
		}
		
		if ( h + 1 == dst_size )
			return 0;

		*( dst + h ++ ) = *( src + i ++ );
	}

	*( dst + h ) = 0;

	return h;
}
