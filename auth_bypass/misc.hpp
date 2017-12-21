/*
    MIT License

    Copyright (c) 2017, namreeb (legal@namreeb.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#pragma once

#include "CDataStore.hpp"

#include <string>
#include <vector>
#include <cstdint>

namespace misc
{
void GetgN(const std::string &username, std::vector<std::uint8_t> &g, std::vector<std::uint8_t> &N);

enum class AuthResult : unsigned char
{
    WOW_SUCCESS = 0x00,
    WOW_FAIL_UNKNOWN0 = 0x01,           //< ? Unable to connect
    WOW_FAIL_UNKNOWN1 = 0x02,           //< ? Unable to connect
    WOW_FAIL_BANNED = 0x03,             //< This <game> account has been closed and is no longer available for use. Please go to <site>/banned.html for further information.
    WOW_FAIL_UNKNOWN_ACCOUNT = 0x04,    //< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    WOW_FAIL_INCORRECT_PASSWORD = 0x05, //< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
                                        //  client reject next login attempts after this error, so in code used WOW_FAIL_UNKNOWN_ACCOUNT for both cases
    WOW_FAIL_ALREADY_ONLINE = 0x06,     //< This account is already logged into <game>. Please check the spelling and try again.
    WOW_FAIL_NO_TIME = 0x07,            //< You have used up your prepaid time for this account. Please purchase more to continue playing
    WOW_FAIL_DB_BUSY = 0x08,            //< Could not log in to <game> at this time. Please try again later.
    WOW_FAIL_VERSION_INVALID = 0x09,    //< Unable to validate game version. This may be caused by file corruption or interference of another program. Please visit <site> for more information and possible solutions to this issue.
    WOW_FAIL_VERSION_UPDATE = 0x0A,     //< Downloading
    WOW_FAIL_INVALID_SERVER = 0x0B,     //< Unable to connect
    WOW_FAIL_SUSPENDED = 0x0C,          //< This <game> account has been temporarily suspended. Please go to <site>/banned.html for further information
    WOW_FAIL_FAIL_NOACCESS = 0x0D,      //< Unable to connect
    WOW_SUCCESS_SURVEY = 0x0E,          //< Connected.
    WOW_FAIL_PARENTCONTROL = 0x0F,      //< Access to this account has been blocked by parental controls. Your settings may be changed in your account preferences at <site>
    WOW_FAIL_LOCKED_ENFORCED = 0x10,    //< You have applied a lock to your account. You can change your locked status by calling your account lock phone number.
    WOW_FAIL_TRIAL_ENDED = 0x11,        //< Your trial subscription has expired. Please visit <site> to upgrade your account.
    WOW_FAIL_USE_BATTLENET = 0x12,      //< WOW_FAIL_OTHER This account is now attached to a Battle.net account. Please login with your Battle.net account email address and password.
};

enum Version
{
    Classic = 0,
    TBC,
    WOTLK,
    Cata,
    MAX
};
}