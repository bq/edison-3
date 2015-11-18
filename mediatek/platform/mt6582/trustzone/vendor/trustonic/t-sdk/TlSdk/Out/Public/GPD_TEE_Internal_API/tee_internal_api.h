/**
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#ifndef   __TEE_INTERNAL_API_H__
#define   __TEE_INTERNAL_API_H__

#if TBASE_API_LEVEL >= 3

#include "tee_type.h"
#include "tee_error.h"

#define TEE_VAR_NOT_USED(variable) do{(void)(variable);}while(0);
#define S_VAR_NOT_USED TEE_VAR_NOT_USED

#define TA_EXPORT

#if (defined(__TLAPICOMMON_H__) && (defined(TRUSTEDAPP)))
#error "Mixing the GP TEE API and the legacy TlApi is not supported."
#endif

/*---------------------------------------------------------
      Common type definition
---------------------------------------------------------*/

typedef union
{
   struct
   {
      void*    buffer;
      size_t   size;
   } memref;

   struct
   {
      uint32_t a;
      uint32_t b;
   } value;
}
TEE_Param;

#define TEE_HANDLE_NULL ((TEE_HANDLE)0)
#define TEE_PARAM_TYPES(t0,t1,t2,t3)  ((t0) | ((t1) << 4) | ((t2) << 8) | ((t3) << 12))
#define TEE_PARAM_TYPE_GET(t, i) (((t) >> (i*4)) & 0xF)

typedef enum {
               TEE_DATA_SEEK_SET,
               TEE_DATA_SEEK_CUR,
               TEE_DATA_SEEK_END
             } TEE_Whence;

typedef struct TEE_Time
{
   uint32_t seconds;
   uint32_t millis;
} TEE_Time;

typedef struct
{
    uint32_t algorithm;
    uint32_t operationClass;
    uint32_t mode;
    uint32_t digestLength;
    uint32_t maxKeySize;
    uint32_t keySize;
    uint32_t requiredKeyUsage;
    uint32_t handleState;
} TEE_OperationInfo;


typedef enum
{
    TEE_MODE_ENCRYPT,
    TEE_MODE_DECRYPT,
    TEE_MODE_SIGN,
    TEE_MODE_VERIFY,
    TEE_MODE_MAC,
    TEE_MODE_DIGEST,
    TEE_MODE_DERIVE
} TEE_OperationMode;


typedef struct
{
    uint32_t attributeID;
    union
    {
        struct
        {
            void* buffer;
            size_t length;
        }ref;
        struct
        {
            uint32_t a, b;
        }value;
    }content;
} TEE_Attribute;


typedef struct
{
    uint32_t dataSize;
    uint32_t dataPosition;
} __TEE_PersistentData;


typedef struct
{
    uint32_t objectType;
    uint32_t objectSize;
    uint32_t maxObjectSize;
    uint32_t objectUsage;
    uint32_t dataSize;
    uint32_t dataPosition;
    uint32_t handleFlags;
} TEE_ObjectInfo;


typedef void*  TEE_OperationHandle;
typedef void*  TEE_ObjectHandle;

typedef TEE_HANDLE  TEE_ObjectEnumHandle;


/*------------------------------------------------------------------------------
         Constants
------------------------------------------------------------------------------*/

#define TEE_TIMEOUT_INFINITE                 0xFFFFFFFF

/* Login types */
#define TEE_LOGIN_PUBLIC                     0x00000000
#define TEE_LOGIN_USER                       0x00000001
#define TEE_LOGIN_GROUP                      0x00000002
#define TEE_LOGIN_APPLICATION                0x00000004
#define TEE_LOGIN_APPLICATION_USER           0x00000005
#define TEE_LOGIN_APPLICATION_GROUP          0x00000006
#define TEE_LOGIN_TRUSTED_APP                0xF0000000

/* Parameter types */
#define TEE_PARAM_TYPE_NONE                  0x0
#define TEE_PARAM_TYPE_VALUE_INPUT           0x1
#define TEE_PARAM_TYPE_VALUE_OUTPUT          0x2
#define TEE_PARAM_TYPE_VALUE_INOUT           0x3
#define TEE_PARAM_TYPE_MEMREF_INPUT          0x5
#define TEE_PARAM_TYPE_MEMREF_OUTPUT         0x6
#define TEE_PARAM_TYPE_MEMREF_INOUT          0x7

