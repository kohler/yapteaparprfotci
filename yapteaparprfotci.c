/* yapteaparprfotci.cc -- Yet Another Program To Enliven Academic Papers
 * through the Application of Randomly-Placed Royalty-Free Old-Time Cat
 * Imagery
 *
 * Copyright (c) 2005-2016 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */


#include <config.h>
#include <lcdf/clp.h>
#include <lcdfgif/gif.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Painful...
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

static int report_error;

typedef struct cat {
    const char *filename;
    int colorize;
    int lighten;
    int rotate;
    Gif_Stream *gfs;
} cat_t;

cat_t *cats;
int ncats;
int cats_capacity;

#define DIR_OPT 300
#define HELP_OPT 301
#define FILE_OPT 302
#define PAPER_OPT 303
#define VERSION_OPT 304
#define COLORIZE_OPT 305
#define LIGHTEN_OPT 306
#define NOCOLORIZE_OPT 307
#define NOLIGHTEN_OPT 308
#define OUTPUT_OPT 309
#define MAXCATS_OPT 310
#define SEED_OPT 311
#define ROTATE_OPT 312

Clp_Option options[] = {
    { "directory", 'd', DIR_OPT, Clp_ValString, 0 },
    { "help", 'h', HELP_OPT, 0, 0 },
    { "file", 'f', FILE_OPT, Clp_ValString, 0 },
    { "paper", 'p', PAPER_OPT, Clp_ValString, 0 },
    { "version", 'v', VERSION_OPT, 0, 0 },
    { "colorize", 'c', COLORIZE_OPT, 0, Clp_Negate },
    { "lighten", 'l', LIGHTEN_OPT, 0, Clp_Negate },
    { "output", 'o', OUTPUT_OPT, Clp_ValString, 0 },
    { "maxcats", 'm', MAXCATS_OPT, Clp_ValInt, 0 },
    { "seed", 's', SEED_OPT, Clp_ValUnsigned, Clp_Optional },
    { "rotate", 'r', ROTATE_OPT, Clp_ValUnsigned, Clp_Optional | Clp_Negate },
    { 0, 'C', NOCOLORIZE_OPT, 0, 0 },
    { 0, 'L', NOLIGHTEN_OPT, 0, 0 }
};

const char *program_name;
int maxcats = 20;


void
addcat(const char *name, int colorize, int lighten, int rotate)
{
    if (ncats >= cats_capacity) {
        cats_capacity = (cats_capacity ? 2 * cats_capacity : 1024);
        cats = realloc(cats, cats_capacity * sizeof(cat_t));
    }
    cats[ncats].filename = name;
    cats[ncats].gfs = 0;
    cats[ncats].colorize = colorize;
    cats[ncats].lighten = lighten;
    cats[ncats].rotate = rotate;
    ncats++;
}

typedef struct namelink {
    struct namelink *next;
    char name[1];
} namelink_t;

static char *
makedirnam(const char *dir, const char *end, const struct dirent *ent, int extra)
{
    int ent_namlen = NAMLEN(ent);
    char *x = (char *) malloc(end - dir + 2 + ent_namlen + extra) + extra;
    char *y = x + (end - dir);
    memcpy(x, dir, end - dir);
    if (end[-1] != '/')
        *y++ = '/';
    memcpy(y, ent->d_name, ent_namlen);
    y[ent_namlen] = '\0';
    return x - extra;
}

