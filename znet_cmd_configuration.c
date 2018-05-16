/**
 * @file znet_cmd_binary_switch.c
 * @date 16 May 2018
 * @brief TBD.
 */

/// INFO: crt & system
#include <assert.h>
#include <string.h>

/// INFO: public
#include <znet/znet.h>

/// INFO: private
#include "znet_main.h"
#include "znet_log.h"
#include "znet_store.h"

/// INFO: internal
#include "heap.h"
#include <znet_lib.h>
#include <znet_lib_cc_application.h>

// node cmd configuration report ///////////////////////////////////////
/// INFO:  Configuration_Report Command Class v1
void znet_cc_configuration_report( const ZFunction func, uint8_t node_id,
                                   int cc_data_len, const uint8_t* cc_data )
{
    assert( cc_data );
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_REPORT_CHECK_LEN );
    assert( cc_data[0] == ZNET_COMMAND_CLASS_CONFIGURATION );
    assert( cc_data[1] == CONFIGURATION_REPORT );
    assert( cc_data[2] );

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        return;

    uint8_t param_size = cc_data[3] & CONFIGURATION_SET_LEVEL_SIZE_MASK;
    if( param_size > ZNET_CMD_CONFIGURATION_PARAM_NUM_MAX || param_size == 0 ||
        param_size == ZNET_CMD_CONFIGURATION_PARAM_NUM_INVALID )
    {
        ZNET_LOGE( "ZNET: Invalid size ID!\n" );
        return;
    }

    znet_configuration_report_t report = {};
    report.param_number = cc_data[2];
    report.data_count = param_size;
    report.value = cc_data[4];
    if( report.data_count > 1 )
    {
        report.value = ( report.value  << 8 ) | ((uint32_t)cc_data[5]);
    }
    if( report.data_count > 2 )
    {
        report.value = ( report.value  << 8 ) | ((uint32_t)cc_data[6]);
        report.value = ( report.value  << 8 ) | ((uint32_t)cc_data[7]);
    }

    if( znet_cb->node_cmd_configuration_result )
        znet_cb->node_cmd_configuration_result( 0, node_id, func->_endpoint,
                                                &report, znet_cb->arg );
}

/// INFO: Configuration_Get Command Class v1

static void _znet_node_cmd_configuration_get_cb( ZFunction func, void* arg,
                                         ZFuncFailures_e reason )
{
    if( FUNC_OK != reason )
    {
        if( znet_cb->node_cmd_configuration_result )
            znet_cb->node_cmd_configuration_result(
                -1, ZNET_NODE_ID_INVALID, func->_endpoint, NULL, znet_cb->arg );
    }
}

void znet_node_cmd_configuration_get(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    uint8_t config_param_num )
{
    assert( config_param_num );
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        if( znet_cb->node_cmd_configuration_result )
            znet_cb->node_cmd_configuration_result( -1, ZNET_NODE_ID_INVALID,
                                             channel_id, NULL, znet_cb->arg );

     /// TODO: check in storage node_id
    /// TODO: check wait report for node_id

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    if( !znet_cc_configuration_get( &znet, node_id, config_param_num,
                            _znet_node_cmd_configuration_get_cb, callbackArg, encap ) )
    {
        if( znet_cb->node_cmd_configuration_result )
            znet_cb->node_cmd_configuration_result( -1, ZNET_NODE_ID_INVALID,
                                            channel_id, NULL, znet_cb->arg );
    }
}

/// INFO: Configuration_Set Command Class v1
void znet_node_cmd_configuration_set(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    uint8_t config_param_num, uint8_t config_size,
    int set_to_default, znet_cmd_configuration_value_t config_value)
{
    assert( config_param_num > 0 );

    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        ZNET_LOGE( "ZNET: Invalid node ID!\n" );
        return;
    }

    uint8_t temp_val = config_size & CONFIGURATION_SET_LEVEL_SIZE_MASK;
    if( temp_val > ZNET_CMD_CONFIGURATION_PARAM_NUM_MAX || temp_val == 0 ||
        temp_val == ZNET_CMD_CONFIGURATION_PARAM_NUM_INVALID )
    {
        ZNET_LOGE( "ZNET: Invalid size ID!\n" );
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }
    znet_cc_configuration_set(&znet, node_id, config_param_num, ( set_to_default ? TRUE : FALSE ),
        config_value, config_size,  NULL,  callbackArg, encap );
}

