#include <stdio.h>

#include <winpr/user.h>
#include <winpr/crypto.h>

#include <freerdp/settings.h>
#include <freerdp/codecs.h>

#include "settings_property_lists.h"
#include "../settings.h"

static BOOL log_result(BOOL value, const char* fkt)
{
	(void)fprintf(stderr, "TestSettings [%s] returned %s\n", fkt, value ? "TRUE" : "FALSE");
	return value;
}

static BOOL log_result_case(BOOL value, const char* fkt, size_t testcase)
{
	(void)fprintf(stderr, "TestSettings [%s] testcase %" PRIuz " returned %s\n", fkt, testcase,
	              value ? "TRUE" : "FALSE");
	return value;
}

static BOOL compare(const ADDIN_ARGV* got, const ADDIN_ARGV* expect)
{
	BOOL rc = TRUE;
	if (!got && !expect)
		return FALSE;
	if (!got && expect)
		return FALSE;
	if (got && !expect)
		return FALSE;
	if (got->argc != expect->argc)
		return FALSE;

	for (int x = 0; x < expect->argc; x++)
	{
		if (strcmp(got->argv[x], expect->argv[x]) != 0)
			rc = FALSE;
	}
	return log_result(rc, __func__);
}

static BOOL test_dyn_channels(void)
{
	BOOL rc = FALSE;
	BOOL test = 0;
	UINT32 u32 = 0;
	rdpSettings* settings = freerdp_settings_new(0);
	const char* argv1[] = { "foobar" };
	ADDIN_ARGV* args1 = NULL;
	const ADDIN_ARGV* cmp1 = NULL;
	const char* argv2[] = { "gaga", "abba", "foo" };
	ADDIN_ARGV* args2 = NULL;
	const ADDIN_ARGV* cmp2 = NULL;
	const ADDIN_ARGV* got = NULL;

	if (!settings)
		goto fail;

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount);
	if (u32 != 0)
		goto fail;

	/* Test the function return an error for unknown channels */
	test = freerdp_dynamic_channel_collection_del(settings, "foobar");
	if (test)
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "foobar");
	if (got)
		goto fail;

	/* Add the channel */
	cmp1 = args1 = freerdp_addin_argv_new(ARRAYSIZE(argv1), argv1);
	test = freerdp_dynamic_channel_collection_add(settings, args1);
	if (!test)
		goto fail;
	args1 = NULL; /* settings have taken ownership */

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount);
	if (u32 != 1)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelArraySize);
	if (u32 < 1)
		goto fail;

	cmp2 = args2 = freerdp_addin_argv_new(ARRAYSIZE(argv2), argv2);
	test = freerdp_dynamic_channel_collection_add(settings, args2);
	if (!test)
		goto fail;
	args2 = NULL; /* settings have taken ownership */

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount);
	if (u32 != 2)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelArraySize);
	if (u32 < 2)
		goto fail;

	/* Test the function return success for known channels */
	got = freerdp_dynamic_channel_collection_find(settings, "foobar");
	if (!compare(got, cmp1))
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "gaga");
	if (!compare(got, cmp2))
		goto fail;
	test = freerdp_dynamic_channel_collection_del(settings, "foobar");
	if (!test)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount);
	if (u32 != 1)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelArraySize);
	if (u32 < 1)
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "foobar");
	if (compare(got, cmp1))
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "gaga");
	if (!compare(got, cmp2))
		goto fail;
	test = freerdp_dynamic_channel_collection_del(settings, "gaga");
	if (!test)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount);
	if (u32 != 0)
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "foobar");
	if (compare(got, cmp1))
		goto fail;
	got = freerdp_dynamic_channel_collection_find(settings, "gaga");
	if (compare(got, cmp2))
		goto fail;

	rc = TRUE;

fail:
	freerdp_settings_free(settings);
	freerdp_addin_argv_free(args1);
	freerdp_addin_argv_free(args2);
	return log_result(rc, __func__);
}

static BOOL test_static_channels(void)
{
	BOOL rc = FALSE;
	BOOL test = 0;
	UINT32 u32 = 0;
	rdpSettings* settings = freerdp_settings_new(0);
	const char* argv1[] = { "foobar" };
	ADDIN_ARGV* args1 = NULL;
	const ADDIN_ARGV* cmp1 = NULL;
	const char* argv2[] = { "gaga", "abba", "foo" };
	ADDIN_ARGV* args2 = NULL;
	const ADDIN_ARGV* cmp2 = NULL;
	const ADDIN_ARGV* got = NULL;

	if (!settings)
		goto fail;

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount);
	if (u32 != 0)
		goto fail;

	/* Test the function return an error for unknown channels */
	test = freerdp_static_channel_collection_del(settings, "foobar");
	if (test)
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "foobar");
	if (got)
		goto fail;

	/* Add the channel */
	cmp1 = args1 = freerdp_addin_argv_new(ARRAYSIZE(argv1), argv1);
	test = freerdp_static_channel_collection_add(settings, args1);
	if (!test)
		goto fail;
	args1 = NULL; /* settings have taken ownership */

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount);
	if (u32 != 1)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelArraySize);
	if (u32 < 1)
		goto fail;

	cmp2 = args2 = freerdp_addin_argv_new(ARRAYSIZE(argv2), argv2);
	test = freerdp_static_channel_collection_add(settings, args2);
	if (!test)
		goto fail;
	args2 = NULL; /* settings have taken ownership */

	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount);
	if (u32 != 2)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelArraySize);
	if (u32 < 2)
		goto fail;

	/* Test the function return success for known channels */
	got = freerdp_static_channel_collection_find(settings, "foobar");
	if (!compare(got, cmp1))
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "gaga");
	if (!compare(got, cmp2))
		goto fail;
	test = freerdp_static_channel_collection_del(settings, "foobar");
	if (!test)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount);
	if (u32 != 1)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelArraySize);
	if (u32 < 1)
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "foobar");
	if (compare(got, cmp1))
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "gaga");
	if (!compare(got, cmp2))
		goto fail;
	test = freerdp_static_channel_collection_del(settings, "gaga");
	if (!test)
		goto fail;
	u32 = freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount);
	if (u32 != 0)
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "foobar");
	if (compare(got, cmp1))
		goto fail;
	got = freerdp_static_channel_collection_find(settings, "gaga");
	if (compare(got, cmp2))
		goto fail;

	rc = TRUE;