#define TEE_MEMORY_ACCESS_READ               0x00000001
#define TEE_MEMORY_ACCESS_WRITE              0x00000002
#define TEE_MEMORY_ACCESS_ANY_OWNER          0x00000004

#define TEE_ORIGIN_API                       1
#define TEE_ORIGIN_COMMS                     2
#define TEE_ORIGIN_TEE                       3
#define TEE_ORIGIN_TRUSTED_APP               4


/* Allocation hints */
#define TEE_ALLOCATION_HINT_ZEROED           0x00000000

/* Crypto Usage Constants */
#define TEE_USAGE_EXTRACTABLE                0x00000001
#define TEE_USAGE_ENCRYPT                    0x00000002
#define TEE_USAGE_DECRYPT                    0x00000004
#define TEE_USAGE_MAC                        0x00000008
#define TEE_USAGE_SIGN                       0x00000010
#define TEE_USAGE_VERIFY                     0x00000020
#define TEE_USAGE_DERIVE                     0x00000040

/* Crypto Handle Flag Constants */
#define TEE_HANDLE_FLAG_PERSISTENT           0x00010000
#define TEE_HANDLE_FLAG_INITIALIZED          0x00020000
#define TEE_HANDLE_FLAG_KEY_SET              0x00040000
#define TEE_HANDLE_FLAG_EXPECT_TWO_KEYS      0x00080000

/* Crypto Operation Constants */
#define TEE_OPERATION_CIPHER                 1
#define TEE_OPERATION_MAC                    3
#define TEE_OPERATION_AE                     4
#define TEE_OPERATION_DIGEST                 5
#define TEE_OPERATION_ASYMMETRIC_CIPHER      6
#define TEE_OPERATION_ASYMMETRIC_SIGNATURE   7
#define TEE_OPERATION_KEY_DERIVATION         8

/* Crypto Algortithm Constants */
#define TEE_ALG_AES_ECB_NOPAD                0x10000010
#define TEE_ALG_AES_CBC_NOPAD                0x10000110
#define TEE_ALG_AES_CTR                      0x10000210
#define TEE_ALG_AES_CTS                      0x10000310
#define TEE_ALG_AES_XTS                      0x10000410
#define TEE_ALG_AES_CBC_MAC_NOPAD            0x30000110
#define TEE_ALG_AES_CBC_MAC_PKCS5            0x30000510
#define TEE_ALG_AES_CMAC                     0x30000610
#define TEE_ALG_AES_CCM                      0x40000710
#define TEE_ALG_AES_GCM                      0x40000810
#define TEE_ALG_DES_ECB_NOPAD                0x10000011
#define TEE_ALG_DES_CBC_NOPAD                0x10000111
#define TEE_ALG_DES_CBC_MAC_NOPAD            0x30000111
#define TEE_ALG_DES_CBC_MAC_PKCS5            0x30000511
#define TEE_ALG_DES_CMAC                     0x30000611
#define TEE_ALG_DES3_ECB_NOPAD               0x10000013
#define TEE_ALG_DES3_CBC_NOPAD               0x10000113
#define TEE_ALG_DES3_CBC_MAC_NOPAD           0x30000113
#define TEE_ALG_DES3_CBC_MAC_PKCS5           0x30000513
#define TEE_ALG_DES3_CMAC                    0x30000613
#define TEE_ALG_RSASSA_PKCS1_V1_5_MD5        0x70001830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA1       0x70002830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA224     0x70003830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA256     0x70004830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA384     0x70005830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA512     0x70006830
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA1   0x70212930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA224 0x70313930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256 0x70414930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA384 0x70515930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA512 0x70616930
#define TEE_ALG_RSAES_PKCS1_V1_5             0x60000130
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1   0x60210230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224 0x60310230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256 0x60410230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384 0x60510230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512 0x60610230
#define TEE_ALG_RSA_NOPAD                    0x60000030
#define TEE_ALG_DSA_SHA1                     0x70002131
#define TEE_ALG_DH_DERIVE_SHARED_SECRET      0x80000032
#define TEE_ALG_MD5                          0x50000001
#define TEE_ALG_SHA1                         0x50000002
#define TEE_ALG_SHA224                       0x50000003
#define TEE_ALG_SHA256                       0x50000004
#define TEE_ALG_SHA384                       0x50000005
#define TEE_ALG_SHA512                       0x50000006
#define TEE_ALG_HMAC_MD5                     0x30000001
#define TEE_ALG_HMAC_SHA1                    0x30000002
#define TEE_ALG_HMAC_SHA224                  0x30000003
#define TEE_ALG_HMAC_SHA256                  0x30000004
#define TEE_ALG_HMAC_SHA384                  0x30000005
#define TEE_ALG_HMAC_SHA512                  0x30000006

