/**
 * @file znet.h
 * @date 16 Mar 2018
 * @brief TBD.
 */

#ifndef ZNET_H
#define ZNET_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#include "znet_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Node ID
 *
 * Valid values 1..232
 * Specific values:
 * 0    - any
 * 0xFF - invalid
 */
typedef uint8_t znet_node_id_t;

#define ZNET_NODE_ID_ANY 0
#define ZNET_NODE_ID_MIN 1
#define ZNET_NODE_ID_MAX 232
#define ZNET_NODE_ID_INVALID 0xFF

/**
 * @brief Channel ID
 */
typedef uint8_t znet_node_channel_id_t;
#define ZNET_CHANNEL_ID_ROOT 0
#define ZNET_CHANNEL_ID_MIN 1
#define ZNET_CHANNEL_ID_MAX 127

/**
 * @brief Basic class
 */
typedef uint8_t znet_basic_class_t;

/**
 * @brief Generic class
 */
typedef uint8_t znet_generic_class_t;

/**
 * @brief Specific class
 */
typedef uint8_t znet_specific_class_t;

/**
 * @brief Command class
 */
typedef uint8_t znet_command_class_t;

/**
 * @brief Command class version
 */
typedef uint8_t znet_command_class_version_t;

#define ZNET_COMMAND_CLASS_VERSION_NONE 0
#define ZNET_COMMAND_CLASS_VERSION_MIN 1
#define ZNET_COMMAND_CLASS_VERSION_MAX 255

/**
 * @brief TBD.
 */
typedef struct znet_nodeinfo_t
{
    uint8_t __ver;                   /**< reserved */
    znet_node_id_t node_id;          /**< Node ID */
    uint8_t __0;                     /**< reserved for capa */
    uint8_t __1;                     /**< reserved for sec */
    uint8_t __2;                     /**< reserved */
    znet_basic_class_t basic;        /**< Basic class type */
    znet_generic_class_t generic;    /**< Generic class type */
    znet_specific_class_t specific;  /**< Specific class type */
    uint8_t commands_count;          /**< Commands classes count */
    znet_command_class_t commands[]; /**< Commands classes, max 35 */
} znet_nodeinfo_t;

/**
 * @brief Function prototype for allocate/reallocate/freed block memmory
 *
 * The allocator function must provide a functionality similar to realloc, but
 * not exactly the same.
 * - ptr is NULL if allocate new block, the allocator behaves like malloc
 * - when size is zero, the allocator must return NULL
 * - if ptr is not NULL and size is zero, the allocator behaves like free
 * - when size is not zero and ptr is not NULL, the allocator behaves like
 * realloc
 * - when size is not zero, the allocator returns NULL if and only if it cannot
 * fill the request
 * - znet assumes that the allocator never fails when old size for realloc <=
 * size
 *
 * Example of implementation:
 * @code
 * static void* znet_alloc( void* ptr, size_t size, void* arg)
 * {
 *     if( size != 0 )
 *         return realloc( ptr, size );
 *
 *     free( ptr );
 *     return NULL;
 * }
 * @endcode
 *
 * @param ptr A pointer to the block being allocated/reallocated/freed
 * @param size The new size of the block
 * @param arg Parameter for callback functions
 * @return pointer to allocated block
 */
typedef void* ( *ZNET_ALLOC )( void* ptr, size_t size, void* arg );

/**
 * @brief Function prototype for clock that cannot be set and represents
 * monotonic time since some unspecified starting point.
 *
 * @param arg Parameter for callback functions
 * @return Time from monotonic timer in ms. On error, 0 is returned
 */
typedef uint64_t ( *ZNET_CLOCK )( void* arg );

