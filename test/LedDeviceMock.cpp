// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

class LedDeviceExpect {
public:
    static void Open();
    static void Close();
    static void SetColor(int index, const LedColor& color);
    static void SetColorMask(int ledMask, const LedColor& color);
    static void Show();
};

void LedDevice::Open()
{
    mock().actualCall("LedDevice::Open");
}

void LedDeviceExpect::Open()
{
    mock().expectOneCall("LedDevice::Open");
}

void LedDevice::Close()
{
    mock().actualCall("LedDevice::Close");
}

void LedDeviceExpect::Close()
{
    mock().expectOneCall("LedDevice::Close");
}

void LedDevice::SetColor(int index, const LedColor& color)
{
    mock().actualCall("LedDevice::SetColor")
        .withParameter("index",       index)
        .withParameter("color.red",   color.red)
        .withParameter("color.green", color.green)
        .withParameter("color.blue",  color.blue);
}

void LedDeviceExpect::SetColor(int index, const LedColor& color)
{
    mock().expectOneCall("LedDevice::SetColor")
        .withParameter("index",       index)
        .withParameter("color.red",   color.red)
        .withParameter("color.green", color.green)
        .withParameter("color.blue",  color.blue);
}

void LedDevice::SetColorMask(int ledMask, const LedColor& color)
{
    mock().actualCall("LedDevice::SetColorMask")
        .withParameter("ledMask",     ledMask)
        .withParameter("color.red",   color.red)
        .withParameter("color.green", color.green)
        .withParameter("color.blue",  color.blue);
}

void LedDeviceExpect::SetColorMask(int ledMask, const LedColor& color)
{
    mock().expectOneCall("LedDevice::SetColorMask")
        .withParameter("ledMask",     ledMask)
        .withParameter("color.red",   color.red)
        .withParameter("color.green", color.green)
        .withParameter("color.blue",  color.blue);
}

void LedDevice::Show()
{
    mock().actualCall("LedDevice::Show");
}

void LedDeviceExpect::Show()
{
    mock().expectOneCall("LedDevice::Show");
}

