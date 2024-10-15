#include <raylib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <unistd.h>
#endif

void createDirectory(const char *path)
{
    if (!DirectoryExists(path))
    {
        if (mkdir(path) == -1)
        {
            perror("Error creating directory");
        }
    }
}


// Load an Image font file (XNA style)
Font LoadFontFromImageHeadless(Image image, Color key, int firstChar)
{
#ifndef MAX_GLYPHS_FROM_IMAGE
    #define MAX_GLYPHS_FROM_IMAGE   256     // Maximum number of glyphs supported on image scan
#endif

    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a))

    Font font = GetFontDefault();

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_GLYPHS_FROM_IMAGE] = { 0 };
    Rectangle tempCharRecs[MAX_GLYPHS_FROM_IMAGE] = { 0 };

    Color *pixels = LoadImageColors(image);

    // Parse image data to get charSpacing and lineSpacing
    for (y = 0; y < image.height; y++)
    {
        for (x = 0; x < image.width; x++)
        {
            if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
        }

        if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
    }

    if ((x == 0) || (y == 0)) return font; // Security check

    charSpacing = x;
    lineSpacing = y;

    int charHeight = 0;
    int j = 0;

    while (!COLOR_EQUAL(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

    charHeight = j;

    // Check array values to get characters: value, x, y, w, h
    int index = 0;
    int lineToRead = 0;
    int xPosToRead = charSpacing;

    // Parse image data to get rectangle sizes
    while ((lineSpacing + lineToRead*(charHeight + lineSpacing)) < image.height)
    {
        while ((xPosToRead < image.width) &&
              !COLOR_EQUAL((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
        {
            tempCharValues[index] = firstChar + index;

            tempCharRecs[index].x = (float)xPosToRead;
            tempCharRecs[index].y = (float)(lineSpacing + lineToRead*(charHeight + lineSpacing));
            tempCharRecs[index].height = (float)charHeight;

            int charWidth = 0;

            while (!COLOR_EQUAL(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

            tempCharRecs[index].width = (float)charWidth;

            index++;

            xPosToRead += (charWidth + charSpacing);
        }

        lineToRead++;
        xPosToRead = charSpacing;
    }

    // NOTE: We need to remove key color borders from image to avoid weird
    // artifacts on texture scaling when using TEXTURE_FILTER_BILINEAR or TEXTURE_FILTER_TRILINEAR
    for (int i = 0; i < image.height*image.width; i++) if (COLOR_EQUAL(pixels[i], key)) pixels[i] = BLANK;

    // Create a new image with the processed color data (key color replaced by BLANK)
    Image fontClear = {
        .data = pixels,
        .width = image.width,
        .height = image.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Set font with all data parsed from image
    font.glyphCount = index;
    font.glyphPadding = 0;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    font.glyphs = (GlyphInfo *)RL_MALLOC(font.glyphCount*sizeof(GlyphInfo));
    font.recs = (Rectangle *)RL_MALLOC(font.glyphCount*sizeof(Rectangle));

    for (int i = 0; i < font.glyphCount; i++)
    {
        font.glyphs[i].value = tempCharValues[i];

        // Get character rectangle in the font atlas texture
        font.recs[i] = tempCharRecs[i];

        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        font.glyphs[i].offsetX = 0;
        font.glyphs[i].offsetY = 0;
        font.glyphs[i].advanceX = 0;

        // Fill character image data from fontClear data
        font.glyphs[i].image = ImageFromImage(fontClear, tempCharRecs[i]);
    }

    font.baseSize = (int)font.recs[0].height;

    return font;
}

int GetPow2(int value)
{
    int result = 1;
    int pow = 0;
    while (result < value)
    {
        result *= 2;
        pow++;
    }
    return pow;
}


void png2c(const char *filename, const char *output)
{
    // check modification date of files
    struct stat stat_src;
    struct stat stat_dst;
    struct stat stat_gen;
    if (stat(filename, &stat_src) == 0 && stat(output, &stat_dst) == 0 && stat("src_build/gen_assets.c", &stat_gen) == 0)
    {
        if (stat_src.st_mtime < stat_dst.st_mtime && stat_gen.st_mtime < stat_dst.st_mtime)
        {
            printf("Skipping %s\n", filename);
            return;
        }
    }

    printf("Processing: %s -> %s\n", filename, output);
    FILE *f = fopen(output, "w");
    if (f == NULL)
    {
        perror("Error opening file");
        return;
    }
    char headerFile[256];
    sprintf(headerFile, "_src_gen/%s.h", GetFileNameWithoutExt(output));

    FILE *header = fopen(headerFile, "w");
    if (header == NULL)
    {
        perror("Error opening file");
        fclose(f);
        return;
    }

    fprintf(header, "#ifndef %s_H\n", GetFileNameWithoutExt(output));
    fprintf(header, "#define %s_H\n", GetFileNameWithoutExt(output));
    fprintf(header, "#include <inttypes.h>\n");
    fprintf(header, "extern const uint16_t %s_width;\n", GetFileNameWithoutExt(output));
    fprintf(header, "extern const uint16_t %s_height;\n", GetFileNameWithoutExt(output));
    fprintf(header, "extern const uint8_t %s_p2width;\n", GetFileNameWithoutExt(output));
    fprintf(header, "extern const uint8_t %s_p2height;\n", GetFileNameWithoutExt(output));
    fprintf(header, "extern const uint32_t %s_data_size;\n", GetFileNameWithoutExt(output));
    fprintf(header, "extern const uint8_t %s_data[];\n", GetFileNameWithoutExt(output));

    Image img = LoadImage(filename);
    fprintf(f, "#include <inttypes.h>\n");
    fprintf(f, "const uint16_t %s_width = %d;\n", GetFileNameWithoutExt(filename), img.width);
    fprintf(f, "const uint16_t %s_height = %d;\n", GetFileNameWithoutExt(filename), img.height);
    fprintf(f, "const uint8_t %s_p2width = %d;\n", GetFileNameWithoutExt(filename), GetPow2(img.width));
    fprintf(f, "const uint8_t %s_p2height = %d;\n", GetFileNameWithoutExt(filename), GetPow2(img.height));
    fprintf(f, "const uint32_t %s_data_size = %d;\n", GetFileNameWithoutExt(filename), img.width * img.height * 4);
    fprintf(f, "const uint8_t %s_data[] = {\n", GetFileNameWithoutExt(filename));
    unsigned char *pixels = (unsigned char*) img.data;
    for (int i = 0; i < img.width * img.height * 4; i++)
    {
        fprintf(f, "0x%02x", pixels[i]);
        if (i < img.width * img.height * 4 - 1)
        {
            fprintf(f, ",");
        }
        if ((i + 1) % 16 == 0)
        {
            fprintf(f, "\n");
        }
    }
    fprintf(f, "};\n");

    for (int i=strlen(filename) - 1;i >= 0;i--)
    {
        if (filename[i] == '/' || filename[i] == '\\')
        {
            if (filename[i + 1] == 'f' && filename[i + 2] == 'n' && filename[i + 3] == 't' && filename[i + 4] == '_')
            {
                printf("Processing font: %s\n", filename);
                Font font = LoadFontFromImageHeadless(img, MAGENTA, 32);
                printf("loaded font: %s\n", filename);
                printf("Font: %s has %d glyphs\n", filename, font.glyphCount);

                fprintf(header, "extern const uint16_t %s_glyph_count;\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint16_t %s_glyph_padding;\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint16_t %s_base_size;\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint16_t %s_glyphs_values[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint8_t %s_glyphs_offsets_x[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint8_t %s_glyphs_offsets_y[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint8_t %s_glyphs_advances_x[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint16_t %s_glyphs_rects_x[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint16_t %s_glyphs_rects_y[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint8_t %s_glyphs_rects_width[];\n", GetFileNameWithoutExt(filename));
                fprintf(header, "extern const uint8_t %s_glyphs_rects_height[];\n", GetFileNameWithoutExt(filename));
                

                fprintf(f, "const uint16_t %s_glyph_count = %d;\n", GetFileNameWithoutExt(filename), font.glyphCount);
                fprintf(f, "const uint16_t %s_glyph_padding = %d;\n", GetFileNameWithoutExt(filename), font.glyphPadding);
                fprintf(f, "const uint16_t %s_base_size = %d;\n", GetFileNameWithoutExt(filename), font.baseSize);
                fprintf(f, "const uint16_t %s_glyphs_values[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", font.glyphs[i].value, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint8_t %s_glyphs_offsets_x[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", font.glyphs[i].offsetX, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint8_t %s_glyphs_offsets_y[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", font.glyphs[i].offsetY, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint8_t %s_glyphs_advances_x[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", font.glyphs[i].advanceX, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint16_t %s_glyphs_rects_x[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", (int)font.recs[i].x, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint16_t %s_glyphs_rects_y[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", (int)font.recs[i].y, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint8_t %s_glyphs_rects_width[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", (int)font.recs[i].width, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
                fprintf(f, "const uint8_t %s_glyphs_rects_height[] = {\n", GetFileNameWithoutExt(filename));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fprintf(f, "%d,%s", (int)font.recs[i].height, (i + 1) % 16 == 0 ? "\n" : "");
                }
                fprintf(f, "};\n");
            }
            break;
        }
    }

    UnloadImage(img);
    fclose(f);
    fprintf(header, "#endif\n");
    fclose(header);
}

void pngs2c(const char *dir, const char *outDir)
{
    // create out director if not exists
    createDirectory(outDir);
    FilePathList list = LoadDirectoryFiles(dir);
    for (int i = 0; i < list.count; i++)
    {
        if (IsFileExtension(list.paths[i], ".png"))
        {
            char outFile[256];
            sprintf(outFile, "%s/%s.c", outDir, GetFileNameWithoutExt(list.paths[i]));
            png2c(list.paths[i], outFile);
        }
    }
    UnloadDirectoryFiles(list);
}

void packMods(const char *dir, const char *outDir)
{
    createDirectory(outDir);
    FilePathList list = LoadDirectoryFiles(dir);
    for (int i = 0; i < list.count; i++)
    {
        if (IsFileExtension(list.paths[i], ".mod"))
        {
            printf("Processing MOD: %s\n", list.paths[i]);
            // read mod file and store as-is in c byte array
            FILE *f = fopen(list.paths[i], "rb");
            if (f == NULL)
            {
                perror("Error opening file");
                continue;
            }
            char outFile[256];
            char rawFileName[256];
            const char *fileNameWithoutExt = GetFileNameWithoutExt(list.paths[i]);
            strcpy(rawFileName, fileNameWithoutExt);
            sprintf(outFile, "%s/%s.c", outDir, rawFileName);
            FILE *out = fopen(outFile, "w");
            if (out == NULL)
            {
                perror("Error opening file");
                fclose(f);
                continue;
            }
            fprintf(out, "char moddata_%s[] = {\n", rawFileName);
            unsigned char buffer[1024];
            size_t bytesRead;
            int byteCount = 0;
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0)
            {
                byteCount += bytesRead;
                for (int i = 0; i < bytesRead; i++)
                {
                    fprintf(out, "0x%02x,", buffer[i]);
                    if ((i + 1) % 16 == 0)
                    {
                        fprintf(out, "\n");
                    }
                }
            }
            fprintf(out, "};\n");
            fprintf(out, "int moddata_%s_size = %d;\n", rawFileName, byteCount);
            fclose(out);
            fclose(f);

            // create header file
            char headerFile[256];
            sprintf(headerFile, "%s/%s.h", outDir, rawFileName);
            FILE *header = fopen(headerFile, "w");
            if (header == NULL)
            {
                perror("Error opening file");
                continue;
            }
            fprintf(header, "#ifndef %s_H\n", rawFileName);
            fprintf(header, "#define %s_H\n", rawFileName);
            fprintf(header, "extern char moddata_%s[];\n", rawFileName);
            fprintf(header, "extern int moddata_%s_size;\n", rawFileName);
            fprintf(header, "#endif\n");
            fclose(header);

            printf("Processed MOD: %s\n", list.paths[i]);
        }
    }

    UnloadDirectoryFiles(list);
}

void generateAssets()
{
    pngs2c("assets", "_src_gen");
    packMods("assets", "_src_gen");
}