/**
 * @brief Function prototype for log massages
 *
 * Support 4 levels:
 * 1 - error
 * 2 - warning
 * 3 - info
 * 4 - debug
 *
 * @param lvl Message level
 * @param fmt Format string that follows the same specifications as format in
 * printf
 * @param list A value identifying a variable arguments list initialized with
 * va_start
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_LOG )( int lvl, const char* fmt, va_list list, void* arg );

/**
 * @brief Write data to uart (to zwave module connected to uart)
 *
 * Use cases:
 * 1. Get free space in uart tx buffer - data == NULL, size == 0,
 * ret_size = <ptr>
 * 2. Write all data (limit by size) to uart tx buffer - data = <ptr>,
 * size != 0, ret_size = <ptr> For write call is blocked if all data can
 * not be written to uart buffer
 *
 * @param data Pointer to buffer with data
 * @param size The size of the data for write
 * @param ret_size Return free size in uart buffer or param size (see use cases)
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_UART_WRITE )( const void* data, size_t size,
                                  size_t* ret_size, void* arg );

/**
 * @brief Read data from uart (from zwave module connected to uart)
 *
 * Use cases:
 * 1. Get free space in uart rx buffer - data == NULL, size == 0,
 * ret_size = <ptr>
 * 2. Read all data (limit by size) from uart rx buffer - data = <ptr>,
 * size !=  0, ret_size = <ptr> For read call is blocked if all data can
 * not be readed from uart buffer
 *
 * @param data Pointer to buffer for data
 * @param size The size of the data for read
 * @param ret_size Return size of available data in uart buffer or param size
 * (see use cases)
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_UART_READ )( void* data, size_t size, size_t* ret_size,
                                 void* arg );

/**
 * @brief Reset zwave module and uart connected to it
 *
 * Implement if it is possible to hard reset the module
 *
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_UART_RESET )( void* arg );

/**
 * @brief Function prototype for writing data to storage
 *
 * @param offset Offset in storage for write
 * @param data Pointer to buffer with data
 * @param size The size of the data for write
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_STORE_SAVE )( size_t offset, const void* data, size_t size,
                                  void* arg );

/**
 * @brief Function prototype for reading data from storage
 *
 * If read outside of the current stored data, then zero out the data and return
 * success.
 *
 * @param offset Offset in storage for read
 * @param data Pointer to buffer for data
 * @param size The size of the data for read
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_STORE_LOAD )( size_t offset, void* data, size_t size,
                                  void* arg );

/**
 * @brief Function prototype for reset storage
 *
 * If reserv is not 0 to reserve space for data
 *
 * @param reserve The size of the reserved space
 * @param arg Parameter for callback functions
 * @return Return zero on success. On error, -1 is returned
 */
typedef int ( *ZNET_STORE_RESET )( size_t reserve, void* arg );

/**
 * @brief Function prototype for notify: set default complated
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_SET_DEFAULT_RESULT )( int err, void* arg );

/**
 * @brief Function prototype for notify: add node complated
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_info Return node info
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_ADD_RESULT )( int err,
                                        const znet_nodeinfo_t* node_info,
                                        void* arg );

/**
 * @brief Function prototype for notify: remove node complated
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_REM_RESULT )( int err,
                                        const znet_nodeinfo_t* node_info,
                                        void* arg );

/**
 * @brief Function prototype for notify: list node found node
 *
 * It is called many times. One time on the found node. node_info is NULL if
 * there are no more nodes.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_info Return node info
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_LIST_RESULT )( int err,
                                         const znet_nodeinfo_t* node_info,
                                         void* arg );

/**
 * @brief TBD.
 */
typedef struct znet_version_report_t
{
    uint8_t __ver;          /**< reserved */
    uint8_t lib_type;       /**< Z-Wave Library Type */
    uint8_t proto_ver;      /**< Z-Wave Protocol Version */
    uint8_t proto_sub_ver;  /**< Z-Wave Protocol Sub Version */
    uint8_t firm_0_ver;     /**< Firmware 0 (Application) Version */
    uint8_t firm_0_sub_ver; /**< Firmware 0 (Application) Sub Version */
    uint8_t hard_ver;       /**< Hardware version */
    uint8_t firms_count;    /**< Firmware count without firm_0 */
    struct
    {
        uint8_t ver;
        uint8_t sub_ver;
    } firms[]; /**< Firmware 1..N Version & Sub Version */
} znet_version_report_t;

/**
 * @brief Function prototype for notify: get version
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_VERSION_RESULT )(
    int err, znet_node_id_t node_id, const znet_version_report_t* value,
    void* arg );

/**
 * @brief TBD.
 */
typedef struct znet_command_version_report_t
{
    uint8_t __ver;                        /**< reserved */
    znet_command_class_t command;         /**< Command class */
    znet_command_class_version_t version; /**< Command class version */
} znet_command_version_report_t;

/**
 * @brief Function prototype for notify: get command class version
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_COMMAND_VERSION_RESULT )(
    int err, znet_node_id_t node_id, const znet_command_version_report_t* value,
    void* arg );

/**
 * @brief Manufacturer specific
 */
typedef struct znet_manufacturer_specific_report_t
{
    uint8_t __ver;            /**< reserved */
    uint8_t __0;              /**< reserved */
    uint16_t manufacturer_id; /**< Manufacturer ID */
    uint16_t product_type;    /**< Product Type ID */
    uint16_t product_id;      /**< Product ID */
} znet_manufacturer_specific_report_t;

/**
 * @brief Function prototype for notify: get manufacturer specific
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MANUFACTURER_SPECIFIC_RESULT )(
    int err, znet_node_id_t node_id,
    const znet_manufacturer_specific_report_t* value, void* arg );

/**
 * @brief Device specific: Device ID Type
 */
typedef uint8_t znet_cmd_device_specific_type_t;

#define ZNET_CMD_DEVICE_SPECIFIC_TYPE_OEM 0
#define ZNET_CMD_DEVICE_SPECIFIC_TYPE_SERIAL 1
#define ZNET_CMD_DEVICE_SPECIFIC_TYPE_PSEUDO_RAND 2

/**
 * @brief Device specific: Data format
 */
