/* help.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */


#include "conf.h"
#include "packmast.h"
#include "packer.h"
#include "compress/compress.h" // upx_ucl_version_string()


/*************************************************************************
//
**************************************************************************/

static bool head_done = 0;

// also see UPX_CONFIG_DISABLE_GITREV in CMakeLists.txt
#if defined(UPX_VERSION_GITREV)
const char gitrev[] = UPX_VERSION_GITREV;
#else
const char gitrev[1] = { 0 };
#endif

void show_head(void)
{
    FILE *f = con_term;
    int fg;

    if (head_done)
        return;
    head_done = 1;

    fg = con_fg(f,FG_GREEN);
    con_fprintf(f,
                "                          可执行文件的专业打包程序\n\n"
                "                          Copyright (C) 1996 - " UPX_VERSION_YEAR "\n"
#if defined(UPX_VERSION_GITREV)
                ""
#else
                ""
#endif
                "                           UPX 4.02 草原企鹅汉化版\n\n"
// #if defined(UPX_VERSION_GITREV)
//                 gitrev,
//                 (sizeof(gitrev)-1 > 6 && gitrev[sizeof(gitrev)-2] == '+') ? '' : ' ',
// #else
//                 UPX_VERSION_STRING,
// #endif
                );
    fg = con_fg(f,fg);
#undef V

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_usage(void)
{
    FILE *f = con_term;

    con_fprintf(f,"用法: %s [-123456789dlthVL] [-qvfk] [-o file] %s文件名..\n", progname,
#if (ACC_OS_DOS32) && defined(__DJGPP__)
                "[@]");
#else
                "");
#endif
}


/*************************************************************************
//
**************************************************************************/

struct PackerNames
{
    struct Entry {
        const char* fname;
        const char* sname;
    };
    Entry names[64];
    size_t names_count;
    const options_t *o;
    PackerNames() : names_count(0), o(nullptr) { }
    void add(const Packer *p)
    {
        p->assertPacker();
        assert(names_count < 64);
        names[names_count].fname = p->getFullName(o);
        names[names_count].sname = p->getName();
        names_count++;
    }
    static Packer* visit(Packer *p, void *user)
    {
        PackerNames *self = (PackerNames *) user;
        self->add(p);
        delete p; p = nullptr;
        return nullptr;
    }
    static int __acc_cdecl_qsort cmp_fname(const void *a, const void *b) {
        return strcmp(((const Entry *) a)->fname, ((const Entry *) b)->fname);
    }
    static int __acc_cdecl_qsort cmp_sname(const void *a, const void *b) {
        return strcmp(((const Entry *) a)->sname, ((const Entry *) b)->sname);
    }
};

static void show_all_packers(FILE *f, int verbose)
{
    options_t o; o.reset();
    PackerNames pn; pn.o = &o;
    PackMaster::visitAllPackers(PackerNames::visit, nullptr, &o, &pn);
    qsort(pn.names, pn.names_count, sizeof(PackerNames::Entry), PackerNames::cmp_fname);
    size_t pos = 0;
    for (size_t i = 0; i < pn.names_count; ++i)
    {
        const char *fn = pn.names[i].fname;
        const char *sn = pn.names[i].sname;
        if (verbose > 0)
        {
            con_fprintf(f, "    %-36s %s\n", fn, sn);
        }
        else
        {
            size_t fl = strlen(fn);
            if (pos == 0) {
                con_fprintf(f, "  %s", fn);
                pos = 2 + fl;
            } else if (pos + 1 + fl > 80) {
                con_fprintf(f, "\n  %s", fn);
                pos = 2 + fl;
            } else {
                con_fprintf(f, " %s", fn);
                pos += 1 + fl;
            }
        }
    }
    if (verbose <= 0 && pn.names_count)
        con_fprintf(f, "\n");
}


/*************************************************************************
//
**************************************************************************/

