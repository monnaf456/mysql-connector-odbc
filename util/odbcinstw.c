/*
  Copyright (C) 2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 @file  odbcinstw.c
 @brief Installer API Unicode wrapper functions.

 unixODBC 2.2.11 does not include the Unicode versions of the ODBC
 installer API. As of November 2007, this was still the current version
 shipped with Debian and Ubuntu Linux.

 SQLGetPrivateProfileString() also has a few bugs in unixODBC 2.2.11 that
 our version of SQLGetPrivateProfileStringW() will work around.
*/

#include "stringutil.h"

#include <sql.h>
#include <wchar.h>

/*
   If SQLGetPrivateProfileString() is not defined, it's also an old version
   of unixODBC that lacks a typedef for LPCWSTR.
*/
#if !defined(HAVE_SQLGETPRIVATEPROFILESTRINGW)
typedef const LPWSTR LPCWSTR;
#endif

#define INSTAPI

#ifndef FALSE
# define FALSE 0
#endif


#if !defined(HAVE_SQLGETPRIVATEPROFILESTRINGW) || defined(USE_UNIXODBC)
int INSTAPI
MySQLGetPrivateProfileStringW(LPCWSTR lpszSection, LPCWSTR lpszEntry,
                              LPCWSTR lpszDefault, LPWSTR lpszRetBuffer,
                              int cbRetBuffer, LPCWSTR lpszFilename)
{
  SQLINTEGER len;
  int rc;
  char *section, *entry, *def, *ret, *filename;

  len= SQL_NTS;
  section= sqlwchar_as_utf8(lpszSection, &len);
  len= SQL_NTS;
  entry= sqlwchar_as_utf8(lpszEntry, &len);
  len= SQL_NTS;
  def= sqlwchar_as_utf8(lpszDefault, &len);
  len= SQL_NTS;
  filename= sqlwchar_as_utf8(lpszFilename, &len);

  if (lpszRetBuffer && cbRetBuffer)
    ret= malloc(cbRetBuffer + 1);
  else
    ret= NULL;

  /* unixODBC 2.2.11 can't handle NULL for default, so pass "" instead. */
  rc= SQLGetPrivateProfileString(section, entry, def ? def : "", ret,
                                 cbRetBuffer, filename);

  if (rc > 0 && lpszRetBuffer)
  {
    /*
     unixODBC 2.2.11 returns the wrong value from SQLGetPrivateProfileString
     when getting the list of sections or entries in a section, so we have to
     re-calculate the correct length by walking the list of values.
    */
    if (!entry || !section)
    {
      char *pos= ret;
      while (*pos && pos < ret + cbRetBuffer)
        pos+= strlen(pos) + 1;
      rc= pos - ret;
    }

    /** @todo error handling */
    utf8_as_sqlwchar(lpszRetBuffer, cbRetBuffer, ret, rc);
  }

  x_free(section);
  x_free(entry);
  x_free(def);
  x_free(ret);
  x_free(filename);

  return rc;
}
#endif


#ifndef HAVE_SQLGETPRIVATEPROFILESTRINGW
int INSTAPI
SQLGetPrivateProfileStringW(LPCWSTR lpszSection, LPCWSTR lpszEntry,
                            LPCWSTR lpszDefault, LPWSTR lpszRetBuffer,
                            int cbRetBuffer, LPCWSTR lpszFilename)
{
  return MySQLGetPrivateProfileStringW(lpszSection, lpszEntry, lpszDefault,
                                       lpszRetBuffer, cbRetBuffer,
                                       lpszFilename);
}


/**
 @todo The rest of the replacement functions are not actually implemented.
*/

BOOL INSTAPI
SQLInstallDriverExW(LPCWSTR lpszDriver, LPCWSTR lpszPathIn, LPWSTR lpszPathOut,
                    WORD cbPathOutMax, WORD *pcbPathOut, WORD fRequest,
                    LPDWORD lpdwUsageCount)
{
  return FALSE;
}


RETCODE INSTAPI
SQLPostInstallerErrorW(DWORD fErrorCode, LPWSTR szErrorMsg)
{
  return SQL_ERROR;
}


BOOL INSTAPI
SQLRemoveDSNFromIniW(LPCWSTR lpszDSN)
{
  return FALSE;
}


BOOL INSTAPI
SQLRemoveDriverW(LPCWSTR lpszDriver, BOOL fRemoveDSN, LPDWORD lpdwUsageCount)
{
  return FALSE;
}


BOOL INSTAPI
SQLValidDSNW(LPCWSTR lpszDSN)
{
  return FALSE;
}


BOOL INSTAPI
SQLWriteDSNToIniW(LPCWSTR lpszDSN, LPCWSTR lpszDriver)
{
  return FALSE;
}


BOOL INSTAPI
SQLWritePrivateProfileStringW(LPCWSTR lpszSection, LPCWSTR lpszEntry,
                              LPCWSTR lpszString, LPCWSTR lpszFilename)
{
  return FALSE;
}
#endif
