/**
 * libvmod-geoip2 - varnish interface to MaxMind's libmaxminddb library
 * libmaxminddb API: https://github.com/maxmind/libmaxminddb/releases
 *
 * See file README.rst for usage instructions
 *
 * This code is licensed under a MIT-style License, see file LICENSE
*/

#include <stdlib.h>
#include <maxminddb.h>
#include <string.h>

#include "cache/cache.h"
#include "vcl.h"

#ifndef VRT_H_INCLUDED
#include "vrt.h"
#endif

#include "vcc_if.h"

#define DEBUG 0

static MMDB_s mmdb_handle;

// close gets called by varnish when then the threads destroyed
static void close_mmdb(VRT_CTX, void *mmdb_handle)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	// don't do anything if the db didn't open correctly.
	if (mmdb_handle == NULL) {
		return;
	}
	MMDB_s *handle = (MMDB_s *)mmdb_handle;
	MMDB_close(handle);
}

static const struct vmod_priv_methods close_mmdb_vmod_priv_methods[1] = {{
	.magic = VMOD_PRIV_METHODS_MAGIC,
	.type = "close_mmdb",
	.fini = close_mmdb
}};

VCL_INT
vmod_init(VRT_CTX, struct vmod_priv *pp, const char *mmdb_path)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

    int mmdb_baddb = MMDB_open(mmdb_path, MMDB_MODE_MMAP, &mmdb_handle);
    if (mmdb_baddb == MMDB_SUCCESS) {
    	pp->priv = (void *)&mmdb_handle;
    	pp->methods = close_mmdb_vmod_priv_methods;
    	return 0;
    } else {
        fprintf(stderr, "[ERROR] MMDB_open: Can't open %s - %s\n",
                mmdb_path, MMDB_strerror(mmdb_baddb));
        if (MMDB_IO_ERROR == mmdb_baddb) {
            fprintf(stderr,
                    "[ERROR] MMDB_open: IO error: %s\n",
                    strerror(mmdb_baddb));
        }
        return 1;
    }
}

/**
 * geo_lookup takes a handle to the mmdb, an IP address and a lookup path
 * and returns a string of the value or NULL.
 * lookup_path is described in this doc: http://maxmind.github.io/MaxMind-DB/
 */
char *
geo_lookup(MMDB_s *const mmdb_handle, const char *ipstr, const char **lookup_path)
{
	char *data = NULL;
	// Lookup IP in the DB
	int gai_error = 0;
	int mmdb_error = 0;
	MMDB_lookup_result_s result =
		MMDB_lookup_string(mmdb_handle, ipstr, &gai_error, &mmdb_error);

	if (0 != gai_error) {
#if DEBUG
		fprintf(stderr,
				"[INFO] Error from MMDB_lookup_string for %s - %s\n\n",
				ipstr, gai_strerror(gai_error));
#endif
		return NULL;
	}

	if (MMDB_SUCCESS != mmdb_error) {
#if DEBUG
		fprintf(stderr,
				"[ERROR] Got an error from libmaxminddb: %s\n\n",
				MMDB_strerror(mmdb_error));
#endif
		return NULL;
	}

	// Parse results
	MMDB_entry_data_s entry_data;

	if (result.found_entry) {
		int status = MMDB_aget_value(&result.entry, &entry_data, lookup_path);

		if (MMDB_SUCCESS != status) {
#if DEBUG
			fprintf(
				stderr,
				"[WARN] Got an error looking up the entry data. Make sure \
 the lookup_path is correct. %s\n",
				MMDB_strerror(status));
#endif
			return NULL;
		}

		if (entry_data.has_data) {
			switch(entry_data.type) {
			case MMDB_DATA_TYPE_UTF8_STRING: {
				data = strndup(entry_data.utf8_string, entry_data.data_size);
				break;
			}
			case MMDB_DATA_TYPE_UINT16: {
				uint16_t num = UINT16_MAX;
				int len      = (int)((ceil(log10(num)))*sizeof(char));
				data         = calloc(sizeof(char), len+1);
				if (data != NULL) {
					snprintf(data, len+1, "%u", entry_data.uint16);
				}
				break;
			}
			default:
#if DEBUG
				fprintf(
					stderr,
					"[WARN] No handler for entry data type (%d) was found\n",
					entry_data.type);
#endif
				break;
			}
		} else {
			return NULL;
		}
	} else {
#if DEBUG
		fprintf(
			stderr,
			"[INFO] No entry for this IP address (%s) was found\n",
			ipstr);
#endif
		return NULL;
	}
	return data;
}

// Lookup a field
VCL_STRING
vmod_lookup(VRT_CTX, struct vmod_priv *global, const char *ipstr, const char **lookup_path)
{
    char *data = NULL;
    char *cp   = NULL;
    MMDB_s * mmdb_handle = (struct MMDB_s *)global->priv;

    if (mmdb_handle == NULL) {
        fprintf(stderr, "[WARN] varnish gave NULL maxmind db handle");
        return cp;
    }
    data = geo_lookup(mmdb_handle, ipstr, lookup_path);

    if (data != NULL) {
        cp = WS_Copy(ctx->ws, data, strlen(data)+1);
        free(data);
    } else {
        cp = WS_Copy(ctx->ws, "", 1);
    }

    return cp;
}

VCL_STRING
vmod_country_code(VRT_CTX, struct vmod_priv *global, const char *ipstr)
{
    const char *lookup_path[] = {"country", "iso_code", NULL};
    return vmod_lookup(ctx, global, ipstr, lookup_path);
}

VCL_STRING
vmod_country_name(VRT_CTX, struct vmod_priv *global, const char *ipstr)
{
    const char *lookup_path[] = {"country", "names", "en", NULL};
    return vmod_lookup(ctx, global, ipstr, lookup_path);
}

VCL_STRING
vmod_region_code(VRT_CTX, struct vmod_priv *global, const char *ipstr)
{
    const char *lookup_path[] = {"subdivisions", "0", "iso_code", NULL};
    return vmod_lookup(ctx, global, ipstr, lookup_path);
}

VCL_STRING
vmod_region_name(VRT_CTX, struct vmod_priv *global, const char *ipstr)
{
    const char *lookup_path[] = {"subdivisions", "0", "names", "en", NULL};
    return vmod_lookup(ctx, global, ipstr, lookup_path);
}

VCL_STRING
vmod_city_name(VRT_CTX, struct vmod_priv *global, const char *ipstr)
{
    const char *lookup_path[] = {"city", "names", "en", NULL};
    return vmod_lookup(ctx, global, ipstr, lookup_path);
}
