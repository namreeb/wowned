# wowned
This application is a proof of concept exploit for the authentication bypass methods in many World of Warcraft emulation
authentication servers discovered by Chaosvex (https://github.com/Chaosvex) and Daemon (https://github.com/DevDaemon).

To use, `auth_bypass.dll` must be injected into wow.exe (versions 1.12.1, 2.4.3 and 3.3.5a are supported).  An injector `wowned.exe`
is included.

An example usage would be:

`wowned.exe -c -p "f:\wow 3.3.5\WoW.exe" --2`

wowned.exe --help output:

```
wowned v0.1 injector
Allowed options:
  -h [ --help ]                   display help message
  -c [ --console ]                enable wow console
  -p [ --program ] arg (=wow.exe) path to wow binary
  --1                             exploit method one
  --2                             exploit method two
  ```
  
# ethics
The bugs which this application will exploit have been publicly disclosed since early November 2016 (see here:
https://www.reddit.com/r/wowservers/comments/5b0chc/attention_server_developers_and_administrators/).  Some private servers have
opted to ignore the warning.  It is a common practice among security researched to release a proof of concept exploit after
vendors and users have had ample opportunity to apply a patch.  Doing so can encourage the remaining vendors or users to follow suit.

For reference, these are two commits which fix 'method one' and 'method two' respectively:

https://github.com/cmangos/mangos-classic/commit/74d51cf70d67f6d4a47321a4226e7473cb8e2601
https://github.com/cmangos/mangos-classic/commit/0d2b7e38c886ddd6828cfa75e2daba5121467383

# credit
As mentioned above, credit for the initial discovery goes to Chaosvex.  Credit for the discovery of method two goes to
Daemon of nostalrius.org, who found the second issue when he and I were discussing the first one.

# impact
Some of the private servers that I have tested this on are still vulnerable.  If you are a private server administrator and for
whatever reason are unable to adapt the above-linked commits to your code, please feel free to contact me.