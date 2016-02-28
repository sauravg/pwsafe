/*
* Copyright (c) 2013-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// KeyWrapTest.cpp: Unit test for KeyWrap implementation

#include <cstring>
#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/KeyWrap.h"
#include "core/AES.h"
#include "gtest/gtest.h"

static int AES_wrap_unwrap_test(const unsigned char *kek, int keybits,
                                const unsigned char *eout,
                                const unsigned char *key, int keylen);

TEST(KeyWrapTest, AES_keywrap)
{
  static const unsigned char kek[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };

  static const unsigned char key[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  };

  static const unsigned char e1[] = {
    0x1f, 0xa6, 0x8b, 0x0a, 0x81, 0x12, 0xb4, 0x47,
    0xae, 0xf3, 0x4b, 0xd8, 0xfb, 0x5a, 0x7b, 0x82,
    0x9d, 0x3e, 0x86, 0x23, 0x71, 0xd2, 0xcf, 0xe5
  };

  static const unsigned char e2[] = {
    0x96, 0x77, 0x8b, 0x25, 0xae, 0x6c, 0xa4, 0x35,
    0xf9, 0x2b, 0x5b, 0x97, 0xc0, 0x50, 0xae, 0xd2,
    0x46, 0x8a, 0xb8, 0xa1, 0x7a, 0xd8, 0x4e, 0x5d
  };

  static const unsigned char e3[] = {
    0x64, 0xe8, 0xc3, 0xf9, 0xce, 0x0f, 0x5b, 0xa2,
    0x63, 0xe9, 0x77, 0x79, 0x05, 0x81, 0x8a, 0x2a,
    0x93, 0xc8, 0x19, 0x1e, 0x7d, 0x6e, 0x8a, 0xe7
  };

  static const unsigned char e4[] = {
    0x03, 0x1d, 0x33, 0x26, 0x4e, 0x15, 0xd3, 0x32,
    0x68, 0xf2, 0x4e, 0xc2, 0x60, 0x74, 0x3e, 0xdc,
    0xe1, 0xc6, 0xc7, 0xdd, 0xee, 0x72, 0x5a, 0x93,
    0x6b, 0xa8, 0x14, 0x91, 0x5c, 0x67, 0x62, 0xd2
  };

  static const unsigned char e5[] = {
    0xa8, 0xf9, 0xbc, 0x16, 0x12, 0xc6, 0x8b, 0x3f,
    0xf6, 0xe6, 0xf4, 0xfb, 0xe3, 0x0e, 0x71, 0xe4,
    0x76, 0x9c, 0x8b, 0x80, 0xa3, 0x2c, 0xb8, 0x95,
    0x8c, 0xd5, 0xd1, 0x7d, 0x6b, 0x25, 0x4d, 0xa1
  };

  static const unsigned char e6[] = {
    0x28, 0xc9, 0xf4, 0x04, 0xc4, 0xb8, 0x10, 0xf4,
    0xcb, 0xcc, 0xb3, 0x5c, 0xfb, 0x87, 0xf8, 0x26,
    0x3f, 0x57, 0x86, 0xe2, 0xd8, 0x0e, 0xd3, 0x26,
    0xcb, 0xc7, 0xf0, 0xe7, 0x1a, 0x99, 0xf4, 0x3b,
    0xfb, 0x98, 0x8b, 0x9b, 0x7a, 0x02, 0xdd, 0x21
  };
  
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 128, e1, key, 16) != 0) << "(kek, 128, e1, key, 16)";
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 192, e2, key, 16) != 0) << "(kek, 192, e2, key, 16)";
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 256, e3, key, 16) != 0) << "(kek, 256, e3, key, 16)";
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 192, e4, key, 24) != 0) << "(kek, 192, e4, key, 24)";
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 256, e5, key, 24) != 0) << "(kek, 256, e5, key, 24)";
  EXPECT_TRUE(AES_wrap_unwrap_test(kek, 256, e6, key, 32) != 0) << "(kek, 256, e6, key, 32)";
}

static int AES_wrap_unwrap_test(const unsigned char *kek, int keybits,
                                const unsigned char *eout,
                                const unsigned char *key, int keylen)
{
  unsigned char *otmp = NULL, *ptmp = NULL;
  int ret = 0;
  otmp = new unsigned char[keylen + 8];
  ptmp = new unsigned char[keylen];
  if (!otmp || !ptmp)
    return 0;
  AES wctx(kek, keybits/8);
  KeyWrap kw(&wctx);
  kw.Wrap(key, otmp, keylen);

  if (eout && std::memcmp(eout, otmp, keylen))
    goto err;
    
  if (!kw.Unwrap(otmp, ptmp, keylen+8))
    goto err;

  if (std::memcmp(key, ptmp, keylen))
    goto err;

  ret = 1;

 err:
  delete[] otmp;
  delete[] ptmp;
  return ret;
}
