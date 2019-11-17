#ifndef DLN_BOOTLOADER_H_INCLUDED
#define DLN_BOOTLOADER_H_INCLUDED

#include "../common/dln.h"


#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ICCARM__)
        #pragma pack(1)
        #define __PACKED_ATTR   /*nothing*/
        //#define __attribute__(...)
#elif defined(WIN32) || defined(__BORLANDC__)  || defined(WIN64)
        #pragma pack(push,1)
        #define __PACKED_ATTR   /*nothing*/
        //#define __attribute__(...)
#else
        #define __PACKED_ATTR __attribute__ ((packed))
#endif

//#define DLN_MODULE_BOOT  DLN_MODULE(0xFF)	

#define DLN_MSG_ID_BOOT_GET_MODE DLN_BUILD_MSG_ID(0x00, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_ENTER_BOOTLOADER DLN_BUILD_MSG_ID(0x01, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_EXIT_BOOTLOADER DLN_BUILD_MSG_ID(0x02, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_GET_MEM_COUNT DLN_BUILD_MSG_ID(0x03, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_GET_MEM_DESC DLN_BUILD_MSG_ID(0x04, DLN_MODULE_BOOT)


#define DLN_MSG_ID_BOOT_READ DLN_BUILD_MSG_ID(0x05, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_WRITE DLN_BUILD_MSG_ID(0x06, DLN_MODULE_BOOT)
#define DLN_MSG_ID_BOOT_ERASE   DLN_BUILD_MSG_ID(0x07, DLN_MODULE_BOOT)


#define DLN_BOOT_MODE   uint32_t
#define DLN_BOOT_MODE_BOOTLOADER            ((DLN_BOOT_MODE)1)
#define DLN_BOOT_MODE_APPLICATION           ((DLN_BOOT_MODE)2)
#define DLN_BOOT_MODE_UPDATE_IN_PROGRESS    ((DLN_BOOT_MODE)3)

#define DLN_BOOT_MEM_TYPE   uint32_t
#define DLN_BOOT_MEM_FLASH                  ((DLN_BOOT_MEM_TYPE)1)

/*!
\struct DLN_BOOT_FLASH_DESC
This structure is used to store configuration of the device internal flash memory.
*/
typedef struct
{
    DLN_BOOT_MEM_TYPE memoryType;
    uint32_t startAddr;					///< Starting address of the device firmware.
    uint32_t size;					///< Maximum size of the device firmware.
    uint32_t pageSize;						///< Page size.
    uint32_t minBufferSize;
} __PACKED_ATTR DLN_BOOT_MEM_DESC;

/*!
\struct DLN_BOOT_GET_MODE_CMD
The command is used to obtain current mode of your DLN device.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
} __PACKED_ATTR DLN_BOOT_GET_MODE_CMD;

/*!
\struct DLN_BOOT_GET_MODE_RSP
The response contains the current mode of your DLN device.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
    DLN_BOOT_MODE mode;						///< Current mode of the DLN device.
} __PACKED_ATTR DLN_BOOT_GET_MODE_RSP;

/*!
\struct DLN_BOOT_ENTER_BOOTLOADER_CMD
The command is used to enter Bootloader mode.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
} __PACKED_ATTR DLN_BOOT_ENTER_BOOTLOADER_CMD;

/*!
\struct DLN_BOOT_ENTER_BOOTLOADER_RSP
The response informs if the bootloader mode has been successfully switched on.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
} __PACKED_ATTR DLN_BOOT_ENTER_BOOTLOADER_RSP;

/*!
\struct DLN_BOOT_EXIT_BOOTLOADER_CMD
The command is used to exit the Bootloader mode and continue in Application mode.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
} __PACKED_ATTR DLN_BOOT_EXIT_BOOTLOADER_CMD;

/*!
\struct DLN_BOOT_EXIT_BOOTLOADER_RSP
The response informs if the bootloader mode has been successfully switched off.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
} __PACKED_ATTR DLN_BOOT_EXIT_BOOTLOADER_RSP;


typedef struct
{
    DLN_MSG_HEADER header;
} __PACKED_ATTR DLN_BOOT_GET_MEM_COUNT_CMD;

typedef struct
{
    DLN_MSG_HEADER header;
    DLN_RESULT result;
    uint32_t count;
} __PACKED_ATTR DLN_BOOT_GET_MEM_COUNT_RSP;

/*!
\struct DLN_BOOT_GET_FLASH_DESC_CMD
The command is used to retrieve information about the device internal memory.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    uint32_t memory;
} __PACKED_ATTR DLN_BOOT_GET_MEM_DESC_CMD;

/*!
\struct DLN_BOOT_GET_FLASH_DESC_RSP
The response contains information about the device internal memory.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
    DLN_BOOT_MEM_DESC descriptor;				///< Contains the DLN_BOOT_FLASH_DESC structure.
} __PACKED_ATTR DLN_BOOT_GET_MEM_DESC_RSP;

/*!
\struct DLN_BOOT_WRITE_FLASH_CMD
The command is used to write new firmware into the device internal memory.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    uint32_t memory;
    uint32_t address;						///< Address to start writing from.
    uint16_t size;							///< Size of the firmware.
    uint8_t buffer[256];					///< Buffer.
} __PACKED_ATTR DLN_BOOT_WRITE_CMD;

/*!
\struct DLN_BOOT_WRITE_FLASH_RSP
The response informs if the firmware has been successfully written.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
} __PACKED_ATTR DLN_BOOT_WRITE_RSP;

/*!
\struct DLN_BOOT_READ_FLASH_CMD
The command is used to read the firmware, currently installed in the DLN device.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    uint32_t memory;
    uint32_t address;						///< Address to start reading from.
    uint16_t size;							///< Size of the data to be read.
} __PACKED_ATTR DLN_BOOT_READ_CMD;

/*!
\struct DLN_BOOT_READ_FLASH_RSP
The response contains data about current device firmmware.
*/
typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
    uint8_t buffer[256];					///< Buffer.
} __PACKED_ATTR DLN_BOOT_READ_RSP;


typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
} __PACKED_ATTR DLN_BOOT_ERASE_CMD;

typedef struct
{
    DLN_MSG_HEADER header;					///< Defines the DLN message header.
    DLN_RESULT result;						///< Contains the result of the command execution.
} __PACKED_ATTR DLN_BOOT_ERASE_RSP;



/*!
Retrieves current mode of your DLN device.
\param handle - a handle to the DLN device.
\param mode - a pointer to the DLN_BOOT_MODE variable, which will be used to store current device mode after the function execution.
\retval DLN_RES_SUCCESS - the mode of DLN device has been successfully retrieved.
*/
DLN_RESULT DlnBootGetMode(HDLN handle, DLN_BOOT_MODE *mode);

/*!
Retrieves the number of flash panes of your DLN-series adapter.
\param handle - a handle to the DLN device.
\param count - a pointer to 8-bit unsigned integer, which will be filled with the number of flash panes after the function execution.
\retval DLN_SUCCESS - the number of flash panes was successfully retrieved.
*/
DLN_RESULT DlnBootGetMemCount(HDLN handle, uint32_t *count);

/*!
Enters the Bootloader mode.
\param handle - a handle to the DLN device.
\retval DLN_SUCCESS - the bootloader mode has been successfully entered;
\retval DLN_RES_COMMAND_NOT_SUPPORTED - already in bootloader mode.
*/
DLN_RESULT DlnBootEnterBootloader(HDLN handle);

/*!
Exits the Bootloader mode.
\param handle - a handle to the DLN device.
\retval DLN_SUCCESS - the bootloader mode has been successfully exited;
\retval DLN_RES_COMMAND_NOT_SUPPORTED - already in application mode.
*/
DLN_RESULT DlnBootExitBootloader(HDLN handle);

/*!
Retrieves information about the device internal flash memory.
\param handle - a handle to the DLN device.
\retval DLN_RES_SUCCESS - the flash memory information has been successfully retrieved;
\retval DLN_RES_INVALID_PLANE_NUMBER - the memory value is out of range;
*/
DLN_RESULT DlnBootGetMemDesc(HDLN handle, uint32_t memory, DLN_BOOT_MEM_DESC *descriptor);

/*!
Writes new firmware into the device internal memory.
\param handle - a handle to the DLN device.
\param address - address to start writing from.
\param size - size of the firmware.
\param buffer - Buffer.
\retval DLN_RES_SUCCESS - the flash memory has been writted successfully;
\retval DLN_RES_INVALID_ADDRESS - the address is out of range;
\retval DLN_RES_OVERFLOW - the address+size is out of range.
*/
DLN_RESULT DlnBootWrite(HDLN handle, uint32_t memory, uint32_t address, uint16_t size, uint8_t *buffer);

/*!
Reads data on the DLN device current firmware.
\param handle - a handle to the DLN device.
\param address - address to start reading from.
\param size - size of the data to be read.
\param buffer - A pointer to an unsigned 8 bit integer. This integer will be filles with the firmware data after the function execution.
\retval DLN_RES_SUCCESS - the flash memory has been read successfully;
\retval DLN_RES_INVALID_ADDRESS - the address is out of range;
\retval DLN_RES_OVERFLOW - the address+size is out of range.
*/
DLN_RESULT DlnBootRead(HDLN handle, uint32_t memory, uint32_t address, uint16_t size, uint8_t *buffer);

/*!
\retval DLN_RES_SUCCESS - the flash memory has been erased successfully;
\retval DLN_RES_COMMAND_NOT_SUPPORTED - the flash erase feature is not supported.
*/
DLN_RESULT DlnBootErase(HDLN handle);


#ifdef __ICCARM__
        #pragma pack()
#elif defined(WIN32) || defined(__BORLANDC__)  || defined(WIN64)
        #pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif




#endif //DLN_BOOTLOADER_H_INCLUDED