/* Storage ID Values */
#define TEE_STORAGE_PRIVATE                  0x00000001

/* Data Flags */
#define TEE_DATA_FLAG_ACCESS_READ            0x00000001
#define TEE_DATA_FLAG_ACCESS_WRITE           0x00000002
#define TEE_DATA_FLAG_ACCESS_WRITE_META      0x00000004
#define TEE_DATA_FLAG_SHARE_READ             0x00000010
#define TEE_DATA_FLAG_SHARE_WRITE            0x00000020
#define TEE_DATA_FLAG_CREATE                 0x00000200
#define TEE_DATA_FLAG_EXCLUSIVE              0x00000400

/* Misc */
#define TEE_DATA_MAX_POSITION                0xFFFFFFFF
#define TEE_OBJECT_ID_MAX_LEN                0x40

/* Object Types */
#define TEE_TYPE_AES                         0xA0000010
#define TEE_TYPE_DES                         0xA0000011
#define TEE_TYPE_DES3                        0xA0000013
#define TEE_TYPE_HMAC_MD5                    0xA0000001
#define TEE_TYPE_HMAC_SHA1                   0xA0000002
#define TEE_TYPE_HMAC_SHA224                 0xA0000003
#define TEE_TYPE_HMAC_SHA256                 0xA0000004
#define TEE_TYPE_HMAC_SHA384                 0xA0000005
#define TEE_TYPE_HMAC_SHA512                 0xA0000006
#define TEE_TYPE_RSA_PUBLIC_KEY              0xA0000030
#define TEE_TYPE_RSA_KEYPAIR                 0xA1000030
#define TEE_TYPE_DSA_PUBLIC_KEY              0xA0000031
#define TEE_TYPE_DSA_KEYPAIR                 0xA1000031
#define TEE_TYPE_DH_KEYPAIR                  0xA1000032
#define TEE_TYPE_GENERIC_SECRET              0xA0000000

/* Object Attribute Identifier Flags - bits 28 and 29 indicate the type (value/ref) and its visibility */
#define TEE_ATTR_FLAG_VALUE                  0x20000000
#define TEE_ATTR_FLAG_PUBLIC                 0x10000000

/* Operation and Object Attribute ID Values */
#define TEE_ATTR_SECRET_VALUE                0xC0000000
#define TEE_ATTR_RSA_MODULUS                 0xD0000130
#define TEE_ATTR_RSA_PUBLIC_EXPONENT         0xD0000230
#define TEE_ATTR_RSA_PRIVATE_EXPONENT        0xC0000330
#define TEE_ATTR_RSA_PRIME1                  0xC0000430 // p
#define TEE_ATTR_RSA_PRIME2                  0xC0000530 // q
#define TEE_ATTR_RSA_EXPONENT1               0xC0000630 // dp
#define TEE_ATTR_RSA_EXPONENT2               0xC0000730 // dq
#define TEE_ATTR_RSA_COEFFICIENT             0xC0000830 // iq
#define TEE_ATTR_DSA_PRIME                   0xD0001031 // p
#define TEE_ATTR_DSA_SUBPRIME                0xD0001131 // q
#define TEE_ATTR_DSA_BASE                    0xD0001231 // g
#define TEE_ATTR_DSA_PUBLIC_VALUE            0xD0000131 // y
#define TEE_ATTR_DSA_PRIVATE_VALUE           0xC0000231 // x
#define TEE_ATTR_DH_PRIME                    0xD0001032 // p
#define TEE_ATTR_DH_SUBPRIME                 0xD0001132 // q
#define TEE_ATTR_DH_BASE                     0xD0001232 // g
#define TEE_ATTR_DH_X_BITS                   0xF0001332 // type "Value" not "Ref"
#define TEE_ATTR_DH_PUBLIC_VALUE             0xD0000132 // y
#define TEE_ATTR_DH_PRIVATE_VALUE            0xC0000232 // x
#define TEE_ATTR_RSA_OAEP_LABEL              0xD0000930
#define TEE_ATTR_RSA_PSS_SALT_LENGTH         0xF0000A30 // type "Value" not "Ref"