fail:
	freerdp_settings_free(settings);
	freerdp_addin_argv_free(args1);
	freerdp_addin_argv_free(args2);
	return log_result(rc, __func__);
}

static BOOL test_copy(void)
{
	BOOL rc = FALSE;
	wLog* log = WLog_Get(__func__);
	rdpSettings* settings = freerdp_settings_new(0);
	rdpSettings* copy = freerdp_settings_clone(settings);
	rdpSettings* modified = freerdp_settings_clone(settings);

	if (!settings || !copy || !modified)
		goto fail;
	if (!freerdp_settings_set_string(modified, FreeRDP_ServerHostname, "somerandomname"))
		goto fail;
	if (freerdp_settings_print_diff(log, WLOG_WARN, settings, copy))
		goto fail;
	if (!freerdp_settings_print_diff(log, WLOG_WARN, settings, modified))
		goto fail;

	rc = TRUE;

fail:
	freerdp_settings_free(settings);
	freerdp_settings_free(copy);
	freerdp_settings_free(modified);
	return log_result(rc, __func__);
}

static BOOL test_helpers(void)
{
	BOOL rc = FALSE;
	UINT32 flags = 0;
	rdpSettings* settings = freerdp_settings_new(0);
	if (!settings)
		goto fail;
	if (!freerdp_settings_set_bool(settings, FreeRDP_RemoteFxCodec, TRUE))
		goto fail;
	if (!freerdp_settings_set_bool(settings, FreeRDP_NSCodec, TRUE))
		goto fail;
	flags = freerdp_settings_get_codecs_flags(settings);
	if (flags != FREERDP_CODEC_ALL)
		goto fail;

	if (!freerdp_settings_set_bool(settings, FreeRDP_NSCodec, FALSE))
		goto fail;
	flags = freerdp_settings_get_codecs_flags(settings);
	if (flags != (FREERDP_CODEC_ALL & ~FREERDP_CODEC_NSCODEC))
		goto fail;

	if (!freerdp_settings_set_bool(settings, FreeRDP_RemoteFxCodec, FALSE))
		goto fail;
	flags = freerdp_settings_get_codecs_flags(settings);
	if (flags != (FREERDP_CODEC_ALL & ~(FREERDP_CODEC_NSCODEC | FREERDP_CODEC_REMOTEFX)))
		goto fail;

	if (!freerdp_settings_set_bool(settings, FreeRDP_NSCodec, TRUE))
		goto fail;
	flags = freerdp_settings_get_codecs_flags(settings);
	if (flags != (FREERDP_CODEC_ALL & ~FREERDP_CODEC_REMOTEFX))
		goto fail;

	rc = TRUE;
fail:
	freerdp_settings_free(settings);
	return log_result(rc, __func__);
}

static BOOL format_uint(char* buffer, size_t size, UINT64 value, UINT16 intType, UINT64 max)
{
	const UINT64 lvalue = value > max ? max : value;
	intType = intType % 3;
	switch (intType)
	{
		case 0:
			(void)_snprintf(buffer, size, "%" PRIu64, lvalue);
			return TRUE;
		case 1:
			(void)_snprintf(buffer, size, "0x%" PRIx64, lvalue);
			return TRUE;
		case 2:
			if (max < UINT64_MAX)
				(void)_snprintf(buffer, size, "%" PRIu64, max + 1);
			else
				(void)_snprintf(buffer, size, "too large a number");
			return FALSE;
		default:
			(void)_snprintf(buffer, size, "not a number value");
			return FALSE;
	}
}

static BOOL print_negative(char* buffer, size_t size, INT64 value, INT64 min)
{
	switch (min)
	{
		case INT16_MIN:
			(void)_snprintf(buffer, size, "%" PRId16, (INT16)value);
			return FALSE;
		case INT32_MIN:
			(void)_snprintf(buffer, size, "%" PRId32, (INT32)value);
			return FALSE;
		case INT64_MIN:
			(void)_snprintf(buffer, size, "%" PRId64, value);
			return FALSE;
		default:
			(void)_snprintf(buffer, size, "too small a number");
			return FALSE;
	}
}

static BOOL print_xpositive(char* buffer, size_t size, INT64 value, INT64 max)
{
	if (value < 0)
	{
		(void)_snprintf(buffer, size, "%" PRId64, value);
		return TRUE;
	}

	switch (max)
	{
		case INT16_MAX:
			(void)_snprintf(buffer, size, "%" PRIx16, (INT16)value);
			return FALSE;
		case INT32_MAX:
			(void)_snprintf(buffer, size, "%" PRIx32, (INT32)value);
			return FALSE;
		case INT64_MAX:
			(void)_snprintf(buffer, size, "%" PRIx64, value);
			return FALSE;
		default:
			(void)_snprintf(buffer, size, "too small a number");
			return FALSE;
	}
}