void show_help(int verbose)
{
    FILE *f = con_term;
    int fg;

    show_head();
    show_usage();

    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"\n命令:\n");
    fg = con_fg(f,fg);
    con_fprintf(f,
                "  -1     压缩更快                   -9    压缩更好\n"
                "%s"
                "  -d     解压缩                     -l    列出压缩文件\n"
                "  -t     测试压缩文件               -V    显示版本号\n"
                "  -h     获取%s帮助               -L    显示软件许可证\n%s",
                verbose == 0 ? "" : "  --更好压缩 (对于大文件会稍慢一些)\n",
                verbose == 0 ? "更多" : "这个", verbose == 0 ? "" : "\n");

    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"选项:\n");
    fg = con_fg(f,fg);

    con_fprintf(f,
                "  -q     不输出详细信息             -v    输出详细信息\n"
                "  -oFILE 将结果写入 'FILE'\n"
                //"  -f     force overwrite of output files and compression of suspicious files\n"
                "  -f     强制压缩可疑文件\n"
                "%s%s"
                , (verbose == 0) ? "  -k     保留备份文件\n" : ""
#if 1
                , (verbose > 0) ? "  --no-color, --mono, --color, --no-progress   改变外观\n" : ""
#else
                , ""
#endif
                );

    if (verbose > 0)
    {
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"\n压缩调整选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --lzma              尝试LZMA[比NRV更慢但更紧]\n"
                    "  --brute             尝试所有可用的压缩方法和过滤器 [慢]\n"
                    "  --ultra-brute       尝试更多的压缩变量 [很慢]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"备份选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  -k, --backup        保留备份文件\n"
                    "  --no-backup         不保留备份文件 [默认选项]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"覆盖选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --overlay=copy      复制附加到文件的任何额外数据 [默认选项]\n"
                    "  --overlay=strip     删除附加到文件的所有额外数据 [危险选项]\n"
                    "  --overlay=skip      不使用覆盖来压缩文件\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"djgpp2/coff选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --coff              生成COFF输出 [默认: EXE]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"dos/com选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              使压缩的com在任何8086上工作\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"dos/exe选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              使压缩的可执行文件在任何8086上运行\n"
                    "  --no-reloc          不在可执行文件头中放置任何位置\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"dos/sys选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8086              使压缩的sys文件在任何8086上运行\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"ps1/exe选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --8-bit             使用8位大小的压缩 [默认: 32 bit]\n"
                    "  --8mib-ram          8 MB内存限制 [默认: 2 MiB]\n"
                    "  --boot-only         禁用 客户端/主机传输兼容性\n"
                    "  --no-align          不对齐到2048字节 [启用: --console-run]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"watcom/le选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --le                LE输出 [默认: EXE]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"win32/pe, win64/pe, rtm32/pe & arm/pe选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --compress-exports=0    不压缩导出节\n"
                    "  --compress-exports=1    压缩导出节 [默认]\n"
                    "  --compress-icons=0      不压缩任何图标\n"
                    "  --compress-icons=1      压缩除第一个图标外的所有图标\n"
                    "  --compress-icons=2      压缩除第一个图标目录外的所有图标目录 [默认]\n"
                    "  --compress-icons=3      压缩所有图标\n"
                    "  --compress-resources=0  不压缩任何资源\n"
                    "  --keep-resource=list    不压缩列表指定的资源\n"
                    "  --strip-relocs=0        不剥离位置调整\n"
                    "  --strip-relocs=1        剥离位置调整 [默认]\n"
                    "\n");
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"linux/elf选项:\n");
        fg = con_fg(f,fg);
        con_fprintf(f,
                    "  --preserve-build-id     将.gnu .note .build-id复制到压缩输出\n"
                    "\n");
    }

    con_fprintf(f, "文件..   要(解)压缩的可执行文件\n");

    if (verbose > 0)
    {
        fg = con_fg(f,FG_YELLOW);
        con_fprintf(f,"\n此版本支持:\n");
        fg = con_fg(f,fg);
        show_all_packers(f, verbose);
    }
    else
    {
        con_fprintf(f,"\n输入 '%s --help' 获取更详细的帮助.\n", progname);
    }

    con_fprintf(f,"\nUPX为开源软件; 详情请访问 https://upx.github.io\n"
//                "\nUPX comes with ABSOLUTELY NO WARRANTY; for details type 'upx -L'.\n"
                "");