/*------------------------------------------------------------------------------
      Trusted Application Interface
 ------------------------------------------------------------------------------*/

TEE_Result TA_EXPORT TA_CreateEntryPoint(void);

void TA_EXPORT TA_DestroyEntryPoint(void);

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(
        uint32_t    nParamTypes,
        TEE_Param   pParams[4],
        void**      ppSessionContext);

void TA_EXPORT TA_CloseSessionEntryPoint(
        void*       pSessionContext);

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(
        void*       pSessionContext,
        uint32_t    nCommandID,
        uint32_t    nParamTypes,
        TEE_Param   pParams[4]);


/*------------------------------------------------------------------------------
      Trusted Core Framework APIs
 ------------------------------------------------------------------------------*/

_TLAPI_EXTERN_C void TEE_Panic(
        TEE_Result nPanicCode);


/*------------------------------------------------------------------------------
      Property Access Functions
 ------------------------------------------------------------------------------*/

_TLAPI_EXTERN_C TEE_Result TEE_GetPropertyAsString(
        TEE_PropSetHandle propsetOrEnumerator,
        char* name,
        char* valueBuffer,
        size_t* valueBufferLen);

_TLAPI_EXTERN_C TEE_Result TEE_GetPropertyAsBool(
        TEE_PropSetHandle propsetOrEnumerator,
        char* name,
        bool* value);

_TLAPI_EXTERN_C TEE_Result TEE_GetPropertyAsU32 (
        TEE_PropSetHandle propsetOrEnumerator,
        char* name,
        uint32_t* value);

_TLAPI_EXTERN_C TEE_Result TEE_GetPropertyAsBinaryBlock(
        TEE_PropSetHandle propsetOrEnumerator,
        char* name,
        void* valueBuffer,
        size_t* valueBufferLen);

_TLAPI_EXTERN_C TEE_Result TEE_GetPropertyAsUUID(
        TEE_PropSetHandle propsetOrEnumerator,
        char* name,
        TEE_UUID* value);

/*------------------------------------------------------------------------------
      Cancellation Functions
 ------------------------------------------------------------------------------*/

_TLAPI_EXTERN_C bool TEE_GetCancellationFlag( void );

_TLAPI_EXTERN_C bool TEE_UnmaskCancellation( void );

_TLAPI_EXTERN_C bool TEE_MaskCancellation( void );


/*------------------------------------------------------------------------------
      Memory Management Functions
 ------------------------------------------------------------------------------*/

_TLAPI_EXTERN_C TEE_Result TEE_CheckMemoryAccessRights(
        uint32_t accessFlags,
        void* buffer, size_t size);

_TLAPI_EXTERN_C void TEE_SetInstanceData(
        void* instanceData);

_TLAPI_EXTERN_C void* TEE_GetInstanceData( void );

_TLAPI_EXTERN_C void* TEE_Malloc(uint32_t size, uint32_t hint);

_TLAPI_EXTERN_C void* TEE_Realloc(
        void* buffer,
        uint32_t newSize);

_TLAPI_EXTERN_C void TEE_Free(
        void *buffer);

_TLAPI_EXTERN_C void TEE_MemMove(
        void* dest,
        void* src,
        uint32_t size);

_TLAPI_EXTERN_C int32_t TEE_MemCompare(
        void* buffer1,
        void* buffer2,
        uint32_t size);