static BOOL format_int(char* buffer, size_t size, INT64 value, UINT16 intType, INT64 max, INT64 min)
{
	const INT64 lvalue = (value > max) ? max : ((value < min) ? min : value);
	intType = intType % 4;

	switch (intType)
	{
		case 0:
			(void)_snprintf(buffer, size, "%" PRId64, lvalue);
			return TRUE;
		case 1:
			print_xpositive(buffer, size, lvalue, max);
			return TRUE;
		case 2:
			if (max < INT64_MAX)
				(void)_snprintf(buffer, size, "%" PRId64, max + 1);
			else
				(void)_snprintf(buffer, size, "too large a number");
			return FALSE;
		case 3:
			if (min < INT64_MIN)
				print_negative(buffer, size, min - 1, INT64_MIN);
			else
				(void)_snprintf(buffer, size, "too small a number");
			return FALSE;
		default:
			(void)_snprintf(buffer, size, "not a number value");
			return FALSE;
	}
}

static BOOL format_bool(char* buffer, size_t size, UINT16 intType)
{
	intType = intType % 10;
	switch (intType)
	{
		case 0:
			(void)_snprintf(buffer, size, "FALSE");
			return TRUE;
		case 1:
			(void)_snprintf(buffer, size, "FaLsE");
			return TRUE;
		case 2:
			(void)_snprintf(buffer, size, "False");
			return TRUE;
		case 3:
			(void)_snprintf(buffer, size, "false");
			return TRUE;
		case 4:
			(void)_snprintf(buffer, size, "falseentry");
			return FALSE;
		case 5:
			(void)_snprintf(buffer, size, "TRUE");
			return TRUE;
		case 6:
			(void)_snprintf(buffer, size, "TrUe");
			return TRUE;
		case 7:
			(void)_snprintf(buffer, size, "True");
			return TRUE;
		case 8:
			(void)_snprintf(buffer, size, "true");
			return TRUE;
		case 9:
			(void)_snprintf(buffer, size, "someentry");
			return FALSE;
		default:
			(void)_snprintf(buffer, size, "ok");
			return FALSE;
	}
}

static BOOL check_key_helpers(size_t key, const char* stype)
{
	int test_rounds = 100;
	BOOL res = FALSE;
	rdpSettings* settings = NULL;
	SSIZE_T rc = 0;
	SSIZE_T tkey = 0;
	SSIZE_T type = 0;
	const size_t clear_keys[] = { FreeRDP_RdpServerCertificate,
		                          FreeRDP_RdpServerRsaKey,
		                          FreeRDP_RedirectionPassword,
		                          FreeRDP_RedirectionTsvUrl,
		                          FreeRDP_LoadBalanceInfo,
		                          FreeRDP_ServerRandom,
		                          FreeRDP_ClientRandom,
		                          FreeRDP_ServerCertificate,
		                          FreeRDP_TargetNetAddresses,
		                          FreeRDP_ReceivedCapabilities,
		                          FreeRDP_ServerLicenseProductIssuers,
		                          FreeRDP_TargetNetPorts,
		                          FreeRDP_DeviceArray,
		                          FreeRDP_ChannelDefArray,
		                          FreeRDP_MonitorDefArray,
		                          FreeRDP_ClientAutoReconnectCookie,
		                          FreeRDP_ServerAutoReconnectCookie,
		                          FreeRDP_ClientTimeZone,
		                          FreeRDP_BitmapCacheV2CellInfo,
		                          FreeRDP_GlyphCache,
		                          FreeRDP_FragCache,
		                          FreeRDP_StaticChannelArray,
		                          FreeRDP_DynamicChannelArray,
		                          FreeRDP_ReceivedCapabilities,
		                          FreeRDP_OrderSupport,
		                          FreeRDP_MonitorIds };
	const char* name = freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
	if (!name)
	{
		printf("[%s] missing name for key %" PRIuz "\n", stype, key);
		return FALSE;
	}
	tkey = freerdp_settings_get_key_for_name(name);
	if (tkey < 0)
	{
		printf("[%s] missing reverse name for key %s [%" PRIuz "]\n", stype, name, key);
		return FALSE;
	}
	if ((size_t)tkey != key)
	{
		printf("[%s] mismatch reverse name for key %s [%" PRIuz "]: %" PRIdz "\n", stype, name, key,
		       tkey);
		return FALSE;
	}
	type = freerdp_settings_get_type_for_name(name);
	if (type < 0)
	{
		printf("[%s] missing reverse type for key %s [%" PRIuz "]\n", stype, name, key);
		return FALSE;
	}
	rc = freerdp_settings_get_type_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
	if (rc < 0)
	{
		printf("[%s] missing reverse name for key %s [%" PRIuz "]\n", stype, name, key);
		return FALSE;
	}

	if (rc != type)
	{
		printf("[%s] mismatch reverse type for key %s [%" PRIuz "]: %" PRIdz " <--> %" PRIdz "\n",
		       stype, name, key, rc, type);
		return FALSE;
	}

	settings = freerdp_settings_new(0);
	if (!settings)
	{
		printf("[%s] freerdp_settings_new failed\n", stype);
		goto fail;
	}
	for (size_t x = 0; x < ARRAYSIZE(clear_keys); x++)
	{
		const size_t id = clear_keys[x];
		const char* foo = freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, id));
		if (!freerdp_settings_set_pointer_len(settings, id, NULL, 0))
		{
			printf("[%s] freerdp_settings_set_pointer_len(%s, NULL, 0) failed\n", stype, foo);
			goto fail;
		}
	}
	do
	{
		UINT16 intEntryType = 0;
		BOOL expect = 0;
		BOOL have = 0;
		char value[8192] = { 0 };
		union
		{
			UINT64 u64;
			INT64 i64;
			UINT32 u32;
			INT32 i32;
			UINT16 u16;
			INT16 i16;
			void* pv;
		} val;

		winpr_RAND(&intEntryType, sizeof(intEntryType));
		winpr_RAND(&val.u64, sizeof(val.u64));

		switch (type)
		{
			case RDP_SETTINGS_TYPE_BOOL:
				expect = format_bool(value, sizeof(value), intEntryType);
				break;
			case RDP_SETTINGS_TYPE_UINT16:
				expect = format_uint(value, sizeof(value), val.u64, intEntryType, UINT16_MAX);
				break;
			case RDP_SETTINGS_TYPE_INT16:
				expect =
				    format_int(value, sizeof(value), val.i64, intEntryType, INT16_MAX, INT16_MIN);
				break;
			case RDP_SETTINGS_TYPE_UINT32:
				expect = format_uint(value, sizeof(value), val.u64, intEntryType, UINT32_MAX);
				break;
			case RDP_SETTINGS_TYPE_INT32:
				expect =
				    format_int(value, sizeof(value), val.i64, intEntryType, INT32_MAX, INT32_MIN);
				break;
			case RDP_SETTINGS_TYPE_UINT64:
				expect = format_uint(value, sizeof(value), val.u64, intEntryType, UINT64_MAX);
				break;
			case RDP_SETTINGS_TYPE_INT64:
				expect =
				    format_int(value, sizeof(value), val.i64, intEntryType, INT64_MAX, INT64_MIN);
				break;
			case RDP_SETTINGS_TYPE_STRING:
				expect = TRUE;
				(void)_snprintf(value, sizeof(value), "somerandomstring");
				break;
			case RDP_SETTINGS_TYPE_POINTER:
				expect = FALSE;
				break;

			default:
				printf("[%s] invalid type for key %s [%" PRIuz "]: %" PRIdz " <--> %" PRIdz "\n",
				       stype, name, key, rc, type);
				goto fail;
		}

		have = freerdp_settings_set_value_for_name(settings, name, value);
		if (have != expect)
		{
			printf("[%s] have[%s] != expect[%s]\n", stype, have ? "TRUE" : "FALSE",
			       expect ? "TRUE" : "FALSE");
			goto fail;
		}

	} while (test_rounds-- > 0);

	res = TRUE;