typedef uint8_t znet_cmd_device_specific_format_t;

#define ZNET_CMD_DEVICE_SPECIFIC_FORMAT_UTF8 0x00
#define ZNET_CMD_DEVICE_SPECIFIC_FORMAT_BIN 0x01

/**
 * @brief Device specific
 */
typedef struct znet_device_specific_report_t
{
    uint8_t __ver;                                 /**< reserved */
    znet_cmd_device_specific_type_t type;          /**< Device ID Type */
    znet_cmd_device_specific_format_t data_format; /**< Device ID Data Format */
    uint8_t data_count;                            /**< Device ID Data Length */
    uint8_t data[];                                /**< Device ID Data */
} znet_device_specific_report_t;

/**
 * @brief Function prototype for notify: get device specific
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_DEVICE_SPECIFIC_RESULT )(
    int err, znet_node_id_t node_id, const znet_device_specific_report_t* value,
    void* arg );

/**
 * @brief Z-Wave Plus Info: Role Type
 */
typedef uint8_t znet_zwaveplus_info_role_type_t;

/**
 * @brief Z-Wave Plus Info: Node Type
 */
typedef uint8_t znet_zwaveplus_info_node_type_t;

#define ZNET_ZWAVEPLUS_INFO_NODE_TYPE_NODE 0x00
#define ZNET_ZWAVEPLUS_INFO_NODE_TYPE_FOR_IP_GATEWAY 0x02

/**
 * @brief Z-Wave Plus Info: Installer Icon Type
 */
typedef uint16_t znet_zwaveplus_info_installer_icon_t;

/**
 * @brief Z-Wave Plus Info: User Icon Type
 */
typedef uint16_t znet_zwaveplus_info_user_icon_t;

/**
 * @brief Z-Wave Plus Info
 */
typedef struct znet_zwaveplus_info_report_t
{
    uint8_t __ver;          /**< reserved */
    uint8_t zwave_plus_ver; /**< Z-Wave Plus Version */
    znet_zwaveplus_info_role_type_t role_type;
    znet_zwaveplus_info_node_type_t node_type;
    znet_zwaveplus_info_installer_icon_t installer_icon;
    znet_zwaveplus_info_user_icon_t user_icon;
} znet_zwaveplus_info_report_t;

/**
 * @brief Function prototype for notify: get z-wave plus info
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_ZWAVEPLUS_INFO_RESULT )(
    int err, znet_node_id_t node_id, const znet_zwaveplus_info_report_t* value,
    void* arg );

/**
 * @brief TBD.
 */
typedef uint8_t znet_cmd_basic_value_t;

#define ZNET_CMD_BASIC_VALUE_MIN 0
#define ZNET_CMD_BASIC_VALUE_MAX 0x63
#define ZNET_CMD_BASIC_VALUE_UNKNOWN 0xFE
#define ZNET_CMD_BASIC_VALUE_MAX_ABSOLUTE 0xFF

/**
 * @brief Function prototype for notify: node cmd basic report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_BASIC_RESULT )( int err, znet_node_id_t node_id,
                                              znet_node_channel_id_t channel_id,
                                              znet_cmd_basic_value_t value,
                                              void* arg );

/**
 * @brief TBD.
 */
typedef uint8_t znet_cmd_binary_switch_value_t;

#define ZNET_CMD_BINARY_SWITCH_VALUE_MIN 0
#define ZNET_CMD_BINARY_SWITCH_VALUE_MAX 0x63
#define ZNET_CMD_BINARY_SWITCH_VALUE_UNKNOWN 0xFE
#define ZNET_CMD_BINARY_SWITCH_VALUE_MAX_ABSOLUTE 0xFF

/**
 * @brief Function prototype for notify: node cmd bianry switch report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_BINARY_SWITCH_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    znet_cmd_binary_switch_value_t value, void* arg );

/**
 * @brief TBD.
 */
typedef uint8_t znet_cmd_multilevel_switch_value_t;

#define ZNET_CMD_MULTILEVEL_SWITCH_VALUE_MIN 0
#define ZNET_CMD_MULTILEVEL_SWITCH_VALUE_MAX 0x63
#define ZNET_CMD_MULTILEVEL_SWITCH_VALUE_UNKNOWN 0xFE
#define ZNET_CMD_MULTILEVEL_SWITCH_VALUE_MAX_ABSOLUTE 0xFF

/**
 * @brief Function prototype for notify: node cmd multilevel switch
 * report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MULTILEVEL_SWITCH_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    znet_cmd_multilevel_switch_value_t value, void* arg );

/**
 * @brief Meter type specifies what type of metering device.
 */
#define ZNET_METER_TYPE_ELECTRIC 0x01
#define ZNET_METER_TYPE_GAS 0x02
#define ZNET_METER_TYPE_WATER 0x03

/**
 * @brief Scale advertise the unit used.
 */