_TLAPI_EXTERN_C void TEE_MemFill(
        void* buffer,
        uint32_t x,
        uint32_t size );

 /*------------------------------------------------------------------------------
      Cryptographic Operations API
 ------------------------------------------------------------------------------*/

 /*------------------------------------------------------------------------------
      Generic Operations Functions
 ------------------------------------------------------------------------------*/
_TLAPI_EXTERN_C TEE_Result TEE_AllocateOperation(
        TEE_OperationHandle* operation,
        uint32_t             algorithm,
        uint32_t             mode,
        uint32_t             maxKeySize);

_TLAPI_EXTERN_C void TEE_FreeOperation(
        TEE_OperationHandle operation);

_TLAPI_EXTERN_C void TEE_GetOperationInfo(
        TEE_OperationHandle operation,
        TEE_OperationInfo*  operationInfo);


_TLAPI_EXTERN_C void TEE_ResetOperation(
        TEE_OperationHandle operation);

_TLAPI_EXTERN_C TEE_Result TEE_SetOperationKey(
        TEE_OperationHandle operation,
        TEE_ObjectHandle    key);

_TLAPI_EXTERN_C TEE_Result TEE_SetOperationKey2(
        TEE_OperationHandle operation,
        TEE_ObjectHandle    key1,
        TEE_ObjectHandle    key2);

_TLAPI_EXTERN_C void TEE_CopyOperation(
        TEE_OperationHandle dstOperation,
        TEE_OperationHandle srcOperation);


/* Message Digest Functions                                                  */
_TLAPI_EXTERN_C void TEE_DigestUpdate(
        TEE_OperationHandle operation,
        void*               chunk,
        size_t              chunkSize);

_TLAPI_EXTERN_C TEE_Result TEE_DigestDoFinal(
        TEE_OperationHandle operation,
        void*               chunk,
        size_t              chunkLen,
        void*               hash,
        size_t*             hashLen);

/* Symmetric Cipher Functions                                                 */
_TLAPI_EXTERN_C void TEE_CipherInit(
        TEE_OperationHandle operation,
        void*               IV,
        size_t              IVLen);

_TLAPI_EXTERN_C TEE_Result TEE_CipherUpdate(
        TEE_OperationHandle operation,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen);

_TLAPI_EXTERN_C TEE_Result TEE_CipherDoFinal(
        TEE_OperationHandle operation,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen);

/*----------------------------------------------------------------------------
  MAC Functions
-----------------------------------------------------------------------------*/
_TLAPI_EXTERN_C void TEE_MACInit(
        TEE_OperationHandle operation,
        void*               IV,
        size_t              IVLen);

_TLAPI_EXTERN_C void TEE_MACUpdate(
        TEE_OperationHandle operation,
        void*               chunk,
        size_t              chunkSize);

_TLAPI_EXTERN_C TEE_Result TEE_MACComputeFinal(
        TEE_OperationHandle operation,
        void*               message,
        size_t              messageLen,
        void*               mac,
        size_t*             macLen);

_TLAPI_EXTERN_C TEE_Result TEE_MACCompareFinal(
        TEE_OperationHandle operation,
        void*               message,
        size_t              messageLen,
        void*               mac,
        size_t              macLen);

/*----------------------------------------------------------------------------
   Authenticated Encryption Functions ----------------------------------------
  ---------------------------------------------------------------------------*/
_TLAPI_EXTERN_C TEE_Result TEE_AEInit(
        TEE_OperationHandle operation,
        void*               nonce,
        size_t              nonceLen,
        uint32_t            tagLen,
        uint32_t            AADLen,
        uint32_t            payloadLen);

_TLAPI_EXTERN_C void TEE_AEUpdateAAD(
        TEE_OperationHandle operation,
        void*               AADdata,
        size_t              AADdataLen);

_TLAPI_EXTERN_C TEE_Result TEE_AEUpdate(
        TEE_OperationHandle operation,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen);

_TLAPI_EXTERN_C TEE_Result TEE_AEEncryptFinal(
        TEE_OperationHandle operation,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen,
        void*               tag,
        size_t*             tagLen);

_TLAPI_EXTERN_C TEE_Result TEE_AEDecryptFinal(
        TEE_OperationHandle operation,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen,
        void*               tag,
        size_t              tagLen);