fail:
	freerdp_settings_free(settings);
	return log_result(res, __func__);
}

static BOOL check_args(const RDPDR_DEVICE* what, size_t count, const char* args[])
{
	WINPR_ASSERT(what);

	if (count > 0)
	{
		if (strcmp(what->Name, args[0]) != 0)
			return FALSE;
	}

	switch (what->Type)
	{
		case RDPDR_DTYP_PRINT:
		{
			const RDPDR_PRINTER* a = (const RDPDR_PRINTER*)what;
			if (count <= 1)
				return TRUE;
			if (!a->DriverName)
				return FALSE;
			return strcmp(a->DriverName, args[1]) == 0;
		}

		case RDPDR_DTYP_SERIAL:
		{
			const RDPDR_SERIAL* a = (const RDPDR_SERIAL*)what;

			if (count > 1)
			{
				if (!a->Path)
					return FALSE;
				if (strcmp(a->Path, args[1]) != 0)
					return FALSE;
			}

			if (count > 2)
			{
				if (!a->Driver)
					return FALSE;
				if (strcmp(a->Driver, args[2]) != 0)
					return FALSE;
			}

			if (count > 3)
			{
				if (!a->Permissive)
					return FALSE;
				if (strcmp(a->Permissive, args[3]) != 0)
					return FALSE;
			}
			return TRUE;
		}

		case RDPDR_DTYP_PARALLEL:
		{
			const RDPDR_PARALLEL* a = (const RDPDR_PARALLEL*)what;
			if (count <= 1)
				return TRUE;
			if (!a->Path)
				return FALSE;
			return strcmp(a->Path, args[1]) == 0;
		}

		case RDPDR_DTYP_SMARTCARD:
			return TRUE;

		case RDPDR_DTYP_FILESYSTEM:
		{
			const RDPDR_DRIVE* a = (const RDPDR_DRIVE*)what;
			if (count > 1)
			{
				if (!a->Path)
					return FALSE;
				if (strcmp(a->Path, args[1]) != 0)
					return FALSE;
			}
			if (count > 2)
			{
				return a->automount == (args[2] == NULL) ? TRUE : FALSE;
			}
			else
				return !a->automount;
		}

		default:
			return FALSE;
	}
}

static int check_device_type_arg(UINT32 Type, size_t count, const char* args[])
{
	int rc = -3;
	RDPDR_DEVICE* device = freerdp_device_new(Type, count, args);
	RDPDR_DEVICE* clone = freerdp_device_clone(device);

	if (!device)
		goto fail;

	rc++;
	if (!clone)
		goto fail;

	rc++;
	if (!check_args(device, count, args))
		goto fail;

	rc++;
	if (!freerdp_device_equal(clone, device))
		goto fail;
	rc++;

fail:
	freerdp_device_free(device);
	freerdp_device_free(clone);
	return log_result(rc, __func__);
}

