/**
 * @addtogroup TEST_TLAPI
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "tlApiErrorStrings.h"

/**
 * NOTE: this needs to be updated whenever new error codes are defined
 *       in TlApiError.h
 */
const char* tlApiCodeToString(tlApiResult_t result) {

	switch (result) {
	case TLAPI_OK:
		return "TLAPI_OK";

	case E_TLAPI_NOT_IMPLEMENTED:
		return "E_TLAPI_NOT_IMPLEMENTED";
	case E_TLAPI_UNKNOWN:
		return "E_TLAPI_UNKNOWN";
	case E_TLAPI_UNKNOWN_FUNCTION:
		return "E_TLAPI_UNKNOWN_FUNCTION";
	case E_TLAPI_INVALID_INPUT:
		return "E_TLAPI_INVALID_INPUT";
	case E_TLAPI_INVALID_RANGE:
		return "E_TLAPI_INVALID_RANGE";
	case E_TLAPI_BUFFER_TOO_SMALL:
		return "E_TLAPI_BUFFER_TOO_SMALL";
	case E_TLAPI_INVALID_TIMEOUT:
		return "E_TLAPI_INVALID_TIMEOUT";
	case E_TLAPI_TIMEOUT:
		return "E_TLAPI_TIMEOUT";
	case E_TLAPI_NULL_POINTER:
		return "E_TLAPI_NULL_POINTER";
	case E_TLAPI_INVALID_PARAMETER:
		return "E_TLAPI_INVALID_PARAMETER";
	case E_TLAPI_UNMAPPED_BUFFER:
		return "E_TLAPI_UNMAPPED_BUFFER";
	case E_TLAPI_UNALIGNED_POINTER:
		return "E_TLAPI_UNALIGNED_POINTER";

		/* Communication error codes */
	case E_TLAPI_COM_WAIT:
		return "E_TLAPI_COM_WAIT";
	case E_TLAPI_COM_ERROR:
		return "E_TLAPI_COM_ERROR";

		/* Crypto error codes */
	case E_TLAPI_CR_HANDLE_INVALID:
		return "E_TLAPI_CR_HANDLE_INVALID";

	case E_TLAPI_CR_ALGORITHM_NOT_AVAILABLE:
		return "E_TLAPI_CR_ALGORITHM_NOT_AVAILABLE";
	case E_TLAPI_CR_ALGORITHM_NOT_SUPPORTED:
		return "E_TLAPI_CR_ALGORITHM_NOT_SUPPORTED";
	case E_TLAPI_CR_WRONG_INPUT_SIZE:
		return "E_TLAPI_CR_WRONG_INPUT_SIZE";
	case E_TLAPI_CR_WRONG_OUPUT_SIZE:
		return "E_TLAPI_CR_WRONG_OUPUT_SIZE";
	case E_TLAPI_CR_INCONSISTENT_DATA:
		return "E_TLAPI_CR_INCONSISTENT_DATA";
	case E_TLAPI_CR_OUT_OF_RESOURCES:
		return "E_TLAPI_CR_OUT_OF_RESOURCES";

		/* Secure object specific. */
	case E_TLAPI_SO_WRONG_CONTEXT:
		return "E_TLAPI_SO_WRONG_CONTEXT";
	case E_TLAPI_SO_WRONG_PADDING:
		return "E_TLAPI_SO_WRONG_PADDING";
	case E_TLAPI_SO_WRONG_CHECKSUM:
		return "E_TLAPI_SO_WRONG_CHECKSUM";
	case E_TLAPI_SO_CONTEXT_KEY_FAILED:
		return "E_TLAPI_SO_CONTEXT_KEY_FAILED";
	case E_TLAPI_SO_WRONG_LIFETIME:
		return "E_TLAPI_SO_WRONG_LIFETIME";
	case E_TLAPI_SO_WRONG_VERSION:
		return "E_TLAPI_SO_WRONG_VERSION";
	case E_TLAPI_SO_WRONG_TYPE:
		return "E_TLAPI_SO_WRONG_TYPE";

	default:
		return "!!! ERROR CODE UNKNOWN !!!";
	}
}