/*-----------------------------------------------------------------------------
 Asymmetric Functions
-----------------------------------------------------------------------------*/
_TLAPI_EXTERN_C TEE_Result TEE_AsymmetricEncrypt(
        TEE_OperationHandle operation,
        TEE_Attribute*      params,
        uint32_t            paramCount,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen);

_TLAPI_EXTERN_C TEE_Result TEE_AsymmetricDecrypt(
        TEE_OperationHandle operation,
        TEE_Attribute*      params,
        uint32_t            paramCount,
        void*               srcData,
        size_t              srcLen,
        void*               destData,
        size_t*             destLen);

_TLAPI_EXTERN_C TEE_Result TEE_AsymmetricSignDigest(
        TEE_OperationHandle operation,
        TEE_Attribute*      params,
        uint32_t            paramCount,
        void*               digest,
        size_t              digestLen,
        void*               signature,
        size_t*             signatureLen);

_TLAPI_EXTERN_C TEE_Result TEE_AsymmetricVerifyDigest(
        TEE_OperationHandle operation,
        TEE_Attribute*      params,
        uint32_t            paramCount,
        void*               digest,
        size_t              digestLen,
        void*               signature,
        size_t              signatureLen);

/*-----------------------------------------------------------------------------
 Random Data Generation Function
-----------------------------------------------------------------------------*/
_TLAPI_EXTERN_C void TEE_GenerateRandom(
        void*  randomBuffer,
        size_t randomBufferLen);


/*------------------------------------------------------------------------------
      Trusted Storage Functions
------------------------------------------------------------------------------*/

/* Generic Object Functions */
_TLAPI_EXTERN_C void TEE_GetObjectInfo(
        TEE_ObjectHandle object,
        TEE_ObjectInfo*  objectInfo);

_TLAPI_EXTERN_C void TEE_RestrictObjectUsage(
        TEE_ObjectHandle object,
        uint32_t         objectUsage);

_TLAPI_EXTERN_C TEE_Result TEE_GetObjectBufferAttribute(
        TEE_ObjectHandle object,
        uint32_t         attributeID,
        void*            buffer,
        size_t*          size);

_TLAPI_EXTERN_C TEE_Result TEE_GetObjectValueAttribute(
        TEE_ObjectHandle object,
        uint32_t         attributeID,
        uint32_t*        a,
        uint32_t*        b);

_TLAPI_EXTERN_C void TEE_CloseObject(
        TEE_ObjectHandle object);

/* Transient Object Functions */
_TLAPI_EXTERN_C TEE_Result TEE_AllocateTransientObject(
        uint32_t          objectType,
        uint32_t          maxObjectSize,
        TEE_ObjectHandle* object);

_TLAPI_EXTERN_C void TEE_FreeTransientObject(
        TEE_ObjectHandle object);

_TLAPI_EXTERN_C void TEE_ResetTransientObject(
        TEE_ObjectHandle object);

_TLAPI_EXTERN_C TEE_Result TEE_PopulateTransientObject(
        TEE_ObjectHandle object,
        TEE_Attribute*   attrs,
        uint32_t         attrCount);

_TLAPI_EXTERN_C void TEE_InitRefAttribute(
        TEE_Attribute* attr,
        uint32_t       attributeID,
        void*          buffer,
        size_t         length);

_TLAPI_EXTERN_C void TEE_InitValueAttribute(
        TEE_Attribute* attr,
        uint32_t       attributeID,
        uint32_t       a,
        uint32_t       b);

_TLAPI_EXTERN_C void TEE_CopyObjectAttributes(
        TEE_ObjectHandle destObject,
        TEE_ObjectHandle srcObject);

_TLAPI_EXTERN_C TEE_Result TEE_GenerateKey(
        TEE_ObjectHandle object,
        uint32_t         keySize,
        TEE_Attribute*   params,
        uint32_t         paramCount);

/* Persistant Object Functions */
_TLAPI_EXTERN_C TEE_Result TEE_CreatePersistentObject(
        uint32_t          storageID,
        void*             objectID,
        size_t            objectIDLen,
        uint32_t          flags,
        TEE_ObjectHandle  attributes,
        void*             initialData,
        size_t            initialDataLen,
        TEE_ObjectHandle* object);