/// INFO: Configuration Command Class v2
void znet_cc_configuration_bulk_report( const ZFunction func, uint8_t node_id,
                                   int cc_data_len, const uint8_t* cc_data )
{
    assert( cc_data );
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_BULK_REPORT_CHECK_LEN );
    assert( cc_data[0] == ZNET_COMMAND_CLASS_CONFIGURATION );
    assert( cc_data[1] == CONFIGURATION_BULK_REPORT_V4 );
    assert( cc_data[4] );

    //TBD: consider usage of cc_data[5] -> Reports to follow
    uint16_t temp_value = ((uint16_t)cc_data[2] << 8) | cc_data[3];
    if( temp_value == 0 )
    {
        ZNET_LOGE( "ZNET: Invalid offset parameter!\n" );
        return;
    }

    uint8_t param_size = cc_data[6] & CONFIGURATION_BULK_REPORT_PROPERTIES1_SIZE_MASK_V2;
    if( param_size > ZNET_CMD_CONFIGURATION_PARAM_NUM_MAX || param_size == 0 ||
        param_size == ZNET_CMD_CONFIGURATION_PARAM_NUM_INVALID )
    {
        ZNET_LOGE( "ZNET: Invalid size ID!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        return;

    /* Configuration Value is (M*N bytes) where N = param_size
       and number of parameters M = cc_data[4] */
    size_t data_count = param_size * cc_data[4];
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_BULK_REPORT_CHECK_LEN + data_count );

    const size_t buff_size = sizeof( znet_configuration_bulk_report_t ) + data_count;
    uint8_t buff[buff_size];
    memset( buff, 0, buff_size );

    znet_configuration_bulk_report_t* bulk_report = (znet_configuration_bulk_report_t*)&buff;
    bulk_report->param_offset =  temp_value;
    bulk_report->param_number = cc_data[4];
    bulk_report->rep_to_follows = cc_data[5];
    bulk_report->data_count = param_size;

    for( size_t i = 0; i < data_count; i++ )
        bulk_report->data[i] = cc_data[ZNET_CMD_CONFIGURATION_BULK_REPORT_CHECK_LEN + i];

    /// TODO: check in storage node_id
    /// TODO: check wait report for node_id done!
    if( znet_cb->node_cmd_configuration_bulk_result )
        znet_cb->node_cmd_configuration_bulk_result( 0, node_id, func->_endpoint,
                                                bulk_report, znet_cb->arg );
}

void znet_node_cmd_configuration_bulk_set(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    znet_cmd_configuration_id_t config_id,
    uint8_t config_count, uint8_t config_size,
    int need_report, int set_to_default, const uint8_t* config_value)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        return;
    }

    uint8_t temp_val = config_size & CONFIGURATION_SET_LEVEL_SIZE_MASK;
    if( temp_val > ZNET_CMD_CONFIGURATION_PARAM_NUM_MAX || temp_val == 0 ||
        temp_val == ZNET_CMD_CONFIGURATION_PARAM_NUM_INVALID )
    {
        ZNET_LOGE( "ZNET: Invalid size ID!\n" );
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    znet_cc_configuration_bulk_set( &znet, node_id, config_id, config_count,
        ( set_to_default ? TRUE : FALSE ), ( need_report ? TRUE : FALSE ),
        temp_val, config_value, NULL, callbackArg, encap );
}