#define ZNET_METER_SCALE_ELECTRIC_kWh 0
#define ZNET_METER_SCALE_ELECTRIC_kVAh 1
#define ZNET_METER_SCALE_ELECTRIC_W 2
#define ZNET_METER_SCALE_ELECTRIC_PULSE_COUNT 3
#define ZNET_METER_SCALE_ELECTRIC_V 4
#define ZNET_METER_SCALE_ELECTRIC_A 5
#define ZNET_METER_SCALE_ELECTRIC_POWER_FACTOR 6
#define ZNET_METER_SCALE_ELECTRIC_EX 7
#define ZNET_METER_SCALE_ELECTRIC_kVar 7
#define ZNET_METER_SCALE_ELECTRIC_kVarh 8
#define ZNET_METER_SCALE_GAS_CUBIC_METERS 0
#define ZNET_METER_SCALE_GAS_CUBIC_FEET 1
#define ZNET_METER_SCALE_GAS_PULSE_COUNT 3
#define ZNET_METER_SCALE_WATER_CUBIC_METERS 0
#define ZNET_METER_SCALE_WATER_CUBIC_FEET 1
#define ZNET_METER_SCALE_WATER_US_GALLONS 2
#define ZNET_METER_SCALE_WATER_PULSE_COUNT 3
#define ZNET_METER_SCALE_HEATING_kWh 0
#define ZNET_METER_SCALE_COOLING_kWh 0

/**
 * @brief Rate type specifies if it is import or export values to be read.
 *
 * Setting the rate type to import on the meter report is an indication
 * that the meter value is a consumed measurement. In contrary when
 * the rate type is set to export the indication of the meter value is
 * a produced measurement.
 */
#define ZNET_METER_RATE_TYPE_IMPORT 0x01
#define ZNET_METER_RATE_TYPE_EXPORT 0x02

/**
 * @brief Meter report
 */
typedef struct znet_meter_report_t
{
    uint8_t __ver;     /**< reserved */
    uint8_t type;      /**< Meter type */
    uint16_t scale;    /**< Scale of values */
    uint32_t value;    /**< Current value */
    uint8_t precision; /**< Precision (real value = value / (10 ^ precision)) */
    uint8_t rate_type; /**< Rate type */
    uint16_t dtime;    /**< Delta time between previos and current value */
    uint32_t prev_value; /**< Previos value */
} znet_meter_report_t;

/**
 * @brief TBD.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_METER_RESULT )( int err, znet_node_id_t node_id,
                                              znet_node_channel_id_t channel_id,
                                              const znet_meter_report_t* value,
                                              void* arg );

/**
 * @brief Meter supported report
 */
typedef struct znet_meter_supported_report_t
{
    uint8_t __ver;       /**< reserved */
    uint8_t type;        /**< Meter type */
    uint8_t reset;       /**< flag: support meter erset */
    uint8_t rate_import; /**< flag: support rate type import */
    uint8_t rate_export; /**< flag: support rate type export */
    uint8_t scount;      /**< Scales count */
    uint8_t scales[];    /**< flag: support scale */
} znet_meter_supported_report_t;

/**
 * @brief TBD.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_METER_SUPPORTED_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_meter_supported_report_t* value, void* arg );

/**
 * @brief Multi channel end point report
 */
typedef struct znet_multichannel_endpoint_report_t
{
    uint8_t __ver;     /**< reserved */
    uint8_t dynamic;   /**< flag: implemented a dynamic number of end points */
    uint8_t identical; /**< flag: all end points have identical capabilities */
    uint8_t channel_ids_count;           /**< End points count */
    uint8_t agregated_channel_ids_count; /**< Agregated end points count */
} znet_multichannel_endpoint_report_t;

/**
 * @brief TBD.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MULTICHANNEL_ENDPOINT_RESULT )(
    int err, znet_node_id_t node_id,
    const znet_multichannel_endpoint_report_t* value, void* arg );

/**
 * @brief Multi channel capability report
 */
typedef struct znet_multichannel_capability_report_t
{
    uint8_t __ver;                     /**< reserved */
    uint8_t dynamic;                   /**< flag: Channel ID is dynamic */
    znet_node_channel_id_t channel_id; /**< Channel ID */
    znet_generic_class_t generic;      /**< Generic class type */
    znet_specific_class_t specific;    /**< Specific class type */
    uint8_t commands_count;            /**< Commands classes count */
    znet_command_class_t commands[];   /**< Commands classes, max 35 */
} znet_multichannel_capability_report_t;

/**
 * @brief TBD.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MULTICHANNEL_CAPABILITY_RESULT )(
    int err, znet_node_id_t node_id,
    const znet_multichannel_capability_report_t* value, void* arg );

/**
 * @brief Multi channel end point report
 */
typedef struct znet_multichannel_endpoint_find_report_t
{
    uint8_t __ver;                        /**< reserved */
    znet_generic_class_t generic;         /**< Generic class type */
    znet_specific_class_t specific;       /**< Specific class type */
    uint8_t channel_ids_count;            /**< End points count */
    znet_node_channel_id_t channel_ids[]; /**< End points */
} znet_multichannel_endpoint_find_report_t;