_TLAPI_EXTERN_C TEE_Result TEE_OpenPersistentObject(
        uint32_t          storageID,
        void*             objectID,
        size_t            objectIDLen,
        uint32_t          flags,
        TEE_ObjectHandle* object);

_TLAPI_EXTERN_C void TEE_CloseAndDeletePersistentObject(TEE_ObjectHandle object);

_TLAPI_EXTERN_C TEE_Result TEE_RenamePersistentObject(
        TEE_ObjectHandle object,
        void*            newObjectID,
        size_t           newObjectIDLen);

_TLAPI_EXTERN_C TEE_Result TEE_AllocatePersistentObjectEnumerator(
        TEE_ObjectEnumHandle* objectEnumerator);

_TLAPI_EXTERN_C void TEE_FreePersistentObjectEnumerator(
        TEE_ObjectEnumHandle objectEnumerator);

_TLAPI_EXTERN_C void TEE_ResetPersistentObjectEnumerator(
        TEE_ObjectEnumHandle objectEnumerator);

_TLAPI_EXTERN_C TEE_Result TEE_StartPersistentObjectEnumerator(
        TEE_ObjectEnumHandle objectEnumerator,
        uint32_t             storageID);

_TLAPI_EXTERN_C TEE_Result TEE_GetNextPersistentObject(
        TEE_ObjectEnumHandle objectEnumerator,
        TEE_ObjectInfo*      objectInfo,
        void*                objectID,
        size_t*              objectIDLen);

_TLAPI_EXTERN_C TEE_Result TEE_ReadObjectData(
        TEE_ObjectHandle object,
        void*            buffer,
        size_t           size,
        uint32_t*        count);

_TLAPI_EXTERN_C TEE_Result TEE_WriteObjectData(
        TEE_ObjectHandle object,
        void*            buffer,
        size_t           size);

_TLAPI_EXTERN_C TEE_Result TEE_TruncateObjectData(
        TEE_ObjectHandle object,
        uint32_t         size);

_TLAPI_EXTERN_C TEE_Result TEE_SeekObjectData(
        TEE_ObjectHandle object,
        int32_t          offset,
        TEE_Whence       whence);

/*------------------------------------------------------------------------------
      Debugging Functions
------------------------------------------------------------------------------*/
/** Formatted logging functions.
 *
 * TEE_LogvPrintf, TEE_LogPrintf
 *
 * Minimal printf-like function to print logging message to NWd log.
 *
 * Supported formatters:
 *   %s String, NULL value emit "<NULL>".
 *   %x %X hex
 *   %p pointer (hex with fixed width of 8)
 *   %d %i signed decimal
 *   %u unsigned decimal
 *   %t timestamp (if available in platform). NOTE: This does not consume any value in parameter list.
 *   %% outputs single %
 *
 *   %s, %x, %d, and %u support width (example %5s). Width is interpreted as minimum number of characters.
 *   Hex number is left padded using '0' to desired width.
 *   Decimal number is left padded using ' ' to desired width.
 *   String is right padded to desired length.
 *
 *   Newline is used to terminate logging line.
 *
 * @param fmt     Formatter
 */
_TLAPI_EXTERN_C void TEE_LogvPrintf(
        const char *fmt,
        va_list args);

_TLAPI_EXTERN_C void TEE_LogPrintf(
        const char *fmt,
        ...);


#if defined(__INCLUDE_DEBUG)

    #define TEE_DbgPrintf(...)   TEE_LogPrintf(__VA_ARGS__)
    #define TEE_DbgvPrintf(...)  TEE_LogvPrintf(__VA_ARGS__)

#else

   #define TEE_DbgPrintf(...)    DUMMY_FUNCTION()
   #define TEE_DbgvPrintf(...)   DUMMY_FUNCTION()

#endif // __INCLUDE_DEBUG

#define TEE_DbgPrintLnf(...)      do{TEE_DbgPrintf(__VA_ARGS__);TEE_DbgPrintf(EOL);}while(FALSE)

#endif /* TBASE_API_LEVEL */

#endif /* __TEE_INTERNAL_API_H__ */

