#ifndef __UTF8_HANDLER_H_INCLUDED_4AD09038_006D_A018_36AB_D116C344B856
#define __UTF8_HANDLER_H_INCLUDED_4AD09038_006D_A018_36AB_D116C344B856

#include <stddef.h>
#include <inttypes.h>

typedef char utf8_t;

size_t
utf8GetBytesToNextChar( const char aChar );

/**
 * get the header char of a utf8 string if it need bytesToNextChar.
 *
 * @param [in] bytesToNextChar
 * @see utf8GetBytesToNextChar()
 */
uint8_t
utf8GetHeaderMask( uint8_t bytesToNextChar );

/**
 *
 */
uint8_t
utf8GetHeaderShift( const uint8_t bytesToNextChar );

/**
 * convert a utf8 character to utf32
 *
 * @param [in] str
 * @return utf32 code
 */
uint32_t
utf8ToUtf32( const char * str );

size_t
utf8ToUtf32String(
	uint32_t * 		utf32Str,
	const char * 	str,
	const size_t 	strSize
	);

size_t
utf8CalculateSizeFromUtf32( const uint32_t value );

size_t
utf8CalculateSizeFromUtf32String( const uint32_t *str_begin, const uint32_t * str_end );

/**
 * convert a utf32 character to utf8
 *
 * @param [out] str
 * @param [in] value
 * @return bytes written, if failed, 0 be returned.
 */
size_t
utf8FromUtf32( char * str, const uint32_t value );

size_t
utf8FromUtf32String(
	char * 			str,
	const uint32_t *utf32Str,
	const size_t 	utf32StrSize
	);


bool
utf8IsStartMarker( const char aChar );

/**
 * find next character from the given utf8 string
 *
 * @param [in] str the utf8 formatted string
 * @return the start position of next char
 */
char *
utf8FindNextChar( const char * str );

/**
 * find prior character from the given utf8 string
 *
 * @param [in] str the utf8 formatted string
 * @return the start position of prior char
 */
char *
utf8FindPriorChar( const char * str );

char *
utf8SkipCharsForward( const char *str, size_t distance );

char *
utf8SkipCharsBackward( const char *str, size_t distance );

size_t
utf8GetLength( const char * str );

void
utf8GetLengthAndSize( const char * str, size_t &length, size_t &size );

size_t
utf8GetLengthBetween( const char * strBegin, const char * strEnd );

size_t
utf8GetSizeBetween( const char * strBegin, const char * strEnd );

void
utf8GetLengthAndSizeBetween(
	const char * strBegin,
	const char * strEnd,
	size_t &length,
	size_t &size );

ptrdiff_t
utf8GetSizeFromLength( const char * str, size_t length );

#endif // __UTF8_HANDLER_H_INCLUDED_4AD09038_006D_A018_36AB_D116C344B856