/**
 * @brief TBD.
 *
 * It is called many times. value is NULL if
 * there are no more reports.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MULTICHANNEL_ENDPOINT_FIND_RESULT )(
    int err, znet_node_id_t node_id,
    const znet_multichannel_endpoint_find_report_t* value, void* arg );

/**
 * @brief Multi Channel Aggregated Members report
 */
typedef struct znet_multichannel_aggregated_members_report_t
{
    uint8_t __ver;                                  /**< reserved */
    znet_node_channel_id_t channel_id;              /**< Generic class type */
    uint8_t agregated_channel_ids_count;            /**< End points count */
    znet_node_channel_id_t agregated_channel_ids[]; /**< End points */
} znet_multichannel_aggregated_members_report_t;

/**
 * @brief TBD.
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_MULTICHANNEL_AGGREGATED_MEMBERS_RESULT )(
    int err, znet_node_id_t node_id,
    const znet_multichannel_aggregated_members_report_t* value, void* arg );

/**
 * @brief Configuration specific: Parameter types
 */
typedef uint16_t znet_cmd_configuration_id_t;
typedef uint32_t znet_cmd_configuration_value_t;

#define ZNET_CMD_CONFIGURATION_PARAM_NUM_INVALID        0x03
#define ZNET_CMD_CONFIGURATION_PARAM_NUM_MAX            0x04
#define ZNET_CMD_CONFIGURATION_REPORT_CHECK_LEN         0x04

/**
 * @brief Configuration reports: min length constants
 * ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN - parameter
 * for Name Report and Info Report;
 */
#define ZNET_CMD_CONFIGURATION_BULK_REPORT_CHECK_LEN    0x07
#define ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN     0x05
#define ZNET_CMD_CONFIGURATION_PROP_REPORT_CHECK_LEN    0x06

/**
 * @brief  Command Class, version 1: Configuration Report
 */
typedef struct znet_configuration_report_t
{
    uint8_t __ver;                        /**< reserved */
    uint8_t param_number;                 /**< Parameter Number */
    uint8_t data_count;                   /**< Size of the actual parameter. */
    znet_cmd_configuration_value_t value; /**< Configuration Data */
} znet_configuration_report_t;

/**
 * @brief  Command Class, versions 2-4: Configuration Bulk Report
 */
typedef struct znet_configuration_bulk_report_t
{
    uint8_t __ver;                              /**< reserved */
    znet_cmd_configuration_id_t param_offset;   /**< Parameter Offset */
    uint8_t param_number;                       /**< Parameter Number */
    uint8_t rep_to_follows;                     /**< Report to follows */
    uint8_t data_count;                         /**< Size of the actual parameter. */
    uint8_t data[];                             /**< Configuration Data */
} znet_configuration_bulk_report_t;

/**
 * @brief  Command Class, versions 3-4: Configuration Name Report
 */
typedef struct znet_configuration_name_report_t
{
    uint8_t __ver;                              /**< reserved */
    znet_cmd_configuration_id_t param_number;   /**< Parameter Number */
    uint8_t rep_to_follows;                     /**< Report to follows */
    uint8_t data[];                             /**< Configuration Data */
} znet_configuration_name_report_t;

/**
 * @brief  Command Class, versions 3-4: Configuration Info Report
 */
typedef struct znet_configuration_info_report_t
{
    uint8_t __ver;                              /**< reserved */
    znet_cmd_configuration_id_t param_number;   /**< Parameter Number */
    uint8_t rep_to_follows;                     /**< Report to follows */
    uint8_t data[];                             /**< Configuration Data */
} znet_configuration_info_report_t;

/**
 * @brief  Command Class, versions 3-4: Configuration Properties Report
 */
typedef struct znet_configuration_properties_report_t
{
    uint8_t __ver;                              /**< reserved */
    znet_cmd_configuration_id_t param_number;   /**< Parameter Number */
    uint8_t data_format;                        /**< Format of the actual parameter. */
    uint8_t data_size;                          /**< Size of the actual parameter. */
    uint8_t data[];                             /**< Configuration Data */
} znet_configuration_properties_report_t;

/**
 * @brief Function prototype for notify: node cmd configuration report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current configuration value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_CONFIGURATION_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_configuration_report_t* value, void* arg );

 /**
 * @brief Function prototype for notify: node cmd configuration bulk report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current configuration value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_CONFIGURATION_BULK_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_configuration_bulk_report_t* value, void* arg );

 /**
 * @brief Function prototype for notify: node cmd configuration name report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current configuration value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_CONFIGURATION_NAME_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_configuration_name_report_t* value, void* arg );

 /**
 * @brief Function prototype for notify: node cmd configuration info report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param value Current configuration value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_CONFIGURATION_INFO_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_configuration_info_report_t* value, void* arg );

 /**
 * @brief Function prototype for notify: node cmd configuration info report/result
 *
 * @param err Return zero on success. On error, other value is returned.
 * @param node_id Node ID
 * @param config_value Current configuration value
 * @param arg Parameter for callback functions
 */
