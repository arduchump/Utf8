
#include "Utf8.h"
#include <Arduino.h>

#define ENSURE(condition) do { if (!(condition)) {Serial.println("ENSURE failed at: "#condition); while(1); } } while(0)
#define SIZE_OF_ARRAY(array) (sizeof((array))/sizeof((array)[0]))
#define PTR_OFFSET_BYTES(ptr, nBytes) ((void*)(((char *)(ptr)) + (nBytes)))
#define PTR_OFFSET_BETWEEN(ptrBegin, ptrEnd) ((char*)(ptrEnd) - (char*)(ptrBegin))

size_t
utf8GetBytesToNextChar( const char aChar )
{
  /**
   * utf-8 skip data extract from glibc.
   * the last 0xFE, 0xFF use as special using ( file BOM header )
   */
  static const uint8_t s_skip_data[256] PROGMEM =
  {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1,
  };
  return (uint8_t)pgm_read_byte(s_skip_data + (uint8_t)(aChar));
}

uint8_t
utf8GetHeaderMask( uint8_t bytesToNextChar )
{
  /** the 0 element is unused */
  static const uint8_t s_header_mask[7] PROGMEM =
  {
    0x00, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01,
  };

  ENSURE( bytesToNextChar < SIZE_OF_ARRAY(s_header_mask) );

  return (uint8_t)pgm_read_byte(s_header_mask + (uint8_t)(bytesToNextChar));
}

uint8_t
utf8GetHeaderShift( const uint8_t bytesToNextChar )
{
  return ( bytesToNextChar - 1 ) * 6;
}

uint32_t
utf8ToUtf32( const char * str )
{
	uint8_t		bytesToNextChar;
	size_t		shift = 0;
	uint32_t 	result = 0;

	bytesToNextChar = utf8GetBytesToNextChar( *str );
	shift = utf8GetHeaderShift( bytesToNextChar );
	result |= ( (((uint32_t)(uint8_t)*str) & (uint32_t)utf8GetHeaderMask( bytesToNextChar )) << shift );

	while( shift > 0 )
	{
		++ str;
		shift -= 6;

		result |= ( ( ((uint32_t)(uint8_t)*str) & 0x3F ) << shift );
	}

	return result;
}

uint32_t
utf8ToUtf32( const Utf8FChar * str )
{
	uint8_t		bytesToNextChar;
	size_t		shift = 0;
	uint32_t 	result = 0;

	bytesToNextChar = utf8GetBytesToNextChar( pgm_read_byte_far(str) );
	shift = utf8GetHeaderShift( bytesToNextChar );
	result |= ( (((uint32_t)(uint8_t)pgm_read_byte_far(str)) & (uint32_t)utf8GetHeaderMask( bytesToNextChar )) << shift );

	while( shift > 0 )
	{
		str = (const Utf8FChar *)((char *)(str) + 1);
		shift -= 6;
		result |= ( ( ((uint32_t)(uint8_t)pgm_read_byte_far(str)) & 0x3F ) << shift );
	}

	return result;
}

size_t
utf8ToUtf32String(
	uint32_t * 		utf32Str,
	const char * 	str,
	const size_t 	strSize
	)
{
	uint32_t * utf32StrBegin = utf32Str;

	if( strSize )
	{
		const char * utf8_end = (const char * )PTR_OFFSET_BYTES( str, strSize );

		while( str < utf8_end  )
		{
			*( utf32Str ++ ) = utf8ToUtf32( str );

			str = utf8FindNextChar( str );
		}
	}
	else
	{
		/* wait for '\0' */
		while( *str )
		{
			*( utf32Str ++ ) = utf8ToUtf32( str );

			str = utf8FindNextChar( str );
		}
	}

	*utf32Str = 0; /* last character must be 0 */

	return PTR_OFFSET_BETWEEN( utf32StrBegin, utf32Str );
}

size_t
utf8CalculateSizeFromUtf32( const uint32_t value )
{
  if( value <= 0x7F )
  {
    return 1;
  }
  else if( value <= 0x07FF )
  {
    return 2;
  }
  else if( value <= 0xFFFF )
  {
    return 3;
  }
  else if( value <= 0x1FFFFF )
  {
    return 4;
  }
  else if( value <= 0x3FFFFFF )
  {
    return 5;
  }
  else if( value <= 0x7FFFFFFF )
  {
    return 6;
  }
  else
  { /* invalid values */
    return 0;
  }
}

size_t
utf8CalculateSizeFromUtf32String( const uint32_t *strBegin, const uint32_t * strEnd )
{
  const uint32_t * iterator = strBegin;
  size_t result = 0;

  while( iterator < strEnd )
  {
    result += utf8CalculateSizeFromUtf32( *iterator );
  }

  return result;
}

