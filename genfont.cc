#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <png.h>
#include <stdio.h>
#include <cstring>
#include <clocale>

#define abort_(...) abort()

static int total_output;

static int x, y;

static int width, height;
static png_byte color_type;
static png_byte bit_depth;

static png_structp png_ptr;
static png_infop info_ptr;
static int number_of_passes;
static png_bytep * row_pointers;

void read_png_file(char const* file_name)
{
  char header[8];    // 8 is the maximum size that can be checked

  /* open file and test for it being a png */
  FILE *fp = fopen(file_name, "rb");
  if (!fp)
    abort_("[read_png_file] File %s could not be opened for reading", file_name);
  if (fread(header, 1, 8, fp) < 0)
    abort_("[read_png_file] fread failed");
  // if (png_sig_cmp(header, 0, 8))
  //   abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    abort_("[read_png_file] png_create_read_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    abort_("[read_png_file] png_create_info_struct failed");

  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during init_io");

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  png_set_expand(png_ptr);

  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during read_image");

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (int y=0; y<height; y++)
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));

  png_read_image(png_ptr, row_pointers);

  fclose(fp);
}

int main(int argc, char const **argv)
{
  if (argc <= 1) {
    std::cerr << "Missing source file argument";
    return 1;
  }
  setlocale(LC_ALL, "en_US.UTF-8");
  std::string indent(4, ' ');
  char const *pathname = argv[1];
  char const *dir_end = strrchr(pathname, '\\');
  char const *dir_end2 = strrchr(pathname, '/');
  dir_end = std::max(dir_end, dir_end2);
  size_t dir_len = dir_end ? (dir_end + 1) - pathname : 0;
  std::string file_dir(pathname, pathname + dir_len);
  std::string short_name = pathname + dir_len;
  short_name.resize(short_name.length() - 4);
  std::cerr << "Opening " << argv[1] << '\n';
  std::ifstream sfl(argv[1]);
  // std::vector<char> buf(1 << 14);
  // fontname \n
  // fontsize lineheight \n
  // texturefile \n
  // symbolcount \n
  // [id x y width height xoffset yoffset xadvance \n] for symbolcount
  // kerncount \n
  // [first second amount \n] for kerncount
  std::string font_name;
  std::getline(sfl, font_name, '\n');
  size_t font_size, line_height;
  sfl >> font_size >> line_height;
  std::string texture_name;
  std::getline(sfl, texture_name, '\n');
  if (!texture_name.empty())
    std::cout << "// Discarded \"" << texture_name << "\"\n";
  std::getline(sfl, texture_name, '\n');
  texture_name = file_dir + texture_name;
  size_t symbol_count;
  sfl >> symbol_count;
  struct font_char_info {
    int16_t id, x, y, width, height, xoffset, yoffset, xadvance;
  };
  std::vector<font_char_info> infos;
  std::string dummy;
  for (size_t i = 0; i < symbol_count; ++i) {
    infos.emplace_back();
    font_char_info &item = infos.back();
    sfl >> item.id >> item.x >> item.y >> item.width >> item.height >> 
      item.xoffset >> item.yoffset >> item.xadvance;
  }
  size_t kern_count;
  sfl >> kern_count;
  std::getline(sfl, dummy, '\n');
  struct font_kern_info {
    int16_t first, second, adjustment;
  };
  std::vector<font_kern_info> kerns;
  for (size_t i = 0; i < kern_count; ++i) {
    kerns.emplace_back();
    font_kern_info &item = kerns.back();
    sfl >> item.first >> item.second >> item.adjustment;
    std::getline(sfl, dummy, '\n');
  }
  std::cout << "// " <<
    font_name << ' ' << 
    font_size << ' ' << 
    line_height << ' ' << 
    texture_name << ' ' << 
    symbol_count << '=' << 
    infos.size() << ' ' << 
    kern_count << '=' <<
    kerns.size() << '\n';
  
  read_png_file(texture_name.c_str());

  std::cout << "#include <avr/pgmspace.h>\n";
  std::cout << "#include \"fontdata.h\"\n";
  std::cout << "static font_symbol const " << 
    short_name << "_font_symbols[] FONTDATA = {\n";
  for (int i = 0; i < symbol_count; ++i) {    
    std::cout << indent << "{ " <<
      infos[i].id << ", " <<
      //infos[i].x << ", " <<
      //infos[i].y << ", " <<
      infos[i].width << ", " <<
      infos[i].height << ", " <<
      infos[i].xoffset << ", " <<
      infos[i].yoffset << ", " <<
      infos[i].xadvance << " },\n";
    total_output += 12;
  }
  std::cout << "};\n";
  std::cout << "\n";
  std::cout << "static font_kern const " << 
    short_name << "_font_kerns[] FONTDATA = {\n";
  for (int i = 0; i < kern_count; ++i) {    
    std::cout << indent << "{ " <<
      kerns[i].first << ", " <<
      kerns[i].second << ", " <<
      kerns[i].adjustment << " },\n";
    total_output += 6;
  }
  std::cout << "};\n";
  std::cout << '\n';

  auto pixel_at = [&](int x, int y) -> bool {
    return row_pointers[y][x*4+3];
  };

  auto draw_rect = [&](int sx, int sy, int ex, int ey) {
    for (int y = sy; y < ey; ++y) {
      for (int x = sx; x < ex; ++x) {
        bool p = row_pointers[y][x*4+3];
        std::cout << (p ? "*" : ".");
      }
      std::cout << "\n";
    }
  };

  // std::cout << width << ' ' << height << bit_depth << '\n';
  // for (int i = 0; i < height; ++i) {
  //   uint8_t *row = row_pointers[i];
  //   for (int x = 0; x < width; ++x) {
  //     std::cout << (pixel_at(x, i) ? "*" : ".") << ' ';
  //   }
  //   std::cout << '\n';
  // }

  std::cout << "static uint8_t const " << 
    short_name << "_font_runs[] FONTDATA = {\n";
  std::vector<size_t> runs;
  std::vector<size_t> offsets;
  std::vector<size_t> lengths;
  for (size_t i = 0; i < infos.size(); ++i) {
    offsets.push_back(runs.size());

    std::mbstate_t state = std::mbstate_t();
    wchar_t const codepoints[] = { infos[i].id, 0 };
    wchar_t const *cpptr = codepoints;
    std::size_t len = 1 + std::wcsrtombs(nullptr, &cpptr, 0, &state);
    std::vector<char> mbstr(len);
    cpptr = codepoints;
    std::wcsrtombs(mbstr.data(), &cpptr, mbstr.size(), &state);

    std::cout << indent << "// Glyph " << 
      (int)infos[i].id << ' ' << mbstr.data() << 
      " width=" << infos[i].width <<
      " height=" << infos[i].height <<
      " xadvance=" << infos[i].xadvance <<
      " xofs=" << infos[i].xoffset <<
      " yofs=" << infos[i].yoffset <<
      '\n';

    bool in_pixel = false;
    int run = 0;
    std::cout << indent;

    for (int y = infos[i].y, ye = infos[i].y + infos[i].height, 
        xs = infos[i].x, xe = infos[i].x + infos[i].width; y < ye; ++y) {
      bool any_run = false;
      for (int x = xs; x < xe; ++x, ++run) {
        bool b = row_pointers[y][x*4+3];
        if (in_pixel != b) {
          std::cout << run << ", ";
          runs.push_back(run);
          any_run = true;
          run = 0;          
          in_pixel = b;
          ++total_output;
        }
      }
      if (any_run)
        std::cout << '\n' << indent;
    }
    if (run) {
      std::cout << indent << run << ", ";
      runs.push_back(run);
      ++total_output;
      run = 0;
    }
    std::cout << '\n';

    lengths.push_back(runs.size() - offsets.back());
  }
  std::cout << "};\n";

  std::cout << "static uint16_t const " << 
    short_name << "_font_offsets[] FONTDATA = {\n";
  for (size_t i = 0, ofs = 0; i < lengths.size(); ++i) {
    std::cout << "    " << ofs << ",// length=" << lengths[i] << "\n";
    ofs += lengths[i];
    total_output += 2;
  }
  std::cout << "};\n";

  std::cout << "font_info " << 
    short_name << "_font_info = {\n";
  std::cout << "    " << font_size << ",// font_size\n";
  std::cout << "    " << infos.size() << ",// glyph_count\n";
  std::cout << "    " << short_name <<"_font_symbols,\n";
  std::cout << "    " << short_name <<"_font_runs,\n";
  std::cout << "    " << kerns.size() << ",// kern_count\n";
  std::cout << "    " << short_name <<"_font_kerns,\n";
  std::cout << "    " << short_name <<"_font_offsets,\n";
  std::cout << "};\n";

  std::cout << "// approximate bytes: " << total_output << '\n';
  std::cerr << "Approximate font size: " << total_output << " bytes\n";
  std::cout << "// approximate due to unknown pointer size in code generator\n";

  // for (size_t i = 0; i < lengths.size(); ++i) {
  //   size_t length = lengths[i];
  //   size_t offset = offsets[i];
  //   for (size_t x = offset, e = offset + length; x < e; ++x) {
  //     std::cout << runs[x];
  //     if (x + 1 == e || i + 1 != lengths.size())
  //       std::cout << ", ";
  //   }
  //   std::cout << '\n';
  // }

  // for (size_t i = 0; i < infos.size(); ++i) {
  //   std::cout << "Glyph " << (char)infos[i].id << '\n';
  //   draw_rect(infos[i].x, infos[i].y, 
  //     infos[i].x + infos[i].width, 
  //     infos[i].y + infos[i].height);
  // }

  //" << short_name << "_
}