typedef void ( *ZNET_NODE_CMD_CONFIGURATION_PROPERTIES_RESULT )(
    int err, znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    const znet_configuration_properties_report_t* value, void* arg );

/**
 * @brief TBD.
 */
typedef struct znet_callbacks_t
{
    void* arg; /**< Parameter for callback functions */

    ZNET_ALLOC alloc; /**< Func for memmory management [req] */
    ZNET_CLOCK clock; /**< Func for get monotonic time [req] */
    ZNET_LOG log;     /**< Func for logging [req] */

    ZNET_UART_WRITE uart_write; /**< Func for write data to uart [req] */
    ZNET_UART_READ uart_read;   /**< Func for read data from uart [req] */
    ZNET_UART_RESET uart_reset; /**< Func for reset uart (or buffers) [opt] */

    ZNET_STORE_SAVE store_save;   /**< Func for save data [req] */
    ZNET_STORE_LOAD store_load;   /**< Func for load data [req] */
    ZNET_STORE_RESET store_reset; /**< Func for reset store [req] */

    ZNET_SET_DEFAULT_RESULT
    set_default; /**< Func for async result of set_default [opt] */
    ZNET_NODE_ADD_RESULT
    node_add_result; /**< Func for async result of node_add [opt] */
    ZNET_NODE_REM_RESULT
    node_rem_result; /**< Func for async result of node_rem [opt] */
    ZNET_NODE_LIST_RESULT
    node_list_result; /**< Func for async result of node_list [opt] */

    ZNET_NODE_CMD_VERSION_RESULT
    node_cmd_version_result; /**< Function for async result/report of
                                cmd_version [opt] */
    ZNET_NODE_CMD_COMMAND_VERSION_RESULT
    node_cmd_command_version_result; /**< Function for async result/report of
                                        cmd_command_version [opt] */
    ZNET_NODE_CMD_MANUFACTURER_SPECIFIC_RESULT
    node_cmd_manufacturer_specific_result; /**< Function for async result/report
                                              of cmd_manufacturer_specific [opt]
                                            */
    ZNET_NODE_CMD_DEVICE_SPECIFIC_RESULT
    node_cmd_device_specific_result; /**< Function for async result/report of
                                              cmd_device_specific [opt] */
    ZNET_NODE_CMD_ZWAVEPLUS_INFO_RESULT
    node_cmd_zwaveplus_info_result; /**< Function for async result/report of
                                       cmd_zwaveplus_info [opt] */
    ZNET_NODE_CMD_BASIC_RESULT
    node_cmd_basic_result; /**< Func for async result/report of cmd_basic [opt]
                            */
    ZNET_NODE_CMD_BINARY_SWITCH_RESULT
    node_cmd_binary_switch_result; /**< Func for async result/report of
                                      cmd_binary_switch [opt] */
    ZNET_NODE_CMD_METER_RESULT
    node_cmd_meter_result; /**< Func for async result/report of cmd_meter [opt]
                            */
    ZNET_NODE_CMD_METER_SUPPORTED_RESULT
    node_cmd_meter_supported_result; /**< Func for async result/report of
                                        cmd_meter_supported [opt] */
    ZNET_NODE_CMD_MULTILEVEL_SWITCH_RESULT
    node_cmd_multilevel_switch_result; /**< Func for async result/report of
                                      cmd_multilevel_switch [opt] */
    ZNET_NODE_CMD_MULTICHANNEL_ENDPOINT_RESULT
    node_cmd_multichannel_endpoint_result; /**< Func for async result/report of
                                                cmd_multichannel_endpoint [opt]
                                            */
    ZNET_NODE_CMD_MULTICHANNEL_CAPABILITY_RESULT
    node_cmd_multichannel_capability_result; /**< Func for async result/report
                                                of cmd_multichannel_capability
                                                [opt] */
    ZNET_NODE_CMD_MULTICHANNEL_ENDPOINT_FIND_RESULT
    node_cmd_multichannel_endpoint_find_result; /**< Func for async
                                                result/report of
                                                cmd_multichannel_endpoint_find
                                                [opt] */
    ZNET_NODE_CMD_MULTICHANNEL_AGGREGATED_MEMBERS_RESULT
    node_cmd_multichannel_aggregated_members_result; /**< Func for async
                                                result/report of
                                                cmd_multichannel_aggregated_members
                                                [opt] */
    ZNET_NODE_CMD_CONFIGURATION_RESULT
    node_cmd_configuration_result; /**< Func for async result/report of
                                      cmd_configiration [opt] */
    ZNET_NODE_CMD_CONFIGURATION_BULK_RESULT
    node_cmd_configuration_bulk_result; /**< Func for async result/report of
                                      cmd_configiration_bulk [opt] */
    ZNET_NODE_CMD_CONFIGURATION_NAME_RESULT
    node_cmd_configuration_name_result; /**< Func for async result/report of
                                      cmd_configiration_name [opt] */
    ZNET_NODE_CMD_CONFIGURATION_INFO_RESULT
    node_cmd_configuration_info_result; /**< Func for async result/report of
                                      cmd_configiration_info [opt] */
    ZNET_NODE_CMD_CONFIGURATION_PROPERTIES_RESULT
    node_cmd_configuration_properties_result; /**< Func for async result/report of
                                      cmd_configiration_properties [opt] */
    /// TODO: to declare others callback functions
} znet_callbacks_t;

