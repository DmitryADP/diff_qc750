/*
 * Copyright (c) 2011 Trusted Logic S.A.
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * Trusted Logic S.A. ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with Trusted Logic S.A.
 *
 * TRUSTED LOGIC S.A. MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. TRUSTED LOGIC S.A. SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
#ifndef __SERVICE_SYSTEM_PROTOCOL_H__
#define __SERVICE_SYSTEM_PROTOCOL_H__

#include "s_type.h"

/* -----------------------------------------------------------------------------
      UUID and Name
----------------------------------------------------------------------------- */

/** Service Identifier */
/* {56304b83-5c4e-4428-b99e-605c96ae58d6} */
#define SERVICE_SYSTEM_UUID   { 0x56304b83, 0x5c4e, 0x4428, { 0xb9, 0x9e, 0x60, 0x5c, 0x96, 0xae, 0x58, 0xd6 } }

/** Generic type for error codes in the system service */
#define SERVICE_SYSTEM_NAME_ASCII  "SYSTEM"


/* -----------------------------------------------------------------------------
                               SST Command identifiers
----------------------------------------------------------------------------- */

#define SERVICE_SYSTEM_SST_OPEN_COMMAND_ID                0x00000001
#define SERVICE_SYSTEM_SST_CLOSE_COMMAND_ID               0x00000002
#define SERVICE_SYSTEM_SST_READ_COMMAND_ID                0x00000003
#define SERVICE_SYSTEM_SST_WRITE_COMMAND_ID               0x00000004
#define SERVICE_SYSTEM_SST_SEEK_COMMAND_ID                0x00000005
#define SERVICE_SYSTEM_SST_GET_OFFSET_AND_SIZE_COMMAND_ID 0x00000007
#define SERVICE_SYSTEM_SST_GET_SIZE_COMMAND_ID            0x00000008
#define SERVICE_SYSTEM_SST_CLOSE_DELETE_COMMAND_ID        0x00000009
#define SERVICE_SYSTEM_SST_TRUNCATE_COMMAND_ID            0x0000000A
#define SERVICE_SYSTEM_SST_ENUM_START_COMMAND_ID          0x0000000B
#define SERVICE_SYSTEM_SST_ENUM_GETNEXT_COMMAND_ID        0x0000000C
#define SERVICE_SYSTEM_SST_ENUM_CLOSE_COMMAND_ID          0x0000000D
#define SERVICE_SYSTEM_SST_RENAME_COMMAND_ID              0x0000000E

/* -----------------------------------------------------------------------------
                      PKCS11/MTC Command identifiers
----------------------------------------------------------------------------- */
#define SERVICE_SYSTEM_PKCS11_C_CREATEOBJECT_COMMAND_ID      0x00000020
#define SERVICE_SYSTEM_PKCS11_C_DESTROYOBJECT_COMMAND_ID     0x00000021
#define SERVICE_SYSTEM_PKCS11_C_GETATTRIBUTEVALUE_COMMAND_ID 0x00000022
#define SERVICE_SYSTEM_PKCS11_C_FINDOBJECTSINIT_COMMAND_ID   0x00000023
#define SERVICE_SYSTEM_PKCS11_C_FINDOBJECTS_COMMAND_ID       0x00000024
#define SERVICE_SYSTEM_PKCS11_C_FINDOBJECTSFINAL_COMMAND_ID  0x00000025
#define SERVICE_SYSTEM_PKCS11_C_DIGESTINIT_COMMAND_ID        0x00000026
#define SERVICE_SYSTEM_PKCS11_C_DIGEST_COMMAND_ID            0x00000027
#define SERVICE_SYSTEM_PKCS11_C_DIGESTUPDATE_COMMAND_ID      0x00000028
#define SERVICE_SYSTEM_PKCS11_C_DIGESTFINAL_COMMAND_ID       0x00000029
#define SERVICE_SYSTEM_PKCS11_C_SIGNINIT_COMMAND_ID          0x0000002A
#define SERVICE_SYSTEM_PKCS11_C_SIGN_COMMAND_ID              0x0000002B
#define SERVICE_SYSTEM_PKCS11_C_SIGNUPDATE_COMMAND_ID        0x0000002C
#define SERVICE_SYSTEM_PKCS11_C_SIGNFINAL_COMMAND_ID         0x0000002D
#define SERVICE_SYSTEM_PKCS11_C_ENCRYPTINIT_COMMAND_ID       0x0000002E
#define SERVICE_SYSTEM_PKCS11_C_ENCRYPT_COMMAND_ID           0x0000002F
#define SERVICE_SYSTEM_PKCS11_C_ENCRYPTUPDATE_COMMAND_ID     0x00000030
#define SERVICE_SYSTEM_PKCS11_C_ENCRYPTFINAL_COMMAND_ID      0x00000031
#define SERVICE_SYSTEM_PKCS11_C_DECRYPTINIT_COMMAND_ID       0x00000032
#define SERVICE_SYSTEM_PKCS11_C_DECRYPT_COMMAND_ID           0x00000033
#define SERVICE_SYSTEM_PKCS11_C_DECRYPTUPDATE_COMMAND_ID     0x00000034
#define SERVICE_SYSTEM_PKCS11_C_DECRYPTFINAL_COMMAND_ID      0x00000035
#define SERVICE_SYSTEM_PKCS11_C_GENERATEKEY_COMMAND_ID       0x00000036
#define SERVICE_SYSTEM_PKCS11_C_GENERATEKEYPAIR_COMMAND_ID   0x00000037
#define SERVICE_SYSTEM_PKCS11_C_SEEDRANDOM_COMMAND_ID        0x00000038
#define SERVICE_SYSTEM_PKCS11_C_GENERATERANDOM_COMMAND_ID    0x00000039
#define SERVICE_SYSTEM_PKCS11_C_VERIFYINIT_COMMAND_ID        0x0000003A
#define SERVICE_SYSTEM_PKCS11_C_VERIFY_COMMAND_ID            0x0000003B
#define SERVICE_SYSTEM_PKCS11_C_VERIFYUPDATE_COMMAND_ID      0x0000003C
#define SERVICE_SYSTEM_PKCS11_C_VERIFYFINAL_COMMAND_ID       0x0000003D
#define SERVICE_SYSTEM_PKCS11_C_DERIVEKEY_COMMAND_ID         0x0000003E
#define SERVICE_SYSTEM_PKCS11_C_ABORTOPERATION_COMMAND_ID    0x0000003F
#define SERVICE_SYSTEM_PKCS11_C_COPYOBJECT_COMMAND_ID        0x00000040
#define SERVICE_SYSTEM_PKCS11_C_CLOSEOBJECTHANDLE_COMMAND_ID 0x00000041
#define SERVICE_SYSTEM_PKCS11_C_OPEN_SESSION_COMMAND_ID      0x00000042
#define SERVICE_SYSTEM_PKCS11_C_CLOSE_SESSION_COMMAND_ID     0x00000043
#define SERVICE_SYSTEM_PKCS11_INCREMENT_MTC_COMMAND_ID       0x00000044
#define SERVICE_SYSTEM_PKCS11_GET_MTC_COMMAND_ID             0x00000045

typedef struct
{
   uint32_t             attributeType;
   uint32_t             dataOffset : 30;
   uint32_t             dataParamIndex  : 2;
   uint32_t             dataValueLen;
} INPUT_TEMPLATE_ITEM;

#endif /* __SERVICE_SYSTEM_PROTOCOL_H__ */
