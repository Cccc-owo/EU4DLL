# EU4DLL

本 DLL 使 Europa Universalis IV（欧陆风云4）能够正确显示双字节字符（中日韩等）。

## 本 Fork 特性

- **自动 UTF-8 转换**：本地化文件无需预处理，UTF-8 编码的 `.yml` 文件在加载时自动转换为游戏内部编码，同时兼容传统的预转码文件。
- **Steam 状态修复**：Steam 好友列表中显示的富状态文本（如国家名、统治者头衔）不再乱码，正确显示 CJK 字符。
- **校验码覆盖**：强制游戏显示指定的校验码（如 `491d` 对应原版 1.37.5），使带 Mod 的游戏在多人游戏中显示为未修改状态。
- **成就解锁**：绕过游戏的校验码验证，使开启 Mod 时仍可获取成就。
- **Linux 支持**：通过 Proton/Wine 的 `version.dll` 覆盖方式运行。

## 注意事项

- 本项目为**非官方**项目。
- 仅支持 EU4 v1.37.x。
- 支持 Windows 和 Linux（通过 Proton/Wine），仅限 Steam 版本。
- 不支持 macOS。

## 使用方法

### Windows

1. 下载最新的发布压缩包并解压。
2. 将 `version.dll` 和 `plugins/` 文件夹复制到 EU4 游戏目录。
3. 从 Steam 创意工坊订阅多字节 Mod，或将 Mod 添加到 Paradox 用户目录。
4. 在游戏启动器中启用 Mod。
5. 开始游戏。

### Linux（Steam Play / Proton）

1. 按照 Windows 的步骤放置文件。
2. 在 Steam 中右键 EU4 > 属性 > 启动选项，添加：

   ```bash
   WINEDLLOVERRIDES="version=n,b" %command%
   ```

   这会让 Wine/Proton 加载游戏目录中的 `version.dll`，而非内置版本。
3. 开始游戏。

> **提示：** 如果之后删除了 Mod DLL 文件，游戏仍可正常启动 — Wine 会回退到内置的 `version.dll`。

## 反馈 Bug

如果发现 Bug，请在本仓库中**创建 Issue**。

## 配置说明

### plugins/plugin.ini

#### SEPARATE_CHARACTER_CODE_POINT

修改连接地区名称和国家名称的分隔字符。参见 [ISSUE-164](https://github.com/matanki-saito/EU4dll/issues/164)。

#### REVERSING_WORDS_BATTLE_OF_AREA

调整词序：

- Battle of / xxx -> xxx / Battle of
- Siege of / xxx -> xxx / Siege of
- Occupation of xxx -> xxx / Occupation of

#### AUTO_UTF8_CONVERSION

设为 `yes`（默认）时，Mod 中 UTF-8 编码的 `.yml` 和 `.txt` 文件在加载时自动转换为游戏内部的转义编码。原版游戏文件不受影响。

#### STEAM_RICH_PRESENCE

设为 `yes`（默认）时，修复 Steam 个人资料中富状态显示的 CJK 乱码问题（国家名、统治者头衔等）。

#### CHECKSUM_OVERRIDE

设为 4 位十六进制字符串（如 `491d`）可强制游戏显示该校验码，而非实际计算值。原理是拦截 `checksum_manifest.txt` 并在内存中替换校验码字符串。留空则禁用。

#### ACHIEVEMENT_UNLOCK

设为 `yes` 时，修补游戏内部的校验码验证逻辑，使开启 Mod 时仍可获取成就。默认为 `no`。

### 姓名顺序

在姓氏前添加倒问号（¿），可将名和姓的顺序反转。

```paradox
1534.6.23 = {
  heir = {
    name = "Nobunaga"  # Nobunaga
    monarch_name = "Nobunaga"  # Nobunaga
    dynasty = "¿Oda"  # ¿Oda
    birth_date = 1534.6.23
    death_date = 1582.6.21
    claim = 90
    adm = 5
    dip = 5
    mil = 6
  }
}
# Nobunaga Oda -> Oda Nobunaga（织田信长）
```

使用此功能时，请将倒问号（¿）从所有字体中排除。

## 构建

需要 MinGW-w64 工具链（GCC，支持 C++20）。

### Windows（本地构建）

使用 MSYS2 或独立的 MinGW-w64：

```bash
cmake -B build -S src -G "MinGW Makefiles"
cmake --build build
cmake --build build --target package
```

### Linux / macOS（交叉编译）

```bash
cmake -B build -S src -DCMAKE_TOOLCHAIN_FILE=src/toolchain/mingw-w64-x86_64.cmake
cmake --build build
cmake --build build --target package
```

输出位于 `build/package/`。

## 许可证

MIT 许可证

## 致谢

本 DLL 基于以下项目 Fork 而来，在此表示感谢。

[EU4CHS](https://bitbucket.org/kelashi/eu4chs/src/master/)

[EU4JPS](https://github.com/matanki-saito/EU4dll/tree/develop)