static BOOL check_device_type(void)
{
	struct test_entry
	{
		int expect;
		UINT32 type;
		size_t count;
		const char** args;
	};
	const char* args[] = { "somename", "anothername", "3rdname", "4thname" };
	const struct test_entry tests[] = {
		{ 1, RDPDR_DTYP_SERIAL, 0, NULL },
		{ 1, RDPDR_DTYP_SERIAL, 0, args },
		{ 1, RDPDR_DTYP_SERIAL, 1, args },
		{ 1, RDPDR_DTYP_SERIAL, 2, args },
		{ 1, RDPDR_DTYP_SERIAL, 3, args },
		{ 1, RDPDR_DTYP_SERIAL, 4, args },
		{ 1, RDPDR_DTYP_PARALLEL, 0, NULL },
		{ 1, RDPDR_DTYP_PARALLEL, 0, args },
		{ 1, RDPDR_DTYP_PARALLEL, 1, args },
		{ 1, RDPDR_DTYP_PARALLEL, 2, args },
		{ 1, RDPDR_DTYP_PARALLEL, 3, args },
		{ 1, RDPDR_DTYP_PARALLEL, 4, args },
		{ 1, RDPDR_DTYP_PRINT, 0, NULL },
		{ 1, RDPDR_DTYP_PRINT, 0, args },
		{ 1, RDPDR_DTYP_PRINT, 1, args },
		{ 1, RDPDR_DTYP_PRINT, 2, args },
		{ 1, RDPDR_DTYP_PRINT, 3, args },
		{ 1, RDPDR_DTYP_PRINT, 4, args },
		{ 1, RDPDR_DTYP_FILESYSTEM, 0, NULL },
		{ 1, RDPDR_DTYP_FILESYSTEM, 0, args },
		{ 1, RDPDR_DTYP_FILESYSTEM, 1, args },
		{ 1, RDPDR_DTYP_FILESYSTEM, 2, args },
		{ 1, RDPDR_DTYP_FILESYSTEM, 3, args },
		{ 1, RDPDR_DTYP_FILESYSTEM, 4, args },
		{ 1, RDPDR_DTYP_SMARTCARD, 0, NULL },
		{ 1, RDPDR_DTYP_SMARTCARD, 0, args },
		{ 1, RDPDR_DTYP_SMARTCARD, 1, args },
		{ 1, RDPDR_DTYP_SMARTCARD, 2, args },
		{ 1, RDPDR_DTYP_SMARTCARD, 3, args },
		{ 1, RDPDR_DTYP_SMARTCARD, 4, args },
		{ -3, 0x123, 0, NULL },
		{ -3, 0x123, 0, args },
		{ -3, 0x123, 1, args },
		{ -3, 0x123, 2, args },
		{ -3, 0x123, 3, args },
		{ -3, 0x123, 4, args },
	};
	BOOL rc = TRUE;
	for (size_t x = 0; x < ARRAYSIZE(tests); x++)
	{
		const struct test_entry* cur = &tests[x];
		int got = check_device_type_arg(cur->type, cur->count, cur->args);
		if (got != cur->expect)
			rc = FALSE;
	}
	return log_result(rc, __func__);
}

static BOOL check_offsets(rdpSettings* settings, size_t id, size_t min, size_t max, BOOL checkPtr)
{
	BOOL rc = TRUE;

	WINPR_ASSERT(settings);

	if (!freerdp_settings_get_pointer(settings, id))
		return FALSE;

	for (size_t x = min; x < max; x++)
	{
		const void* ptr = freerdp_settings_get_pointer_array(settings, id, x);
		if (!ptr && checkPtr)
			rc = FALSE;
	}
	return log_result(rc, __func__);
}

static BOOL test_write_offsets(rdpSettings* settings, size_t id, size_t elementSize, size_t min,
                               size_t max)
{
	WINPR_ASSERT(settings);

	for (size_t x = min; x < max; x++)
	{
		const void* ptr = NULL;
		char buffer[8192] = { 0 };

		winpr_RAND(buffer, sizeof(buffer));
		if (!freerdp_settings_set_pointer_array(settings, id, x, buffer))
			return FALSE;
		ptr = freerdp_settings_get_pointer_array(settings, id, x);
		if (!ptr)
			return FALSE;
		if (memcmp(ptr, buffer, elementSize) != 0)
			return FALSE;
	}
	return TRUE;
}