#if DEBUG || TESTING
    fg = con_fg(f,FG_RED);
    con_fprintf(f,"\n警告: 此版本编译于:"
#if DEBUG
                " -DDEBUG"
#endif
#if TESTING
                " -DTESTING"
#endif
                "\n");
    fg = con_fg(f,fg);
#endif

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_license(void)
{
    FILE *f = con_term;

    show_head();

    con_fprintf(f,
        "   本程序可以自由使用，欢迎您在一定条件下重新发布\n"
        "   发布这个程序是希望它会有用,\n"
        "   但没有任何保证;\n"
        "   请查看\n"
        "   有关UPX许可协议的详细信息.\n"
        "\n"
        "   您应该已经收到一份UPX许可协议的副本\n"
        "   伴随着这个程序;见文件复制和许可证.\n"
        "   如果没有，请访问以下页面之一:\n"
        "\n"
    );
    int fg = con_fg(f,FG_CYAN);
    con_fprintf(f,
        "        https://upx.github.io\n"
        "        https://www.oberhumer.com/opensource/upx/\n"
    );
    (void)con_fg(f,FG_ORANGE);
    con_fprintf(f,
        "\n"
        "   Markus F.X.J. Oberhumer              Laszlo Molnar\n"
        "   <markus@oberhumer.com>               <ezerotven+github@gmail.com>\n"
    );
    fg = con_fg(f,fg);

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void show_version(bool one_line)
{
    FILE *fp = stdout;
    const char *v;

#if defined(UPX_VERSION_GIT_DESCRIBE)
    fprintf(fp, "upx %s\n", UPX_VERSION_GIT_DESCRIBE);
#elif defined(UPX_VERSION_GITREV)
    fprintf(fp, "upx %s\n", UPX_VERSION_STRING "-git-" UPX_VERSION_GITREV);
#else
    fprintf(fp, "upx %s\n", UPX_VERSION_STRING);
#endif
    if (one_line)
        return;
#if (WITH_NRV)
    v = upx_nrv_version_string();
    if (v != nullptr && v[0])
        fprintf(fp, "NRV data compression library %s\n", v);
#endif
#if (WITH_UCL)
    v = upx_ucl_version_string();
    if (v != nullptr && v[0])
        fprintf(fp, "UCL data compression library %s\n", v);
#endif
#if (WITH_ZLIB)
    v = upx_zlib_version_string();
    if (v != nullptr && v[0])
        fprintf(fp, "zlib data compression library %s\n", v);
#endif
#if (WITH_LZMA)
    v = upx_lzma_version_string();
    if (v != nullptr && v[0])
        fprintf(fp, "LZMA SDK version %s\n", v);
#endif
#if (WITH_ZSTD)
    v = upx_zstd_version_string();
    if (v != nullptr && v[0])
        fprintf(fp, "zstd data compression library %s\n", v);
#endif
#if !defined(DOCTEST_CONFIG_DISABLE)
    fprintf(fp, "doctest C++ testing framework version %s\n", DOCTEST_VERSION_STR);
#endif
    fprintf(fp, "Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer\n");
    fprintf(fp, "Copyright (C) 1996-2023 Laszlo Molnar\n");
    fprintf(fp, "Copyright (C) 2000-2023 John F. Reiser\n");
    fprintf(fp, "Copyright (C) 2002-2023 Jens Medoch\n");
#if (WITH_ZLIB)
    fprintf(fp, "Copyright (C) 1995" "-2022 Jean-loup Gailly and Mark Adler\n");
#endif
#if (WITH_LZMA)
    fprintf(fp, "Copyright (C) 1999" "-2006 Igor Pavlov\n");
#endif
#if (WITH_ZSTD)
    // see vendor/zstd/LICENSE; main author is Yann Collet
    fprintf(fp, "Copyright (C) 2015" "-2023 Meta Platforms, Inc. and affiliates\n");
#endif
#if !defined(DOCTEST_CONFIG_DISABLE)
    fprintf(fp, "Copyright (C) 2016" "-2021 Viktor Kirilov\n");
#endif
    fprintf(fp, "UPX是开源软件,获取具体帮助请参阅 '%s -L'.\n", progname);
}

/* vim:set ts=4 sw=4 et: */