// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "YudetamagoClient.h"

#include <HTTPClient.h>
#include <base64.h>
#include "sha256.h"
#include "NCMBConfig.h"

static String getGetExistanceSignature(const char *objectId)
{
    Sha256 sha256;
    sha256.initHmac((const uint8_t *)CLIENT_KEY, strlen(CLIENT_KEY));
    sha256.print("GET\n");
    sha256.print("mb.api.cloud.nifty.com\n");
    sha256.print("/2013-09-01/classes/ToggleStocker\n");
    sha256.print("SignatureMethod=HmacSHA256&SignatureVersion=2&X-NCMB-Application-Key=");
    sha256.print(APPLICATION_KEY);
    sha256.print("&X-NCMB-Timestamp=");
    sha256.print(TIMESTAMP);
    // {"objectId":"<your object id>"}
    sha256.print("&where=%7B%22objectId%22%3A%22");
    sha256.print(objectId);
    sha256.print("%22%7D");
    return base64::encode(sha256.resultHmac(), HASH_LENGTH);
}

static String getSetExistanceSignature(const char *objectId)
{
    Sha256 sha256;
    sha256.initHmac((const uint8_t *)CLIENT_KEY, strlen(CLIENT_KEY));
    sha256.print("PUT\n");

    sha256.print("mb.api.cloud.nifty.com\n");

    sha256.print("/2013-09-01/classes/ToggleStocker/");
    sha256.print(objectId);
    sha256.print("\n");

    sha256.print("SignatureMethod=HmacSHA256&SignatureVersion=2&X-NCMB-Application-Key=");
    sha256.print(APPLICATION_KEY);
    sha256.print("&X-NCMB-Timestamp=");
    sha256.print(TIMESTAMP);
    return base64::encode(sha256.resultHmac(), HASH_LENGTH);
}

bool YudetamagoClient::GetExistance(const char *objectId, bool& exists, String& error)
{
    String url = "https://mb.api.cloud.nifty.com/2013-09-01/classes/ToggleStocker?where=%7B%22objectId%22%3A%22";
    url += objectId;
    url += "%22%7D";
    HTTPClient http;
    http.begin(url);
    ///////////////////////////////////////////////////////////////////////////////////////////
    // http.begin(url, NCMB_FINGERPRINT);                                                    //
    //                                                                                       //
    // esp32 has error occured.                                                              //
    //                                                                                       //
    // Log                                                                                   //
    // [E][ssl_client.cpp:28] handle_error(): X509 - The CRT/CRL/CSR format is invalid,      //
    //     e.g. different type expected                                                      //
    // [E][ssl_client.cpp:30] handle_error(): MbedTLS message code: -8576                    //
    // [E][WiFiClientSecure.cpp:108] connect(): lwip_connect_r: 22                           //
    // [ERROR] GetExistance(rZelDIxpcmRMgKNQ) error: http status code -1: connection refused //
    ///////////////////////////////////////////////////////////////////////////////////////////
    http.addHeader("X-NCMB-Application-Key", APPLICATION_KEY);
    http.addHeader("X-NCMB-Timestamp",       TIMESTAMP);
    http.addHeader("X-NCMB-Signature",       getGetExistanceSignature(objectId));
    http.addHeader("Content-Type",           "application/json");
    int httpCode = http.GET();
    if (httpCode != 200) {
        error = "GetExistance(";
        error += objectId;
        error += ") error: ";
        error += "http status code ";
        error += httpCode;
        error += ": ";
        error += http.errorToString(httpCode);
        http.end();
        return false;
    }

    String body = http.getString();
    if (body.lastIndexOf("\"existing\":\"1\"") != -1) {
        exists = true;
        http.end();
        return true;
    } else if (body.lastIndexOf("\"existing\":\"0\"") != -1) {
        exists = false;
        http.end();
        return true;
    }

    error = "GetExistance(";
    error += objectId;
    error += ") unknown body error: ";
    error += body;
    http.end();
    return false;
}

bool YudetamagoClient::SetExistance(const char *objectId, bool exists, String& error)
{
    String command = exists? "{\"existing\":\"1\"}": "{\"existing\":\"0\"}";
    String url = "https://mb.api.cloud.nifty.com/2013-09-01/classes/ToggleStocker/";
    url += objectId;
    HTTPClient http;
    http.begin(url);
    ///////////////////////////////////////////////////////////////////////////////////////////
    // http.begin(url, NCMB_FINGERPRINT);                                                    //
    //                                                                                       //
    // esp32 has error occured.                                                              //
    //                                                                                       //
    // Log                                                                                   //
    // [E][ssl_client.cpp:28] handle_error(): X509 - The CRT/CRL/CSR format is invalid,      //
    //     e.g. different type expected                                                      //
    // [E][ssl_client.cpp:30] handle_error(): MbedTLS message code: -8576                    //
    // [E][WiFiClientSecure.cpp:108] connect(): lwip_connect_r: 22                           //
    // [ERROR] GetExistance(rZelDIxpcmRMgKNQ) error: http status code -1: connection refused //
    ///////////////////////////////////////////////////////////////////////////////////////////
    http.addHeader("X-NCMB-Application-Key", APPLICATION_KEY);
    http.addHeader("X-NCMB-Timestamp",       TIMESTAMP);
    http.addHeader("X-NCMB-Signature",       getSetExistanceSignature(objectId));
    http.addHeader("Content-Type",           "application/json");
    int httpCode = http.sendRequest("PUT", command);
    if (httpCode != 200) {
        error = "SetExistance(";
        error += objectId;
        error += ") error: ";
        error += "http status code ";
        error += httpCode;
        error += ": ";
        error += http.errorToString(httpCode);
        http.end();
        return false;
    }

    http.end();
    return true;
}