void
catdir(const char *dir, int recursive, int colorize, int lighten, int rotate)
{
    char *end;
    DIR *dirf;
    struct dirent *ent;
    namelink_t *recursion = 0;
    namelink_t *free_namelink = 0;

  again:
    end = strchr(dir, '\0');

    if (end - 2 >= dir && end[-1] == '/' && end[-2] == '/')
        recursive = 1;
    while (end - 2 > dir && end[-1] == '/' && end[-2] == '/')
        end--;

    if (!(dirf = opendir(dir))) {
        perror(dir);
        goto next;
    }

    while ((ent = readdir(dirf))) {
        const char *s = ent->d_name;
        int ent_namlen = NAMLEN(ent);
        namelink_t *n = (namelink_t *) makedirnam(dir, end, ent, offsetof(namelink_t, name));
        struct stat sbuf;

        // don't depend on unreliable parts of the dirent structure
        if (stat(n->name, &sbuf) < 0)
            /* do nothing */;
        else if (S_ISREG(sbuf.st_mode) && ent_namlen > 4
                 && s[ent_namlen - 4] == '.'
                 && tolower((unsigned char) s[ent_namlen - 3]) == 'g'
                 && tolower((unsigned char) s[ent_namlen - 2]) == 'i'
                 && tolower((unsigned char) s[ent_namlen - 1]) == 'f') {
            addcat(n->name, colorize, lighten, rotate);
            n = 0;              /* do not free */
        } else if (S_ISDIR(sbuf.st_mode) && recursive
                   && (ent_namlen > 1 || s[0] != '.')
                   && (ent_namlen > 2 || s[0] != '.' || s[1] != '.')) {
            n->next = recursion;
            recursion = n;
            n = 0;              /* do not free */
        }

        free(n);
    }

    closedir(dirf);

  next:
    free(free_namelink);
    if (recursion) {
        free_namelink = recursion;
        recursion = recursion->next;
        dir = free_namelink->name;
        goto again;
    }
}


void
gif_fileerror(Gif_Stream* gfs, Gif_Image* gfi, int is_error, const char* text)
{
    (void) gfs, (void) gfi;
    if (report_error && is_error)
        fprintf(stderr, "%s\n", text);
    report_error = 0;
}

void
hsvtorgb(double *r, double *g, double *b, double h, double s, double v)
{
    int i;
    double f, p, q, t;

    h /= 60;
    i = floor(h);
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch (i) {
      case 0:
        *r = v, *g = t, *b = p;
        break;
      case 1:
        *r = q, *g = v, *b = p;
        break;
      case 2:
        *r = p, *g = v, *b = t;
        break;
      case 3:
        *r = p, *g = q, *b = v;
        break;
      case 4:
        *r = t, *g = p, *b = v;
        break;
      case 5:
        *r = v, *g = p, *b = q;
        break;
    }
}


#define FRANDOM()       ((double) random() / 0x7FFFFFFF)

