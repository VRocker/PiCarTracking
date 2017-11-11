/**	\file
*	\brief Defines some string utilities for simple efficient string usage
*	\author Craig Richards
*	\date 11/11/17
*/

#pragma once

// For size_t
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

	/**	Compares 2 strings to check if they are the same.
	*	Differs from strcmp as it doesn't count where the strings are different
	*	@param str1 The string to compare with
	*	@param str2 The string to compare to
	*	@retval 1 Strings are equal
	*	@retval 0 Strings are NOT equal
	*/
	unsigned int str_equal(const char* str1, const char* str2);

	/**	str_cpy is a _safe_ function for copying strings from 1 location to the other.
	*	@param dest The destination buffer
	*	@param source The source string to copy from
	*	@param size The size of the destination buffer
	*/
	void str_cpy(char* dest, const char* source, size_t size);

	/**	A safe way to append a string to another string. Ensures it is always null terminated.
	*	@param dest The destination buffer
	*	@param source The source string to copy from
	*	@param size The size of the destination buffer
	*/
	void str_cat(char* dest, const char* source, size_t size);

	/**	Counts the number of 'tokens' in a string as defined by the delimeter
	*	@param text The text to count the tokens of
	*	@param delim The delimeter to count in the string
	*	@return The number of tokens in the string
	*/
	size_t numtok(const char* text, char delim);

	/**	Tokenises a string
	*	@param[in] s The input string to tokenise
	*	@param[in] delim The delimeter to tokenise the string by
	*	@param[out] last The buffer holding the last tokenised string
	*	@return The token retrieved from the string
	*/
	char* safe_strtok(char* s, const char delim, char** last);

	/**	Returns if the value is a number
	*	@param str The input string to check
	*	@return If the string is a number or not
	*	@retval 1 String is a number
	*	@retval 0 String is not a number
	*/
	unsigned int isnum(const char* str);

#ifdef __cplusplus
}
#endif
