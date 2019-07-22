![Screenshot](https://i.imgur.com/4D5YEdZ.png)

# xSkins: External Knife & Skin Changer for CSGO

## Features

- Optimized and faster than any other external skin changer
- Self-updates using an external memory library ([xLiteMem](https://github.com/0xf1a/xLiteMem))
- Small console menu for selecting knife models and skins
- Parser for loading skin values from a .txt file

## How it works

The m_hMyWeapons array contains handles to all weapons equipped by the local player. We can apply skin and model values to those weapons' entities independent to which weapon the local player is holding in hands. This alone perfectly applies skins and models without having to write memory to the fullupdate offset. Lastly, the current weapon viewmodel entity is set to the model index of our chosen knife model.

## Fix for strings in VS2017+

Visual Studio 2017 15.5 started setting the /permissive- flag for all new solutions, which disallows the implicit conversion of string literals to non-const char*. You can edit the solution's properties to disable that flag while you update your codebase to conform to the C++ standard. It's listed as "Conformance mode" in the "Language" tab under "C/C++" in the project's properties.

## Credits

- BuckshotYT for [GetModelIndex method](https://www.unknowncheats.me/forum/counterstrike-global-offensive/212036-model-indices-properly-externally.html)
- tracersgta for [skin list](https://www.unknowncheats.me/forum/counterstrike-global-offensive/300854-skin-list-ids-comments.html)

## Important

I'm not responsible for any bans that might occur on your account while using this software. Use at your own risk.