static BOOL test_pointer_array(void)
{
	struct pointer_test_case
	{
		BOOL checkPtr;
		BOOL write;
		size_t id;
		SSIZE_T sizeId;
		size_t size;
		size_t elementSize;
	};
	const struct pointer_test_case tests[] = {
		{ FALSE, FALSE, FreeRDP_DeviceArray, FreeRDP_DeviceArraySize, 32, sizeof(RDPDR_DEVICE*) },
		{ FALSE, FALSE, FreeRDP_StaticChannelArray, FreeRDP_StaticChannelArraySize, 32,
		  sizeof(ADDIN_ARGV*) },
		{ FALSE, FALSE, FreeRDP_DynamicChannelArray, FreeRDP_DynamicChannelArraySize, 33,
		  sizeof(ADDIN_ARGV*) },
		{ TRUE, TRUE, FreeRDP_BitmapCacheV2CellInfo, FreeRDP_BitmapCacheV2NumCells, 5,
		  sizeof(BITMAP_CACHE_V2_CELL_INFO) },
		{ FALSE, FALSE, FreeRDP_OrderSupport, -1, 32, sizeof(BYTE) },
		{ FALSE, FALSE, FreeRDP_ReceivedCapabilities, -1, 32, sizeof(BYTE) },
		{ TRUE, TRUE, FreeRDP_GlyphCache, -1, 10, sizeof(GLYPH_CACHE_DEFINITION) },
		{ TRUE, TRUE, FreeRDP_FragCache, -1, 1, sizeof(GLYPH_CACHE_DEFINITION) },
		{ TRUE, TRUE, FreeRDP_MonitorIds, FreeRDP_NumMonitorIds, 33, sizeof(UINT32) },
		{ TRUE, TRUE, FreeRDP_ChannelDefArray, FreeRDP_ChannelDefArraySize, 42,
		  sizeof(CHANNEL_DEF) },
		{ TRUE, TRUE, FreeRDP_MonitorDefArray, FreeRDP_MonitorDefArraySize, 33,
		  sizeof(rdpMonitor) },
		{ TRUE, TRUE, FreeRDP_ClientTimeZone, -1, 1, sizeof(TIME_ZONE_INFORMATION) },
		{ FALSE, FALSE, FreeRDP_RdpServerCertificate, -1, 1, sizeof(rdpCertificate*) },
		//{ FALSE, FALSE, FreeRDP_RdpServerRsaKey, -1, 1, sizeof(rdpPrivateKey*) },
		{ TRUE, TRUE, FreeRDP_RedirectionPassword, FreeRDP_RedirectionPasswordLength, 42,
		  sizeof(char) },
		{ TRUE, TRUE, FreeRDP_RedirectionTsvUrl, FreeRDP_RedirectionTsvUrlLength, 42,
		  sizeof(char) },
		{ TRUE, TRUE, FreeRDP_LoadBalanceInfo, FreeRDP_LoadBalanceInfoLength, 42, sizeof(char) },
		{ TRUE, TRUE, FreeRDP_ServerRandom, FreeRDP_ServerRandomLength, 42, sizeof(char) },
		{ TRUE, TRUE, FreeRDP_ClientRandom, FreeRDP_ClientRandomLength, 42, sizeof(char) },
		{ TRUE, TRUE, FreeRDP_ServerCertificate, FreeRDP_ServerCertificateLength, 42,
		  sizeof(char) },
		{ TRUE, TRUE, FreeRDP_ClientAutoReconnectCookie, -1, 1, sizeof(ARC_CS_PRIVATE_PACKET) },
		{ TRUE, TRUE, FreeRDP_ServerAutoReconnectCookie, -1, 1, sizeof(ARC_SC_PRIVATE_PACKET) }
	};
	BOOL rc = FALSE;
	rdpSettings* settings = freerdp_settings_new(0);
	if (!settings)
		goto fail;

	for (size_t x = 0; x < ARRAYSIZE(tests); x++)
	{
		const struct pointer_test_case* cur = &tests[x];
		if (!freerdp_settings_set_pointer_len(settings, cur->id, NULL, cur->size))
			goto fail;
		if (cur->sizeId >= 0)
		{
			const UINT32 s = freerdp_settings_get_uint32(settings, (size_t)cur->sizeId);
			if (s != cur->size)
				goto fail;
		}
		if (!check_offsets(settings, cur->id, 0, cur->size, cur->checkPtr))
			goto fail;
		if (check_offsets(settings, cur->id, cur->size, cur->size + 5, TRUE))
			goto fail;
		if (cur->write)
		{
			if (!test_write_offsets(settings, cur->id, cur->elementSize, 0, cur->size))
				goto fail;
			if (test_write_offsets(settings, cur->id, cur->elementSize, cur->size, cur->size + 5))
				goto fail;
		}
		if (!freerdp_settings_set_pointer_len(settings, cur->id, NULL, 0))
			goto fail;
		if (cur->sizeId >= 0)
		{
			const UINT32 s = freerdp_settings_get_uint32(settings, (size_t)cur->sizeId);
			if (s != 0)
				goto fail;
		}
		if (check_offsets(settings, cur->id, 0, cur->size, cur->checkPtr))
			goto fail;
		if (cur->write)
		{
			if (test_write_offsets(settings, cur->id, cur->elementSize, 0, cur->size))
				goto fail;
		}
		if (!freerdp_settings_set_pointer_len(settings, cur->id, NULL, cur->size))
			goto fail;
		if (cur->sizeId >= 0)
		{
			const UINT32 s = freerdp_settings_get_uint32(settings, (size_t)cur->sizeId);
			if (s != cur->size)
				goto fail;
		}
		if (!check_offsets(settings, cur->id, 0, cur->size, cur->checkPtr))
			goto fail;
		if (check_offsets(settings, cur->id, cur->size + 1, cur->size + 5, TRUE))
			goto fail;
		if (cur->write)
		{
			if (!test_write_offsets(settings, cur->id, cur->elementSize, 0, cur->size))
				goto fail;
			if (test_write_offsets(settings, cur->id, cur->elementSize, cur->size, cur->size + 5))
				goto fail;
		}
	}

	rc = TRUE;

fail:
	freerdp_settings_free(settings);
	return log_result(rc, __func__);
}

struct validity_test_case
{
	BOOL expected;
	size_t count;
	const rdpMonitor* monitors;
};

static BOOL prepare_monitor_array(rdpSettings* settings, const struct validity_test_case* testcase)
{
	WINPR_ASSERT(settings);
	WINPR_ASSERT(testcase);

	return freerdp_settings_set_monitor_def_array_sorted(settings, testcase->monitors,
	                                                     testcase->count);
}

static BOOL check_primary_offset(const rdpSettings* settings, const rdpMonitor* monitors,
                                 size_t count)
{
	const rdpMonitor* cprimary = NULL;
	for (size_t x = 0; x < count; x++)
	{
		const rdpMonitor* cur = &monitors[x];
		if (cur->is_primary)
			cprimary = cur;
	}
	if (!cprimary)
	{
		for (size_t x = 0; x < count; x++)
		{
			const rdpMonitor* cur = &monitors[x];
			if ((cur->x == 0) && (cur->y == 0))
				cprimary = cur;
		}
	}
	const rdpMonitor* sprimary = NULL;
	for (size_t x = 0; x < count; x++)
	{
		const rdpMonitor* cur =
		    freerdp_settings_get_pointer_array(settings, FreeRDP_MonitorDefArray, x);
		if (!cur)
			return FALSE;
		if (cur->is_primary)
			sprimary = cur;
	}

	if (!sprimary || !cprimary)
		return FALSE;

	const INT32 xoff = cprimary->x;
	const INT32 yoff = cprimary->y;
	const INT32 sxoff = freerdp_settings_get_int32(settings, FreeRDP_MonitorLocalShiftX);
	const INT32 syoff = freerdp_settings_get_int32(settings, FreeRDP_MonitorLocalShiftY);
	if (xoff != sxoff)
		return FALSE;
	if (yoff != syoff)
		return FALSE;
	return TRUE;
}