void znet_node_cmd_configuration_bulk_get(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    znet_cmd_configuration_id_t config_id, uint8_t config_count)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    if( !znet_cc_configuration_bulk_get( &znet, node_id, config_id,
                            config_count, NULL, callbackArg, encap  ) )
    {
        if( znet_cb->node_cmd_configuration_bulk_result )
            znet_cb->node_cmd_configuration_bulk_result(
                                        -1, ZNET_NODE_ID_INVALID,
                                        channel_id, NULL, znet_cb->arg );
    }
}

/// INFO: Configuration Command Class v3
void znet_cc_configuration_name_report( const ZFunction func, uint8_t node_id,
                                   int cc_data_len, const uint8_t* cc_data  )
{
    assert( cc_data );
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN );
    assert( cc_data[0] == ZNET_COMMAND_CLASS_CONFIGURATION );
    assert( cc_data[1] == CONFIGURATION_NAME_REPORT_V4 );

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        return;
    //TBD: consider usage of cc_data[4] -> Reports to follow

    uint16_t name_count = ((uint16_t)cc_data[2] << 8) | cc_data[3];
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN + name_count );

    const size_t buff_size = sizeof( znet_configuration_name_report_t ) + name_count;
    uint8_t buff[buff_size];
    memset( buff, 0, buff_size );

    znet_configuration_name_report_t* name_report = (znet_configuration_name_report_t*)&buff;
    name_report->param_number = name_count;
    name_report->rep_to_follows = cc_data[4];
    for (int i = 0; i < name_count; i++)
        name_report->data[i] = cc_data[ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN + i];

    /// TODO: check in storage node_id
    /// TODO: check wait report for node_id done!
    if( znet_cb->node_cmd_configuration_name_result )
        znet_cb->node_cmd_configuration_name_result( 0, node_id, func->_endpoint,
                                                name_report, znet_cb->arg );
}

void znet_node_cmd_configuration_name_get(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id,
    znet_cmd_configuration_id_t param_number)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    if( !znet_cc_configuration_name_get( &znet, node_id, param_number,
                                            NULL, callbackArg, encap  ) )
    {
        if( znet_cb->node_cmd_configuration_name_result )
            znet_cb->node_cmd_configuration_name_result(
                                        -1, ZNET_NODE_ID_INVALID,
                                        channel_id, NULL, znet_cb->arg );
    }
}

void znet_cc_configuration_info_report( const ZFunction func, uint8_t node_id,
                                   int cc_data_len, const uint8_t* cc_data  )
{
    assert( cc_data );
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN );
    assert( cc_data[0] == ZNET_COMMAND_CLASS_CONFIGURATION );
    assert( cc_data[1] == CONFIGURATION_INFO_REPORT_V4 );

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        return;
    //TBD: consider usage of cc_data[4] -> Reports to follow

    uint16_t info_count = ((uint16_t)(cc_data[2] << 8)) | cc_data[3];
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN + info_count );

    const size_t buff_size = sizeof( znet_configuration_info_report_t ) + info_count;
    uint8_t buff[buff_size];
    memset( buff, 0, buff_size );

    znet_configuration_info_report_t* info_report =
        (znet_configuration_info_report_t*)&buff;
    info_report->param_number = info_count;
    info_report->rep_to_follows = cc_data[4];
    for (int i = 0; i < info_count; i++)
        info_report->data[i] = cc_data[ZNET_CMD_CONFIGURATION_NIP_REPORT_CHECK_LEN + i];

    /// TODO: check in storage node_id
    /// TODO: check wait report for node_id done!
    if( znet_cb->node_cmd_configuration_info_result )
        znet_cb->node_cmd_configuration_info_result( 0, node_id, func->_endpoint,
                                                info_report, znet_cb->arg );
}

