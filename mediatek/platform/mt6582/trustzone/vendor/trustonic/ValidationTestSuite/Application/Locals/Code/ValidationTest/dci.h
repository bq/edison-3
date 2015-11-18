/**
 * dci.h
 * Used for direct communication between Nwd and Driver (like is used tci
 * for communicating directly between Nwd and Trustlets)
 */

#ifndef DCI_H_
#define DCI_H_

typedef uint32_t dciCommandId_t;
typedef uint32_t dciResponseId_t;
typedef uint32_t dciReturnCode_t;

/* DCI command header */
typedef struct {
    dciCommandId_t commandId; /**< Command ID */
} dciCommandHeader_t;

/* DCI response header */
typedef struct {
    dciResponseId_t responseId; /**< Response ID (must be command ID | RSP_ID_MASK )*/
    dciReturnCode_t returnCode; /**< Return code of command */
} dciResponseHeader_t;

#endif // DCI_H_