void
cat(FILE *f, int n, double bbox[4])
{
    int y, i, j, pos, transparent, bw;
    uint32_t val;
    Gif_Color *colors, white;
    unsigned char buf[8], outbuf[5];
    double r, g, b;
    double scale;
    double xoff, yoff;
    double ptwidth, ptheight;
    double dpi = 300;
    Gif_Stream *gfs;
    Gif_Image *gfi;

    white.gfc_red = white.gfc_green = white.gfc_blue = 255;

  again:
    // pick cat
    if (ncats == 0)
        return;
    do {
        i = random() % ncats;
        if (cats[i].filename && !cats[i].gfs) {
            FILE *f = fopen(cats[i].filename, "rb");
            if (f
                && (cats[i].gfs = Gif_FullReadFile(f, GIF_READ_UNCOMPRESSED, cats[i].filename, gif_fileerror))
                && cats[i].gfs->nimages > 0)
                /* OK */;
            else {
                if (cats[i].gfs)
                    Gif_DeleteStream(cats[i].gfs);
                cats[i].gfs = NULL;
                cats[i].filename = NULL;
            }
            if (f)
                fclose(f);
        }
    } while (!cats[i].gfs);

    gfs = cats[i].gfs;
    gfi = Gif_GetImage(gfs, 0);

    // pick random color
    if (!cats[i].colorize && !cats[i].lighten)
        r = g = b = 0;
    else
        hsvtorgb(&r, &g, &b, FRANDOM() * 360,
                 (cats[i].colorize ? (cats[i].lighten ? FRANDOM() * 0.75 : 1) : 0),
                 (cats[i].lighten ? FRANDOM() * 0.4 + 0.5 : 1));

    // pick scale and position
    scale = (FRANDOM() + 0.2) * 3;
    ptwidth = 72 * gfi->width / dpi * scale;
    ptheight = 72 * gfi->height / dpi * scale;
    xoff = bbox[0] + FRANDOM() * ((bbox[2] - bbox[0]) + .2 * ptwidth) - .2 * ptwidth;
    yoff = bbox[1] + FRANDOM() * ((bbox[3] - bbox[1]) + .2 * ptheight) - .2 * ptheight;

    colors = (gfi->local ? gfi->local->col : gfs->global->col);
    transparent = gfi->transparent;

    // Black-and-white image?  Use /Separation color space to compress PostScript.
    bw = 1;
    pos = (gfi->local ? gfi->local->ncol : gfs->global->ncol);
    for (i = 0; i < pos && bw; i++)
        if (colors[i].gfc_red != colors[i].gfc_green || colors[i].gfc_red != colors[i].gfc_blue)
            bw = 0;

    // Determine color space
    if (bw && r == g && g == b)
        fprintf(f, "gsave /DeviceGray setcolorspace\n");
    else if (bw) {
        static int catcolorspace = 0;
        fprintf(f, "gsave [/Separation (Cat Color %d) /DeviceRGB {neg 1 add dup", catcolorspace++);
        if (r > 0)
            fprintf(f, " %.10g mul %.10g add exch", 1 - r, r);
        fprintf(f, " dup");
        if (g > 0)
            fprintf(f, " %.10g mul %.10g add exch", 1 - g, g);
        if (b > 0)
            fprintf(f, " %.10g mul %.10g add", 1 - b, b);
        fprintf(f, "}] setcolorspace\n");
        bw = 2;
    } else
        fprintf(f, "gsave /DeviceRGB setcolorspace\n");

    fprintf(f, "%.10g %.10g translate", xoff, yoff);
    if (cats[i].rotate)
        fprintf(f, " %.10g rotate", (FRANDOM() - 0.5) * 2 * cats[i].rotate);
    fprintf(f, " %.10g %.10g scale\n", ptwidth, ptheight);
    fprintf(f, "<< /ImageType 4\n\
   /Width %d /Height %d /BitsPerComponent 8\n", gfi->width, gfi->height);

    // Color space decoding: let 0 = white to help compress
    if (bw > 1)
        // /Decode [0 1] to get around Acrobat Distiller bug with /MaskColor
        fprintf(f, "   /MaskColor [0] /Decode [0 1]\n");
    else if (bw)
        fprintf(f, "   /MaskColor [0] /Decode [1 %.10g]\n", r);
    else
        fprintf(f, "   /MaskColor [0 0 0] /Decode [1 %.10g 1 %.10g 1 %.10g]\n", r, g, b);

    fprintf(f, "   /ImageMatrix [%d 0 0 -%d 0 %d]\n\
   /DataSource currentfile /ASCII85Decode filter >>\n\
image\n", gfi->width, gfi->height, gfi->height);

    i = pos = 0;
    for (y = 0; y < gfi->height; y++) {
        const uint8_t *x = gfi->img[y];
        const uint8_t *endx = x + gfi->width;
        for (; x < endx; x++) {
            Gif_Color *c = (*x == transparent ? &white : &colors[*x]);

            if (bw)
                buf[i++] = 255 - c->gfc_red;
            else {
                buf[i++] = 255 - c->gfc_red;
                buf[i++] = 255 - c->gfc_green;
                buf[i++] = 255 - c->gfc_blue;
            }

            if (i >= 4) {
                val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
                if (val == 0) {
                    fputc('z', f);
                    pos++;
                } else {
                    for (j = 4; j >= 0; j--) {
                        outbuf[j] = 33 + val % 85;
                        val /= 85;
                    }
                    fwrite(outbuf, 1, 5, f);
                    pos += 5;
                }
                memcpy(buf, buf + 4, 4);
                i -= 4;
                if (pos >= 72)
                    fputc('\n', f), pos = 0;
            }
        }
    }

    if (i > 0) {
        buf[i] = buf[i + 1] = buf[i + 2] = 0;
        val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
        for (j = 4; j >= 0; j--) {
            outbuf[j] = 33 + val % 85;
            val /= 85;
        }
        fwrite(outbuf, 1, i + 1, f);
    }

    fputs("\n~>\ngrestore\n", f);

    if (FRANDOM() > 0.2 && n < maxcats && maxcats > 0) {
        n++;
        goto again;
    }
}

