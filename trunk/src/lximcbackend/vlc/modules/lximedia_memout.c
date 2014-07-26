/*****************************************************************************
 * memout.c
 *****************************************************************************
 * Copyright (C) 2014 A.J. Admiraal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#define MODULE_STRING "lximedia_memout"
#define N_(X) (X)

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_block.h>

#include <vlc_input.h>
#include <vlc_playlist.h>

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int  Open ( vlc_object_t * );
static void Close( vlc_object_t * );

#define SOUT_CFG_PREFIX "sout-lximedia_memout-"

#define CALLBACK_TEXT N_( "Callback" )
#define CALLBACK_LONGTEXT N_( "Address of the callback function." )
#define OPAQUE_TEXT N_( "Opaque" )
#define OPAQUE_LONGTEXT N_( "Opaque pointer value, passed to " \
                            "callback function." )

vlc_module_begin ()
    set_description( N_("LXiMedia memory output") )
    set_capability( "sout access", 0 )
    set_shortname( N_("LXiMemOut") )
    add_shortcut( "lximedia_memout" )
    set_category( CAT_SOUT )
    set_subcategory( SUBCAT_SOUT_ACO )
    add_string( SOUT_CFG_PREFIX "callback", "",
                CALLBACK_TEXT, CALLBACK_LONGTEXT, true )
    add_string( SOUT_CFG_PREFIX "opaque", "",
                OPAQUE_TEXT, OPAQUE_LONGTEXT, true )
    set_callbacks( Open, Close )
vlc_module_end ()


/*****************************************************************************
 * Exported prototypes
 *****************************************************************************/
static const char *const ppsz_sout_options[] = {
    "callback", "opaque", NULL
};

static ssize_t Write( sout_access_out_t *, block_t * );
static int Seek ( sout_access_out_t *, off_t  );
static int Control( sout_access_out_t *, int, va_list );

struct sout_access_out_sys_t
{
    vlc_mutex_t *p_lock;
    void ( *pf_callback ) ( void *p_opaque, const uint8_t *p_data, size_t size );
    void *p_opaque;
};

/*****************************************************************************
 * Open: open the file
 *****************************************************************************/
static int Open( vlc_object_t *p_this )
{
    char* psz_tmp;
    sout_access_out_t       *p_access = (sout_access_out_t*)p_this;
    sout_access_out_sys_t   *p_sys;

    if( !( p_sys = p_access->p_sys =
                malloc( sizeof( sout_access_out_sys_t ) ) ) )
        return VLC_ENOMEM ;

    config_ChainParse( p_access, SOUT_CFG_PREFIX, ppsz_sout_options, p_access->p_cfg );

    psz_tmp = var_GetString( p_access, SOUT_CFG_PREFIX "callback" );
    p_sys->pf_callback = (void (*) (void *, const uint8_t *, size_t))(intptr_t)atoll( psz_tmp );
    free( psz_tmp );

    psz_tmp = var_GetString( p_access, SOUT_CFG_PREFIX "opaque" );
    p_sys->p_opaque = (void*)(intptr_t)atoll( psz_tmp );
    free( psz_tmp );

    p_access->pf_write       = Write;
    p_access->pf_seek        = Seek;
    p_access->pf_control     = Control;

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Close: close the target
 *****************************************************************************/
static void Close( vlc_object_t * p_this )
{
    sout_access_out_t       *p_access = (sout_access_out_t*)p_this;
    sout_access_out_sys_t   *p_sys = p_access->p_sys;

    msg_Dbg( p_access, "Close" );

    free( p_sys );
}

static int Control( sout_access_out_t *p_access, int i_query, va_list args )
{
    (void)p_access;

    switch( i_query )
    {
        case ACCESS_OUT_CONTROLS_PACE:
            *va_arg( args, bool * ) = true;
            break;

        default:
            return VLC_EGENERIC;
    }
    return VLC_SUCCESS;
}

/*****************************************************************************
 * Write:
 *****************************************************************************/
static ssize_t Write( sout_access_out_t *p_access, block_t *p_buffer )
{
    sout_access_out_sys_t *p_sys = p_access->p_sys;
    size_t i_write = 0;

    while( p_buffer )
    {
        if (p_sys->pf_callback && (p_buffer->i_buffer > 0))
            p_sys->pf_callback(p_sys->p_opaque, p_buffer->p_buffer, p_buffer->i_buffer);

        i_write += p_buffer->i_buffer;

        block_t *p_next = p_buffer->p_next;
        block_Release (p_buffer);
        p_buffer = p_next;
    }

    return i_write;
}

/*****************************************************************************
 * Seek: seek to a specific location in a file
 *****************************************************************************/
static int Seek( sout_access_out_t *p_access, off_t i_pos )
{
    (void)i_pos;
    msg_Warn( p_access, "HTTP sout access cannot seek" );
    return VLC_EGENERIC;
}
