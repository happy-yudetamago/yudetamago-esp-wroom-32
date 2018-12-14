// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

enum E_OTA_RESULT {
    OTA_OK,
    OTA_ERR_CONNECTION,
    OTA_ERR_NOT_FOUND,
    OTA_ERR_TIMEOUT,
    OTA_ERR_NO_OCTET_STREAM,
    OTA_ERR_NO_CONTENT,
    OTA_ERR_NO_SPACES,
    OTA_ERR_INVALID_MD5,
    OTA_ERR_WRITE,
    OTA_ERR_OTHER,
};

E_OTA_RESULT execOTA(const char *firmwareUri, const char *md5Uri);