static BOOL test_validity_check(void)
{
	BOOL rc = FALSE;
	rdpSettings* settings = freerdp_settings_new(0);
	if (!settings)
		goto fail;

	const rdpMonitor single_monitor_valid[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor single_monitor_invalid_1[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 192,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor single_monitor_invalid_2[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 192,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor single_monitor_invalid_3[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 192,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor single_monitor_invalid_4[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 192,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor multi_monitor_valid[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 0,
		  .y = 1080,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = -1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 0,
		  .y = -1080,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 3840,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 5760,
		  .y = -1079,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 7680,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 7680,
		  .y = 1080,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 7680,
		  .y = -1080,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 7680,
		  .y = -2160,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 9599,
		  .y = -3240,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
	};
	const rdpMonitor multi_monitor_invalid_1[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor multi_monitor_invalid_2[] = {
		{ .x = 1,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor multi_monitor_invalid_3[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1921,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};
	const rdpMonitor multi_monitor_invalid_4[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1919,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};

	const rdpMonitor multi_monitor_valid_2[] = {
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 3840,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};

	const rdpMonitor multi_monitor_valid_3[] = {
		{ .x = 1920,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = TRUE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 3840,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } },
		{ .x = 0,
		  .y = 0,
		  .width = 1920,
		  .height = 1080,
		  .is_primary = FALSE,
		  .orig_screen = 0,
		  .attributes = { .physicalWidth = 100,
		                  .physicalHeight = 100,
		                  .orientation = ORIENTATION_PREFERENCE_LANDSCAPE,
		                  .desktopScaleFactor = 100,
		                  .deviceScaleFactor = 100 } }
	};

	const struct validity_test_case tests[] = {
		{ TRUE, ARRAYSIZE(single_monitor_valid), single_monitor_valid },
		{ FALSE, ARRAYSIZE(single_monitor_invalid_1), single_monitor_invalid_1 },
		{ FALSE, ARRAYSIZE(single_monitor_invalid_2), single_monitor_invalid_2 },
		{ FALSE, ARRAYSIZE(single_monitor_invalid_3), single_monitor_invalid_3 },
		{ FALSE, ARRAYSIZE(single_monitor_invalid_4), single_monitor_invalid_4 },
		{ TRUE, ARRAYSIZE(multi_monitor_valid), multi_monitor_valid },
		{ TRUE, ARRAYSIZE(multi_monitor_invalid_1), multi_monitor_invalid_1 },
		{ FALSE, ARRAYSIZE(multi_monitor_invalid_2), multi_monitor_invalid_2 },
		{ FALSE, ARRAYSIZE(multi_monitor_invalid_3), multi_monitor_invalid_3 },
		{ FALSE, ARRAYSIZE(multi_monitor_invalid_4), multi_monitor_invalid_4 },
		{ TRUE, ARRAYSIZE(multi_monitor_valid_2), multi_monitor_valid_2 },
		{ TRUE, ARRAYSIZE(multi_monitor_valid_3), multi_monitor_valid_3 }
	};

	rc = TRUE;
	for (size_t x = 0; x < ARRAYSIZE(tests); x++)
	{
		const struct validity_test_case* cur = &tests[x];

		if (!prepare_monitor_array(settings, cur))
			rc = log_result_case(FALSE, __func__, x);
		else
		{
#if defined(BUILD_TESTING_INTERNAL)
			const BOOL res = freerdp_settings_check_client_after_preconnect(settings);
#else
			const BOOL res = cur->expected;
#endif

			if ((res != cur->expected) ||
			    !check_primary_offset(settings, cur->monitors, cur->count))
			{
				rc = log_result_case(FALSE, __func__, x);
			}
		}
	}

fail:
	freerdp_settings_free(settings);
	return log_result(rc, __func__);
}

static BOOL test_string_null(rdpSettings* settings, FreeRDP_Settings_Keys_String id)
{
	if (!freerdp_settings_set_string(settings, id, NULL))
		return FALSE;

	const char* chk = freerdp_settings_get_string(settings, id);
	return (chk == NULL);
}

static BOOL test_string_check(rdpSettings* settings, FreeRDP_Settings_Keys_String id,
                              const char* string, size_t len)
{
	const char* chk = freerdp_settings_get_string(settings, id);
	if (!chk)
		return FALSE;

	const size_t clen = strnlen(chk, len + 1);

	/* set strings must always be '\0' terminated */
	if (clen != len)
		return FALSE;

	/* Strings must match comparison */
	if (strncmp(string, chk, clen) != 0)
		return FALSE;

	return TRUE;
}

static BOOL test_string_check_reset(rdpSettings* settings, FreeRDP_Settings_Keys_String id,
                                    const char* string, size_t len)
{
	return test_string_check(settings, id, string, len) && test_string_null(settings, id);
}

static BOOL test_string_set_readback(rdpSettings* settings, FreeRDP_Settings_Keys_String id,
                                     const char* string, size_t len)
{
	WINPR_ASSERT(len > 3);

	BOOL rc = FALSE;
	WCHAR* wstr = NULL;

	const size_t slen = strnlen(string, len);
	if (!freerdp_settings_set_string_len(settings, id, string, slen - 1))
		goto fail;

	if (!test_string_check_reset(settings, id, string, slen - 1))
		goto fail;

	if (!freerdp_settings_set_string(settings, id, string))
		goto fail;

	size_t wlen = 0;
	wstr = freerdp_settings_get_string_as_utf16(settings, id, &wlen);
	if (!wstr || (wlen != slen))
		goto fail;

	if (!test_string_check_reset(settings, id, string, slen))
		goto fail;

	if (!freerdp_settings_set_string_from_utf16N(settings, id, wstr, slen - 1))
		goto fail;

	if (!test_string_check(settings, id, string, slen - 1))
		goto fail;

	if (!freerdp_settings_set_string_from_utf16(settings, id, wstr))
		goto fail;

	if (!test_string_check(settings, id, string, slen))
		goto fail;

	rc = TRUE;
fail:
	free(wstr);
	return rc;
}

static BOOL test_string_len(rdpSettings* settings)
{
	BOOL rc = FALSE;

	const char user[] = "abcdefg";
	if (!test_string_set_readback(settings, FreeRDP_Username, user, sizeof(user)))
		goto fail;

	const char pwd[] = "xyz";
	if (!test_string_set_readback(settings, FreeRDP_Password, pwd, sizeof(pwd)))
		goto fail;

	const char domain[] = "foobar";
	if (!test_string_set_readback(settings, FreeRDP_Domain, domain, sizeof(domain)))
		goto fail;

	rc = TRUE;
fail:
	return rc;
}

int TestSettings(int argc, char* argv[])
{
	int rc = -1;
	rdpSettings* settings = NULL;
	rdpSettings* cloned = NULL;
	rdpSettings* cloned2 = NULL;
	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);

	if (!test_dyn_channels())
		goto fail;
	if (!test_static_channels())
		goto fail;
	if (!test_copy())
		goto fail;
	if (!test_helpers())
		goto fail;
	if (!check_device_type())
		goto fail;
	if (!test_pointer_array())
		goto fail;
	if (!test_validity_check())
		goto fail;

	settings = freerdp_settings_new(0);

	if (!settings)
	{
		printf("Couldn't create settings\n");
		return -1;
	}

	if (!test_string_len(settings))
		goto fail;

	cloned = freerdp_settings_clone(settings);

	if (!cloned)
		goto fail;

#if defined(have_bool_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(bool_list_indices); x++)
	{
		const size_t key = bool_list_indices[x];
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const BOOL val = freerdp_settings_get_bool(settings, key);
		const BOOL cval = freerdp_settings_get_bool(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %u -> copy %u\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_bool(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "bool"))
			goto fail;
	}

#endif
#if defined(have_int16_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(int16_list_indices); x++)
	{
		const size_t key = int16_list_indices[x];
		const char* name = freerdp_settings_get_name_for_key(key);
		const INT16 val = freerdp_settings_get_int16(settings, key);
		const INT16 cval = freerdp_settings_get_int16(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRId16 " -> copy %" PRId16 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_int16(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "int16"))
			goto fail;
	}

#endif
#if defined(have_uint16_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(uint16_list_indices); x++)
	{
		const size_t key = uint16_list_indices[x];
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const UINT16 val = freerdp_settings_get_uint16(settings, key);
		const UINT16 cval = freerdp_settings_get_uint16(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRIu16 " -> copy %" PRIu16 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_uint16(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "uint16"))
			goto fail;
	}

#endif
#if defined(have_uint32_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(uint32_list_indices); x++)
	{
		const size_t key = uint32_list_indices[x];
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const UINT32 val = freerdp_settings_get_uint32(settings, key);
		const UINT32 cval = freerdp_settings_get_uint32(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRIu32 " -> copy %" PRIu32 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_uint32(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "uint32"))
			goto fail;
	}

#endif
#if defined(have_int32_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(int32_list_indices); x++)
	{
		const size_t key = int32_list_indices[x];
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const INT32 val = freerdp_settings_get_int32(settings, key);
		const INT32 cval = freerdp_settings_get_int32(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRId32 " -> copy %" PRId32 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_int32(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "int32"))
			goto fail;
	}

#endif
#if defined(have_uint64_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(uint64_list_indices); x++)
	{
		const size_t key = uint64_list_indices[x];
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const UINT64 val = freerdp_settings_get_uint64(settings, key);
		const UINT64 cval = freerdp_settings_get_uint64(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRIu64 " -> copy %" PRIu64 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_uint64(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "uint64"))
			goto fail;
	}

#endif
#if defined(have_int64_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(int64_list_indices); x++)
	{
		const size_t key = int64_list_indices[x];
		const char* name = freerdp_settings_get_name_for_key(key);
		const INT64 val = freerdp_settings_get_int64(settings, key);
		const INT64 cval = freerdp_settings_get_int64(cloned, key);
		if (val != cval)
		{
			printf("mismatch for key %s: %" PRId64 " -> copy %" PRId64 "\n", name, val, cval);
			goto fail;
		}
		if (!freerdp_settings_set_int64(settings, key, val))
			goto fail;
		if (!check_key_helpers(key, "int64"))
			goto fail;
	}

#endif
#if defined(have_string_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(string_list_indices); x++)
	{
		const size_t key = string_list_indices[x];
		const char val[] = "test-string";
		const char* res = NULL;
		const char* name =
		    freerdp_settings_get_name_for_key(WINPR_ASSERTING_INT_CAST(SSIZE_T, key));
		const char* oval = freerdp_settings_get_string(settings, key);
		const char* cval = freerdp_settings_get_string(cloned, key);
		if ((oval != cval) && (strcmp(oval, cval) != 0))
		{
			printf("mismatch for key %s: %s -> copy %s\n", name, oval, cval);
			goto fail;
		}
		if (!freerdp_settings_set_string(settings, key, val))
			goto fail;

		res = freerdp_settings_get_string(settings, key);

		if (strncmp(val, res, sizeof(val)) != 0)
			goto fail;
	}

#endif
#if defined(have_pointer_list_indices)

	for (size_t x = 0; x < ARRAYSIZE(pointer_list_indices); x++)
	{
		const size_t key = pointer_list_indices[x];
		const void* val = freerdp_settings_get_pointer(settings, key);
		WINPR_UNUSED(val);
	}

#endif
	cloned2 = freerdp_settings_clone(settings);
	if (!cloned2)
		goto fail;
	if (!freerdp_settings_copy(cloned2, cloned))
		goto fail;

	rc = 0;
fail:
	freerdp_settings_free(cloned);
	freerdp_settings_free(cloned2);
	freerdp_settings_free(settings);
	return rc;
}
