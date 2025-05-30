
#include <stdio.h>
#include <winpr/crt.h>
#include <winpr/path.h>
#include <winpr/tchar.h>
#include <winpr/winpr.h>

static const TCHAR testBasePathBackslash[] = _T("C:\\Program Files\\");
static const TCHAR testBasePathNoBackslash[] = _T("C:\\Program Files");
static const TCHAR testMorePathBackslash[] = _T("\\Microsoft Visual Studio 11.0");
static const TCHAR testMorePathNoBackslash[] = _T("Microsoft Visual Studio 11.0");
static const TCHAR testPathOut[] = _T("C:\\Program Files\\Microsoft Visual Studio 11.0");

int TestPathCchAppend(int argc, char* argv[])
{
	HRESULT status = 0;
	TCHAR Path[PATHCCH_MAX_CCH];

	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);

	/* Base Path: Backslash, More Path: No Backslash */

	_tcsncpy(Path, testBasePathBackslash, ARRAYSIZE(Path));

	status = PathCchAppend(Path, PATHCCH_MAX_CCH, testMorePathNoBackslash);

	if (status != S_OK)
	{
		_tprintf(_T("PathCchAppend status: 0x%08") _T(PRIX32) _T("\n"), status);
		return -1;
	}

	if (_tcsncmp(Path, testPathOut, ARRAYSIZE(Path)) != 0)
	{
		_tprintf(_T("Path Mismatch: Actual: %s, Expected: %s\n"), Path, testPathOut);
		return -1;
	}

	/* Base Path: Backslash, More Path: Backslash */

	_tcsncpy(Path, testBasePathBackslash, ARRAYSIZE(Path));

	status = PathCchAppend(Path, PATHCCH_MAX_CCH, testMorePathBackslash);

	if (status != S_OK)
	{
		_tprintf(_T("PathCchAppend status: 0x%08") _T(PRIX32) _T("\n"), status);
		return -1;
	}

	if (_tcsncmp(Path, testPathOut, ARRAYSIZE(Path)) != 0)
	{
		_tprintf(_T("Path Mismatch: Actual: %s, Expected: %s\n"), Path, testPathOut);
		return -1;
	}

	/* Base Path: No Backslash, More Path: Backslash */

	_tcsncpy(Path, testBasePathNoBackslash, ARRAYSIZE(Path));

	status = PathCchAppend(Path, PATHCCH_MAX_CCH, testMorePathBackslash);

	if (status != S_OK)
	{
		_tprintf(_T("PathCchAppend status: 0x%08") _T(PRIX32) _T("\n"), status);
		return -1;
	}

	if (_tcsncmp(Path, testPathOut, ARRAYSIZE(Path)) != 0)
	{
		_tprintf(_T("Path Mismatch: Actual: %s, Expected: %s\n"), Path, testPathOut);
		return -1;
	}

	/* Base Path: No Backslash, More Path: No Backslash */

	_tcsncpy(Path, testBasePathNoBackslash, ARRAYSIZE(Path));

	status = PathCchAppend(Path, PATHCCH_MAX_CCH, testMorePathNoBackslash);

	if (status != S_OK)
	{
		_tprintf(_T("PathCchAppend status: 0x%08") _T(PRIX32) _T("\n"), status);
		return -1;
	}

	if (_tcsncmp(Path, testPathOut, ARRAYSIZE(Path)) != 0)
	{
		_tprintf(_T("Path Mismatch: Actual: %s, Expected: %s\n"), Path, testPathOut);
		return -1;
	}

	/* According to msdn a NULL Path is an invalid argument */
	status = PathCchAppend(NULL, PATHCCH_MAX_CCH, testMorePathNoBackslash);
	if (status != E_INVALIDARG)
	{
		_tprintf(_T("PathCchAppend with NULL path unexpectedly returned status: 0x%08") _T(
		             PRIX32) _T("\n"),
		         status);
		return -1;
	}

	/* According to msdn a NULL pszMore is an invalid argument (although optional !?) */
	_tcsncpy(Path, testBasePathNoBackslash, ARRAYSIZE(Path));
	status = PathCchAppend(Path, PATHCCH_MAX_CCH, NULL);
	if (status != E_INVALIDARG)
	{
		_tprintf(_T("PathCchAppend with NULL pszMore unexpectedly returned status: 0x%08") _T(
		             PRIX32) _T("\n"),
		         status);
		return -1;
	}

	/* According to msdn cchPath must be > 0 and <= PATHCCH_MAX_CCH */
	_tcsncpy(Path, testBasePathNoBackslash, ARRAYSIZE(Path));
	status = PathCchAppend(Path, 0, testMorePathNoBackslash);
	if (status != E_INVALIDARG)
	{
		_tprintf(_T("PathCchAppend with cchPath value 0 unexpectedly returned status: 0x%08") _T(
		             PRIX32) _T("\n"),
		         status);
		return -1;
	}
	_tcsncpy(Path, testBasePathNoBackslash, ARRAYSIZE(Path));
	status = PathCchAppend(Path, PATHCCH_MAX_CCH + 1, testMorePathNoBackslash);
	if (status != E_INVALIDARG)
	{
		_tprintf(_T("PathCchAppend with cchPath value > PATHCCH_MAX_CCH unexpectedly returned ")
		         _T("status: 0x%08") _T(PRIX32) _T("\n"),
		         status);
		return -1;
	}

	/* Resulting file must not exceed PATHCCH_MAX_CCH */

	for (size_t i = 0; i < PATHCCH_MAX_CCH - 1; i++)
		Path[i] = _T('X');

	Path[PATHCCH_MAX_CCH - 1] = 0;

	status = PathCchAppend(Path, PATHCCH_MAX_CCH, _T("\\This cannot be appended to Path"));
	if (SUCCEEDED(status))
	{
		_tprintf(_T("PathCchAppend unexpectedly succeeded with status: 0x%08") _T(PRIX32) _T("\n"),
		         status);
		return -1;
	}

	return 0;
}
