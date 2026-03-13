/****************************************************************************
 * watch-ui/framework/middleware/config/config_crypto.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nuttx/crypto/tea.h>

#include "config_crypto.h"
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_CONFIG_DEBUG
#  define CRYPTO_LOG(fmt, ...)    appinfo("[Crypto] " fmt, ##__VA_ARGS__)
#  define CRYPTO_DBG(fmt, ...)    appdbg("[Crypto] " fmt, ##__VA_ARGS__)
#  define CRYPTO_WARN(fmt, ...)   appwarn("[Crypto] " fmt, ##__VA_ARGS__)
#  define CRYPTO_ERR(fmt, ...)    apperr("[Crypto] " fmt, ##__VA_ARGS__)
#else
#  define CRYPTO_LOG(fmt, ...)
#  define CRYPTO_DBG(fmt, ...)
#  define CRYPTO_WARN(fmt, ...)   appwarn("[Crypto] " fmt, ##__VA_ARGS__)
#  define CRYPTO_ERR(fmt, ...)    apperr("[Crypto] " fmt, ##__VA_ARGS__)
#endif

#ifndef CONFIG_FSUTILS_PASSWD_KEY1
#  define CONFIG_FSUTILS_PASSWD_KEY1 0x12345678
#endif

#ifndef CONFIG_FSUTILS_PASSWD_KEY2
#  define CONFIG_FSUTILS_PASSWD_KEY2 0x9abcdef0
#endif

#ifndef CONFIG_FSUTILS_PASSWD_KEY3
#  define CONFIG_FSUTILS_PASSWD_KEY3 0x12345678
#endif

#ifndef CONFIG_FSUTILS_PASSWD_KEY4
#  define CONFIG_FSUTILS_PASSWD_KEY4 0x9abcdef0
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint32_t g_tea_key[4] =
{
  CONFIG_FSUTILS_PASSWD_KEY1,
  CONFIG_FSUTILS_PASSWD_KEY2,
  CONFIG_FSUTILS_PASSWD_KEY3,
  CONFIG_FSUTILS_PASSWD_KEY4
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: base64_decode
 *
 * Description:
 *   Decode a base64 character to its 6-bit value.
 *
 * Input Parameters:
 *   c - The base64 character
 *
 * Returned Value:
 *   The 6-bit value, or -1 if invalid.
 *
 ****************************************************************************/

static int base64_decode(char c)
{
  if (c >= 'A' && c <= 'Z')
    {
      return c - 'A';
    }
  if (c >= 'a' && c <= 'z')
    {
      return c - 'a' + 26;
    }
  if (c >= '0' && c <= '9')
    {
      return c - '0' + 52;
    }
  if (c == '+')
    {
      return 62;
    }
  if (c == '/')
    {
      return 63;
    }

  return -1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: config_encrypt
 *
 * Description:
 *   Encrypt a config string using TEA.
 *
 * Input Parameters:
 *   plaintext - The plaintext string to encrypt
 *   encrypted - Buffer to store the encrypted result (base64 encoded)
 *   len       - Size of the encrypted buffer
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_encrypt(FAR const char *plaintext,
                   char *encrypted,
                   size_t len)
{
  union
  {
    char     b[8];
    uint16_t h[4];
    uint32_t l[2];
  } value;

  FAR const char *src;
  FAR char *dest;
  FAR char *bptr;
  uint32_t tmp;
  uint8_t remainder;
  int remaining;
  int gulpsize;
  int nbits;
  int i;

  CRYPTO_DBG("Encrypting config, len=%zu", len);

  if (!plaintext || !encrypted || len == 0)
    {
      CRYPTO_ERR("Encrypt: invalid parameters");
      return -EINVAL;
    }

  remaining = strlen(plaintext);

  src  = plaintext;
  dest = encrypted;
  *dest = '\0';

  remainder  = 0;
  nbits      = 0;

  for (; remaining > 0; remaining -= gulpsize)
    {
      gulpsize = sizeof(value.b);
      if (gulpsize > remaining)
        {
          gulpsize = remaining;
        }

      bptr = value.b;
      for (i = 0; i < gulpsize; i++)
        {
          *bptr++ = *src++;
        }

      for (; i < sizeof(value.b); i++)
        {
          *bptr++ = ' ';
        }

      tea_encrypt(value.l, g_tea_key);

      tmp = remainder;

      for (i = 0; i < 4; i++)
        {
          tmp    = (uint32_t)value.h[i] << nbits | tmp;
          nbits += 16;

          while (nbits >= 6)
            {
              static const char b64chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
              *dest++ = b64chars[tmp & 0x3f];
              tmp   >>= 6;
              nbits  -= 6;
            }
        }

      remainder = (uint8_t)tmp;
      *dest     = '\0';
    }

  if (nbits > 0)
    {
      static const char b64chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      *dest++ = b64chars[remainder];
      *dest   = '\0';
    }

  return OK;
}

/****************************************************************************
 * Name: config_decrypt
 *
 * Description:
 *   Decrypt a config string encrypted with TEA.
 *
 * Input Parameters:
 *   encrypted - The encrypted string (base64 encoded)
 *   plaintext - Buffer to store the decrypted result
 *   len       - Size of the plaintext buffer
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_decrypt(FAR const char *encrypted,
                   char *plaintext,
                   size_t len)
{
  union
  {
    char     b[8];
    uint16_t h[4];
    uint32_t l[2];
  } value;

  FAR const char *src;
  FAR char *dest;
  int remaining;
  int nbits;
  int val;
  int i;

  CRYPTO_DBG("Decrypting config, len=%zu", len);

  if (!encrypted || !plaintext || len < 0)
    {
      CRYPTO_ERR("Decrypt: invalid parameters");
      return -EINVAL;
    }

  src = encrypted;
  dest = plaintext;
  remaining = strlen(encrypted);
  nbits = 0;
  val = 0;

  while (remaining > 0)
    {
      int decoded = base64_decode(*src++);
      if (decoded < 0)
        {
          break;
        }

      remaining--;
      val = (val << 6) | decoded;
      nbits += 6;

      if (nbits >= 8)
        {
          nbits -= 8;
          *dest++ = (val >> nbits) & 0xff;
          val = val & ((1 << nbits) - 1);
        }
    }

  *dest = '\0';

  if (strlen(plaintext) == 0)
    {
      return -EINVAL;
    }

  remaining = strlen(plaintext);
  src = plaintext;
  dest = plaintext;

  for (i = 0; i < remaining && i < 8; i++)
    {
      value.b[i] = *src++;
    }

  for (i = remaining; i < 8; i++)
    {
      value.b[i] = ' ';
    }

  tea_decrypt(value.l, g_tea_key);

  for (i = 0; i < 8; i++)
    {
      if (value.b[i] != ' ')
        {
          *dest++ = value.b[i];
        }
    }

  *dest = '\0';

  return OK;
}

/****************************************************************************
 * Name: config_encrypted_len
 *
 * Description:
 *   Calculate required buffer size for encrypted data.
 *   Uses TEA (8-byte block) + Base64 encoding.
 *
 * Input Parameters:
 *   json_len - Length of plaintext JSON string
 *
 * Returned Value:
 *   Required buffer size.
 *
 ****************************************************************************/

size_t config_encrypted_len(size_t json_len)
{
  size_t tea_len;

  tea_len = (json_len + 7) & ~7;

  return (tea_len / 3 + 1) * 4 + 8;
}