size_t
utf8FromUtf32( char * str, const uint32_t value )
{
	static const char headerMarks[8] PROGMEM =
		{
			0,
			0,
			(char)192,
			(char)224,
			(char)240,
			(char)248,
			(char)252,
			0, /* unused */
		};
	size_t		shift = 0;
	uint8_t		bytesToNextChar;
	char *		begin = str;

	bytesToNextChar = utf8CalculateSizeFromUtf32( value ) ;

	shift = utf8GetHeaderShift( bytesToNextChar );
	*str = ( ( value >> shift ) & utf8GetHeaderMask( bytesToNextChar ) )
		| (uint8_t)pgm_read_byte(headerMarks + (uint8_t)(bytesToNextChar));

	while( shift > 0 )
	{
		++ str;
		shift -= 6;

		*str = 0x80 | ( ( value >> shift ) & 0x3F );
	}

	return bytesToNextChar;
}

size_t
utf8FromUtf32String(
	char * 			str,
	const uint32_t *utf32Str,
	const size_t 	utf32StrSize
	)
{
	size_t result = 0;

	if( utf32StrSize )
	{
		const uint32_t *utf32StrEnd = (const uint32_t *)PTR_OFFSET_BYTES( utf32Str, utf32StrSize );

		while( utf32Str < utf32StrEnd )
		{
			result += utf8FromUtf32( str, *utf32Str );

			++ utf32Str;
		}
	}
	else
	{
		while( *utf32Str )
		{
			result += utf8FromUtf32( str, *utf32Str );

			++ utf32Str;
		}
	}

	*str = 0; /* last character must be 0 */

	return result;
}


bool
utf8IsStartMarker( const char aChar )
{
  return 0x80 != ( aChar & 0xC0 );
}

char *
utf8FindNextChar( const char * str )
{
  /* validate the current str */
  if( ! utf8IsStartMarker( *str ) )
  {
    while( !utf8IsStartMarker( * ( ++ str ) ) ) {};

    return const_cast<char*>( str );
  }

  return (char *)PTR_OFFSET_BYTES( str, utf8GetBytesToNextChar( *str )  );
};

Utf8FChar *
utf8FindNextChar( const Utf8FChar * str )
{
  /* validate the current str */
  if( ! utf8IsStartMarker( pgm_read_byte_far(str) ) )
  {
    while(1)
    {
      str = (const Utf8FChar *)((char *)str + 1);
      if(utf8IsStartMarker(pgm_read_byte_far(str)))
      {
        break;
      }
    }

    return const_cast<Utf8FChar*>( str );
  }

  return (Utf8FChar *)PTR_OFFSET_BYTES( str, utf8GetBytesToNextChar( (char)pgm_read_byte_far(str) )  );
};

char *
utf8FindPriorChar( const char * str )
{
  while( !utf8IsStartMarker( * ( -- str ) ) ) {};

  return const_cast<char *>(str);
};

char *
utf8SkipCharsForward( const char *str, size_t distance )
{
  while( ( distance > 0 ) && ( *str != 0 ) )
  {
    str = utf8FindNextChar( str );
    -- distance;
  };

  return const_cast<char *>( str );
}

char *
utf8SkipCharsBackward( const char *str, size_t distance )
{
  while( distance > 0 )
  {
    str = utf8FindPriorChar( str );
    -- distance;
  };

  return const_cast<char *>( str );
}

size_t
utf8GetLength( const char * str )
{
  size_t length = 0;

  while( * str )
  {
    ++ length;
    str = utf8FindNextChar( str );
  }

  return length;
};

void
utf8GetLengthAndSize( const char * str, size_t &length, size_t &size )
{
  size_t skipBytes;

  length = 0;
  size = 0;

  while( * str )
  {
    skipBytes = utf8GetBytesToNextChar( *str );

    ++ length;
    size += skipBytes;

    str = (const char *)PTR_OFFSET_BYTES( str, skipBytes );
  }
}

size_t
utf8GetLengthBetween( const char * strBegin, const char * strEnd )
{
  size_t result = 0;

  while( strBegin < strEnd )
  {
    strBegin = utf8FindNextChar( strBegin );
    ++ result;
  }

  return result;
}

size_t
utf8GetSizeBetween( const char * strBegin, const char * strEnd )
{
  const char * iterator = strBegin;

  while( iterator < strEnd )
  {
    iterator = utf8FindNextChar( iterator );
  }

  return PTR_OFFSET_BETWEEN( strBegin, strEnd ) ;
}

void
utf8GetLengthAndSizeBetween(
  const char * strBegin,
  const char * strEnd,
  size_t &length,
  size_t &size )
{
  const char *  iterator = strBegin;

  length = 0;
  while( iterator < strEnd )
  {
    iterator = utf8FindNextChar( iterator );
    ++ length;
  }

  size = PTR_OFFSET_BETWEEN( strBegin, strEnd );
}

ptrdiff_t
utf8GetSizeFromLength( const char * str, size_t length )
{
  const char * strBegin = str;

  while( length -- > 0 )
  {
    str = utf8FindNextChar( str );
  };

  return PTR_OFFSET_BETWEEN( strBegin, str );
}