/**
 * @brief Init znet library
 *
 * @param Callbacks. Do not delete the znet_callbacks_t structure while using
 * the library!!!
 * @return Return zero on success. On error, -1 is returned
 */
int znet_init( const znet_callbacks_t* callbacks );

/**
 * @brief Main handler
 *
 * Need periodically call to process incoming data from the chip.
 *
 * Example of use case:
 * @code
 * for(;;) {
 *     znet_proc();
 *     usleep(1000);
 * }
 * @endcode
 */
void znet_proc( void );

/**
 * @brief Set default
 *
 * Set the Controller back to the factory default state.
 */
void znet_set_default( void );

/**
 * @brief Add node to network
 *
 * Run the process of inclusion a new node to the network.
 */
void znet_node_add( void );

/**
 * @brief Remove any node from network
 *
 * Run the process of exclusion a node from the enetwork.
 */
void znet_node_rem( void );

/**
 * @brief Stop remove any node from network
 *
 * Break the running process of exclusion a node from the enetwork.
 */
void znet_node_rem_break( void );

/**
 * @brief List known nodes
 *
 * Get a list of nodes known to the Controller.
 */
void znet_node_list( void );

/**
 * @brief Get version info
 *
 * @param node_id Node ID
 */
void znet_node_cmd_version_get( znet_node_id_t node_id );

/**
 * @brief Get command class version
 *
 * @param node_id Node ID
 * @param command Command class
 */
void znet_node_cmd_command_version_get( znet_node_id_t node_id,
                                        znet_command_class_t command );

/**
 * @brief Get manufacturer specific information.
 *
 * @param node_id Node ID
 */
void znet_node_cmd_manufacturer_specific_get( znet_node_id_t node_id );

/**
 * @brief Get device specific information.
 *
 * @param node_id Node ID
 * @param type Device ID Type
 */
void znet_node_cmd_device_specific_get( znet_node_id_t node_id,
                                        znet_cmd_device_specific_type_t type );

/**
 * @brief Get Z-Wave Plus Info.
 *
 * @param node_id Node ID
 */
void znet_node_cmd_zwaveplus_info_get( znet_node_id_t node_id );

/**
 * @brief Operate primary functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 * @param value Value
 */
void znet_node_cmd_basic_set(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_basic_value_t value );

/**
 * @brief Get state for primary functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_basic_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief Operate binary switch functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 * @param value Value
 */
void znet_node_cmd_binary_switch_set(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_binary_switch_value_t value );

/**
 * @brief Get state for binary switch functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_binary_switch_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief Query the value of a configuration parameter
 *
 * @param node_id Node ID
 * @param config_param_num Parameter Number
 */
void znet_node_cmd_configuration_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    uint8_t config_param_num);

/**
 * @brief Set the value of a configuration parameter
 *
 * @param node_id Node ID
 * @param config_param_num Parameter Number
 * @param config_size Size of the actual parameter
 * @param set_to_default Default flag
 * @param config_value Configuration Value (N bytes)
 */
void znet_node_cmd_configuration_set(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    uint8_t config_param_num, uint8_t config_size,
    int set_to_default, znet_cmd_configuration_value_t config_value);

/**
 * @brief Set the value of one or more configuration parameters.
 *
 * @param node_id Node ID
 * @param config_id Parameter Offset
 * @param config_count Number  of the configuration parameters
 * @param config_size Size of the actual parameters
 * @param need_report Default flag
 * @param set_to_default Handshake (get report) flag
 * @param config_value Configuration Value (N bytes)
 */
void znet_node_cmd_configuration_bulk_set(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_configuration_id_t config_id,
    uint8_t config_count, uint8_t config_size,
    int need_report, int set_to_default, const uint8_t* config_value);

/**
 * @brief Query the value of one or more configuration parameters
 *
 * @param node_id Node ID
 * @param config_id Parameter Offset
 * @param config_count Number  of the configuration parameters
 */
void znet_node_cmd_configuration_bulk_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_configuration_id_t config_id,
    uint8_t config_count );

/**
 * @brief Request the name of a configuration parameter
 *
 * @param node_id Node ID
 * @param param_number Requested configuration parameter
 */