// Ignore white rectangular fills at the beginnings of pages.
int check_ps2write(int ps2write_state, char* buf) {
    double g;
    int n;
    if (ps2write_state == 1 && memcmp(buf, "%%BeginPageSetup", 16) == 0)
        return 2;
    else if (ps2write_state == 2)
        return memcmp(buf, "%%EndPageSetup", 14) == 0 ? 1 : 2;
    else if (buf[0] == '%')
        return ps2write_state;
    else if (memcmp(buf, "1 1 1 RG\n", 9) == 0
             || memcmp(buf, "1 1 1 rg\n", 9) == 0)
        return 3;
    else if (memcmp(buf, "f\n", 2) == 0 && ps2write_state == 3) {
        buf[0] = 'n';
        return 3;
    }
    n = 0;
    if (sscanf(buf, " << /Length %lg >> stream %n", &g, &n) > 0 && n > 0) {
        while (isspace((unsigned char) buf[n]))
            ++n;
        return buf[n] ? 0 : ps2write_state;
    }
    n = 0;
    while (buf[n]) {
        if (isspace((unsigned char) buf[n])
            || ((buf[n] == 'q' || buf[n] == 'W' || buf[n] == 'n')
                && isspace((unsigned char) buf[n+1])))
            ++n;
        else if (((buf[n] == 'c' && buf[n+1] == 'm')
                  || (buf[n] == 'r' && buf[n+1] == 'e'))
                 && isspace((unsigned char) buf[n+2]))
            n += 2;
        else if (buf[n] == 'o' && buf[n+1] == 'b' && buf[n+2] == 'j'
                 && isspace((unsigned char) buf[n+3]))
            n += 3;
        else if (isdigit((unsigned char) buf[n]) || buf[n] == '-') {
            for (++n; isdigit((unsigned char) buf[n]) || buf[n] == '.'; ++n)
                /* skip */;
            if (!isspace((unsigned char) buf[n]))
                return 0;
        } else
            return 0;
    }
    return ps2write_state;
}

void
usage()
{
    printf("\
'Yapteaparprfotci' is obviously Yet Another Program To Enliven\n\
Academic Papers through the Application of Randomly-Placed Royalty-Free\n\
Old Time Cat Imagery.\n\
\n\
Usage: %s -d CATDIR [OPTIONS]... PAPER > OUTPUT\n\n",
           program_name);
    printf("\
Options:\n\
  -p, --paper PAPER       Enliven the given PDF or PostScript file. PS\n\
                          must follow the Document Structuring Conventions.\n\
  -d, --directory DIR     Use cat imagery from DIR.  If DIR ends with //,\n\
                          recursively search all directories under DIR.\n\
                          Finds GIF files named `*.gif` or `*.GIF` only.\n\
  -f, --file FILE         Use the cat imagery in GIF FILE.\n\
  -o, --output FILE       Write output to FILE.\n\
  -C, --no-colorize       Do not colorize subsequent cat imagery.\n\
  -L, --no-lighten        Do not lighten subsequent cat imagery.\n\
  -r, --rotate[=N]        Rotate subsequent cat imagery buy up to +/-N degrees.\n\
  -m, --maxcats N         Place at most N cat images [default 20].\n\
  -s, --seed S            Set random seed to S [default random].\n\
  -h, --help              Print this message and exit.\n\
  -q, --quiet             Do not generate any error messages.\n\
      --version           Print version number and exit.\n\
\n\
Report bugs to Christine Lavin.\n");
}