void znet_node_cmd_configuration_info_get(
    znet_node_id_t node_id,  znet_node_channel_id_t channel_id,
    znet_cmd_configuration_id_t param_number)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    if( !znet_cc_configuration_info_get( &znet, node_id, param_number,
                                            NULL, callbackArg, encap  ) )
    {
        if( znet_cb->node_cmd_configuration_info_result )
            znet_cb->node_cmd_configuration_info_result(
                                        -1, ZNET_NODE_ID_INVALID,
                                        channel_id, NULL, znet_cb->arg );
    }
}

void znet_cc_configuration_properties_report( const ZFunction func, uint8_t node_id,
                                   int cc_data_len, const uint8_t* cc_data  )
{
    assert( cc_data );
    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_PROP_REPORT_CHECK_LEN );
    assert( cc_data[0] == ZNET_COMMAND_CLASS_CONFIGURATION );
    assert( cc_data[1] == CONFIGURATION_PROPERTIES_REPORT_V4 );
    assert( ( cc_data[4] & 0x07 ) != 0x03 );

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
        return;

    uint16_t param_num = ((uint16_t)cc_data[2] << 8) | cc_data[3];
    assert( param_num );

    size_t param_size = ( cc_data[4] & 0x07 ) * 0x03;

    assert( cc_data_len >= ZNET_CMD_CONFIGURATION_PROP_REPORT_CHECK_LEN + param_size );

    const size_t buff_size = sizeof( znet_configuration_properties_report_t ) + param_size;
    uint8_t buff[buff_size];
    memset( buff, 0, buff_size );

    znet_configuration_properties_report_t* prop_report =
        (znet_configuration_properties_report_t*)&buff;
    prop_report->param_number = param_num;
    prop_report->data_format = ( cc_data[4] & CONFIGURATION_PROPERTIES_REPORT_PROPERTIES1_FORMAT_MASK_V4 ) >> \
        CONFIGURATION_PROPERTIES_REPORT_PROPERTIES1_FORMAT_SHIFT_V4;
    prop_report->data_size = cc_data[4] & CONFIGURATION_PROPERTIES_REPORT_PROPERTIES1_SIZE_MASK_V4;
    for (int i = 0; i < ( param_size + 2 ); i++)
        prop_report->data[i] = cc_data[(ZNET_CMD_CONFIGURATION_PROP_REPORT_CHECK_LEN - 2) + i];

    /// TODO: check in storage node_id
    /// TODO: check wait report for node_id done!
    if( znet_cb->node_cmd_configuration_properties_result )
        znet_cb->node_cmd_configuration_properties_result( 0, node_id, func->_endpoint,
                                                prop_report, znet_cb->arg );
}

void znet_node_cmd_configuration_properties_get(
    znet_node_id_t node_id,  znet_node_channel_id_t channel_id,
    znet_cmd_configuration_id_t param_number)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    if( !znet_cc_configuration_properties_get( &znet, node_id, param_number,
         NULL, callbackArg, encap  ) )
    {
        if( znet_cb->node_cmd_configuration_properties_result )
            znet_cb->node_cmd_configuration_properties_result(
                                        -1, ZNET_NODE_ID_INVALID,
                                        channel_id, NULL, znet_cb->arg );
    }

}

void znet_node_cmd_configuration_default_reset(
    znet_node_id_t node_id, znet_node_channel_id_t channel_id)
{
    if( !znet_cb )
    {
        ZNET_LOGE( "ZNET: Library not initialized!\n" );
        return;
    }

    if( node_id < ZNET_NODE_ID_MIN || node_id > ZNET_NODE_ID_MAX )
    {
        ZNET_LOGE( "ZNET: Wrong node ID!\n" );
        return;
    }

    znet_node_channel_id_t from_to[2] = { ZNET_CHANNEL_ID_ROOT, channel_id };
    void* callbackArg = NULL;
    encap_type_t encap = Encapsulation_None;
    if( channel_id != ZNET_CHANNEL_ID_ROOT )
    {
        callbackArg = &from_to;
        encap |= Encapsulation_MuCh;
    }

    znet_cc_configuration_default_reset(&znet, node_id, NULL, callbackArg, Encapsulation_None );
}

