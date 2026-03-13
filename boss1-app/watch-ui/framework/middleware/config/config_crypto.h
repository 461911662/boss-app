/****************************************************************************
 * watch-ui/framework/middleware/config/config_crypto.h
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

#ifndef __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CRYPTO_H
#define __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CRYPTO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
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
                   size_t len);

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
                   size_t len);

/****************************************************************************
 * Name: config_encrypted_len
 *
 * Description:
 *   Calculate required buffer size for encrypted data.
 *
 * Input Parameters:
 *   json_len - Length of plaintext JSON string
 *
 * Returned Value:
 *   Required buffer size.
 *
 ****************************************************************************/

size_t config_encrypted_len(size_t json_len);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CRYPTO_H */