void znet_node_cmd_configuration_name_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_configuration_id_t param_number);

/**
 * @brief Request usage information for a configuration parameter
 *
 * @param node_id Node ID
 * @param param_number Requested configuration parameter
 */
void znet_node_cmd_configuration_info_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_configuration_id_t param_number);

/**
 * @brief Request the properties of a configuration parameter
 *
 * @param node_id Node ID
 * @param param_number Requested configuration parameter
 */
void znet_node_cmd_configuration_properties_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_configuration_id_t param_number);

/**
 * @brief Reset all configuration parameters to their default value
 *
 * @param node_id Node ID
 */
void znet_node_cmd_configuration_default_reset(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */);

/**
 * @brief Operate multilevel switch functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 * @param value Value
 */
void znet_node_cmd_multilevel_switch_set(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_multilevel_switch_value_t value );

/**
 * @brief Get state for multilevel switch functionality of node
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_multilevel_switch_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief TBD.
 */
typedef enum znet_cmd_multilevel_switch_change_direction_t {
    ZNET_CMD_MULTILEVEL_SWITCH_CHANGE_DIRECTION_INC = 0,
    ZNET_CMD_MULTILEVEL_SWITCH_CHANGE_DIRECTION_DEC = 1,
} znet_cmd_multilevel_switch_change_direction_t;

/**
 * @brief Initiate a transition to a new level on multilevel switch
 *
 * For ignore start value -> set start_value to
 * ZNET_CMD_MULTILEVEL_SWITCH_VALUE_UNKNOWN
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 * @param direction Direction of change (0 - increasing, 1 - decreasing)
 * @param start_value Value
 */
void znet_node_cmd_multilevel_switch_start_change(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    znet_cmd_multilevel_switch_change_direction_t direction,
    znet_cmd_multilevel_switch_value_t start_value );

/**
 * @brief Terminate a transition to a new level on multilevel switch
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_multilevel_switch_stop_change(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief Request the accumulated consumption in physical units from a metering
 * device.
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 * @param scale Scale advertise the unit used
 */
void znet_node_cmd_meter_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */,
    uint16_t scale );

/**
 * @brief Request the supported scales in a sub meter.
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_meter_supported_get(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief Reset the accumulated consumption in physical units on a metering
 * device.
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_meter_reset(
    znet_node_id_t node_id,
    znet_node_channel_id_t channel_id /* = ZNET_CHANNEL_ID_ROOT */ );

/**
 * @brief Query the number of End Points implemented by the node.
 *
 * @param node_id Node ID
 */
void znet_node_cmd_multichannel_endpoint_get( znet_node_id_t node_id );

/**
 * @brief Query the non-secure capabilities of an End Point.
 *
 * @param node_id Node ID
 * @param channel_id Channel ID
 */
void znet_node_cmd_multichannel_capability_get(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id );

/**
 * @brief Search for a specific set of Generic and Specific Device Class.
 *
 * @param node_id Node ID
 * @param generic Generic Class
 * @param specific Specific Class
 */
void znet_node_cmd_multichannel_endpoint_find( znet_node_id_t node_id,
                                               znet_generic_class_t generic,
                                               znet_specific_class_t specific );

/**
 * @brief Query the members of an Aggregated End Point.
 *
 * @param node_id Node ID
 * @param channel_id Aggregated Channel ID
 */
void znet_node_cmd_multichannel_aggregated_members_get(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id );

/// INFO: utils

/**
 * @brief
 */
#define znet_node_info_size( node_info ) \
    ( sizeof( znet_nodeinfo_t ) +        \
      node_info->commands_count * sizeof( znet_command_class_t ) )

/**
 * @brief Get node info from cache
 *
 * Two mode:
 * If node_info is NULL then return recommended node_info_size
 * If node_info is noit NULL then return node info and actual node_info_size
 * size
 *
 * Example of use:
 * @code
 * size_t nis = 0;
 * size_t nis_tmp;
 * znet_nodeinfo_t* ni = NULL;
 * if( znet_node_info( 1, ni, &nis ) )
 * {
 *     // error
 *     exit();
 * }
 * ni = (znet_nodeinfo_t*)malloc( nis );
 * nis_tmp = nis;
 * if( znet_node_info( 1, ni, &nis_tmp ) )
 * {
 *     // error or not found
 *     exit();
 * }
 * // use ni
 * // optionally: recovery nis_tmp and get another node_info
 * free( ni );
 * @endcode
 *
 * If node info not found return error
 *
 * @param node_id Node ID
 * @param node_info Buffer for node info
 * @param node_info_size [IN] Buffer size [OUT] node info size
 * @return Return zero on success. On error, other value is returned.
 */

int znet_node_info( znet_node_id_t node_id, znet_nodeinfo_t* node_info,
                    size_t* node_info_size );

#ifdef __cplusplus
}
#endif

#endif  // ZNET_H