void
usage_error()
{
    fprintf(stderr, "Usage: %s [OPTION]... PAPER\n", program_name);
    fprintf(stderr, "Type %s --help for more information.\n", program_name);
    exit(1);
}

int
main(int argc, char *argv[])
{
    Clp_Parser *clp = Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
    char buf[BUFSIZ];
    const char *paper = NULL, *output = NULL;
    FILE *pf, *of;
    int overflow, nest, is_ps2write = -1, ps2write_state = 0;
    int colorize = 1, lighten = 1, newcoloropt = 0, random_random_seed = 1;
    int rotate = 0;
    uint32_t random_seed = 0;

    program_name = Clp_ProgramName(clp);

    while (1) {
        int opt = Clp_Next(clp);
        switch (opt) {

          case DIR_OPT:
            catdir(clp->vstr, 0, colorize, lighten, rotate);
            newcoloropt = 0;
            break;

          case FILE_OPT:
            addcat(clp->vstr, colorize, lighten, rotate);
            newcoloropt = 0;
            break;

          case PAPER_OPT:
            if (paper) {
                fprintf(stderr, "%s: `--paper` specified twice\n", program_name);
                usage_error();
            }
            paper = clp->vstr;
            break;

          case OUTPUT_OPT:
            if (output) {
                fprintf(stderr, "%s: `--output` specified twice\n", program_name);
                usage_error();
            }
            output = clp->vstr;
            break;

          case SEED_OPT:
            if (clp->negated)
                random_random_seed = -1;
            else if (!clp->have_val)
                random_random_seed = 1;
            else
                random_random_seed = 0, random_seed = clp->val.u;
            break;

          case COLORIZE_OPT:
            colorize = !clp->negated;
            newcoloropt = 1;
            break;

          case LIGHTEN_OPT:
            lighten = !clp->negated;
            newcoloropt = 1;
            break;

          case ROTATE_OPT:
            if (clp->negated)
                rotate = 0;
            else if (clp->have_val)
                rotate = clp->val.i;
            else
                rotate = 20;
            newcoloropt = 1;
            break;

          case NOCOLORIZE_OPT:
            colorize = 0;
            newcoloropt = 1;
            break;

          case NOLIGHTEN_OPT:
            lighten = 0;
            newcoloropt = 1;
            break;

          case MAXCATS_OPT:
            maxcats = clp->val.i;
            break;

          case HELP_OPT:
            usage();
            exit(0);

          case VERSION_OPT:
            printf("yapteaparprfotci %s\n", VERSION);
            printf("Copyright (c) 2005-2016 Eddie Kohler\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty, not even for merchantability or fitness for a\n\
particular purpose.\n");
            exit(0);
            break;

          case Clp_NotOption:
            if (paper) {
                fprintf(stderr, "%s: too many arguments\n", program_name);
                usage_error();
            }
            paper = clp->vstr;
            break;

          case Clp_Done:
            goto done;

          case Clp_BadOption:
            usage_error();

        }
    }

  done:
    if (newcoloropt)
        fprintf(stderr, "%s: warning: last colorize/lighten options ignored\n(These options only affect subsequently-named cat imagery.)\n", program_name);
    if (!ncats)
        fprintf(stderr, "%s: warning: no cat imagery!  (Try `-d DIR` or `-f FILE`.)\n", program_name);
    if (!paper || strcmp(paper, "-") == 0) {
        pf = stdin;
        paper = "<stdin>";
    } else {
        pf = fopen(paper, "r");
        if (!pf) {
            perror(paper);
            exit(1);
        }
        fclose(stdin);
    }

    // Set random seed
    if (random_random_seed == 0)
        srandom(random_seed);
    else if (random_random_seed > 0) {
#if HAVE_SRANDOMDEV
        srandomdev();
#else
        int i;
        srandom(time(NULL));
        for (i = 0; i < getpid() % 43; i++)
            (void) random();
#endif
    }

    // Check for PDF
    if (fgets(buf, sizeof(buf), pf) == NULL) {
        fprintf(stderr, "%s: Empty file\n", paper);
        exit(1);
    }
    int need_pdf = 0;
    if (memcmp(buf, "%PDF-", 5) == 0) {
        int pipe1[2];
        if (pipe(pipe1) != 0) {
            perror(program_name);
            exit(1);
        }

        pid_t child = fork();
        if (child == 0) {
            size_t nread;

            close(STDOUT_FILENO);
            close(pipe1[0]);
            dup2(pipe1[1], STDOUT_FILENO);
            close(pipe1[1]);
            FILE* p = popen("pdf2ps -dLanguageLevel=3 - -", "w");
            if (!p) {
                perror("pdftops");
                exit(1);
            }
            close(STDOUT_FILENO);

            fputs(buf, p);
            while ((nread = fread(buf, 1, sizeof(buf), pf)))
                fwrite(buf, 1, nread, p);

            pclose(p);
            exit(0);
        } else if (child < 0) {
            perror(program_name);
            exit(1);
        }

        fclose(pf);
        close(pipe1[1]);
        pf = fdopen(pipe1[0], "r");
        if (fgets(buf, sizeof(buf), pf) == NULL) {
            fprintf(stderr, "%s: PDF conversion failed\n", paper);
            exit(1);
        }
        need_pdf = 1;
    }

    // Output
    if (need_pdf) {
        char* ocmd = malloc(strlen(output ? output : "-") * 2 + 20);
        char* opos;
        const char* ipos;
        strcpy(ocmd, "ps2pdf - ");
        opos = ocmd + strlen(ocmd);
        for (ipos = output ? output : "-"; *ipos; ++ipos)
            if (isalnum(*ipos) || *ipos == '/' || *ipos == '.')
                *opos++ = *ipos;
            else {
                *opos++ = '\\';
                *opos++ = *ipos;
            }
        *opos++ = 0;
        of = popen(ocmd, "w");
        free(ocmd);
    } else if (!output || strcmp(output, "-") == 0)
        of = stdout;
    else if (!(of = fopen(output, "w"))) {
        perror(output);
        exit(1);
    }

    // Actually process
    overflow = nest = 0;
    do {
        char *newline = strchr(buf, '\n');
        if (is_ps2write > 0 && ps2write_state)
            ps2write_state = check_ps2write(ps2write_state, buf);
        else if (is_ps2write < 0) {
            if (buf[0] != '%')
                is_ps2write = 0;
            else if (memcmp(buf, "%%Creator:", 10) == 0)
                is_ps2write = strstr(buf, "ps2write") != 0;
        }
        fputs(buf, of);
        if (!overflow && newline && memcmp(buf, "%%BeginDocument:", 16) == 0)
            nest++;
        else if (!overflow && newline && memcmp(buf, "%%EndDocument", 13) == 0)
            nest--;
        if (!overflow && newline && memcmp(buf, "%%Page: ", 8) == 0 && nest == 0) {
            double bbox[4] = {0, 0, 612, 792};
            if (fgets(buf, sizeof(buf), pf) != NULL
                && memcmp(buf, "%%PageBoundingBox: ", 19) == 0)
                sscanf(buf + 19, "%lg %lg %lg %lg", &bbox[0], &bbox[1], &bbox[2], &bbox[3]);
            cat(of, 0, bbox);
            ps2write_state = 1;
            continue;
        }
        overflow = (newline == NULL);
    } while (fgets(buf, sizeof(buf), pf) != NULL);

    if (need_pdf)
        pclose(of);
    else
        fclose(of);
    return 0;
}
